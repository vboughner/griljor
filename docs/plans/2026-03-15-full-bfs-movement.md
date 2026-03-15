# Full BFS Movement Path Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the oscillation-prone hybrid Bresenham+single-BFS-step movement with a full BFS path computed once at click time.

**Architecture:** Add `computeBfsPath` to `game-utils.ts` that returns the complete walkable path from start to target. Replace `computeBresenhamPath` in `startMovingTo` with `computeBfsPath`. Simplify `doMoveStep` by removing all inline BFS rerouting — the path is now pre-validated so `doMoveStep` just follows it step by step, stopping only if a dynamic obstacle (another player) blocks the way. Remove the now-unused `findNextStep` import from `game.ts`.

**Tech Stack:** TypeScript, Vitest (client unit tests with `happy-dom` env)

---

### Task 1: Add `computeBfsPath` to `game-utils.ts`

**Files:**
- Modify: `client/src/game-utils.ts`
- Test: `client/src/__tests__/game-utils.test.ts`

**Step 1: Write failing tests** — add these to `client/src/__tests__/game-utils.test.ts`, importing `computeBfsPath` alongside the existing imports:

```ts
import { isTileBlocked, findNextStep, computeBresenhamPath, buildExitMap, computeBfsPath } from '../game-utils';
```

Add a new `describe` block at the end of the file:

```ts
describe('computeBfsPath', () => {
  it('returns empty array when already at target', () => {
    const room = emptyRoom();
    expect(computeBfsPath(5, 5, 5, 5, room, [])).toEqual([]);
  });

  it('returns direct path in open space', () => {
    const room = emptyRoom();
    const path = computeBfsPath(5, 5, 8, 5, room, []);
    expect(path[path.length - 1]).toEqual({ x: 8, y: 5 });
    expect(path[0]).toEqual({ x: 6, y: 5 }); // first step right
    expect(path.length).toBe(3);
  });

  it('returns empty array when target is unreachable', () => {
    const room = emptyRoom();
    const objects = makeObjects({ 99: { movement: 0 } });
    // Surround target (10,10) with walls on all 8 neighbours
    for (const [dx, dy] of [[0,1],[0,-1],[1,0],[-1,0],[1,1],[1,-1],[-1,1],[-1,-1]]) {
      room.spot![10 + dx][10 + dy] = [0, 99];
    }
    expect(computeBfsPath(5, 5, 10, 10, room, objects)).toEqual([]);
  });

  it('navigates around a wall — no oscillation', () => {
    // Reproduces the oscillation bug: horizontal wall blocking direct path,
    // player must go around the corner.
    const room = emptyRoom();
    const objects = makeObjects({ 99: { movement: 0 } });
    // Wall at x=5, y=3..7 (vertical wall)
    for (let y = 3; y <= 7; y++) room.spot![5][y] = [0, 99];

    // Player at (4,5) wants to reach (7,5) — must go around the wall
    const path = computeBfsPath(4, 5, 7, 5, room, objects);

    // Path must reach target
    expect(path[path.length - 1]).toEqual({ x: 7, y: 5 });
    // Path must not pass through any walled tile
    for (const step of path) {
      expect(isTileBlocked(step.x, step.y, room, objects)).toBe(false);
    }
    // Each consecutive step must be adjacent (distance ≤ 1 in each axis)
    let cx = 4, cy = 5;
    for (const step of path) {
      expect(Math.abs(step.x - cx)).toBeLessThanOrEqual(1);
      expect(Math.abs(step.y - cy)).toBeLessThanOrEqual(1);
      cx = step.x; cy = step.y;
    }
  });

  it('path does not include the starting tile', () => {
    const room = emptyRoom();
    const path = computeBfsPath(3, 3, 5, 3, room, []);
    expect(path.every(p => !(p.x === 3 && p.y === 3))).toBe(true);
  });
});
```

**Step 2: Run tests to verify they fail**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout && npm run test:client -- --reporter=verbose 2>&1 | grep -A3 "computeBfsPath"
```
Expected: FAIL — `computeBfsPath` not exported

**Step 3: Implement `computeBfsPath`** — add to `client/src/game-utils.ts` after `computeBresenhamPath`:

```ts
/**
 * Full BFS path from (x0,y0) to (x1,y1) through walkable tiles.
 * Returns the sequence of tiles to visit (not including start).
 * Returns [] if already at target or no path exists.
 */
export function computeBfsPath(
  x0: number,
  y0: number,
  x1: number,
  y1: number,
  room: RoomData,
  objects: ObjDef[],
  exitKeys?: Set<string>,
): Array<{ x: number; y: number }> {
  if (x0 === x1 && y0 === y1) return [];
  const NONE = -1;
  const visited = new Uint8Array(GRID * GRID);
  const prev = new Int16Array(GRID * GRID).fill(NONE);
  const queue: Array<{ x: number; y: number }> = [];
  visited[y0 * GRID + x0] = 1;
  queue.push({ x: x0, y: y0 });
  let found = false;
  outer: while (queue.length > 0) {
    const { x, y } = queue.shift()!;
    for (const [dx, dy] of STEP_DIRS) {
      const nx = x + dx,
        ny = y + dy;
      if (nx < 0 || nx >= GRID || ny < 0 || ny >= GRID) continue;
      if (isTileBlocked(nx, ny, room, objects, exitKeys)) continue;
      const k = ny * GRID + nx;
      if (visited[k]) continue;
      visited[k] = 1;
      prev[k] = y * GRID + x;
      if (nx === x1 && ny === y1) {
        found = true;
        break outer;
      }
      queue.push({ x: nx, y: ny });
    }
  }
  if (!found) return [];
  // Reconstruct path by tracing prev pointers from target back to start
  const path: Array<{ x: number; y: number }> = [];
  let cx = x1,
    cy = y1;
  while (cx !== x0 || cy !== y0) {
    path.unshift({ x: cx, y: cy });
    const p = prev[cy * GRID + cx];
    if (p === NONE) return []; // should not happen
    cx = p % GRID;
    cy = Math.floor(p / GRID);
  }
  return path;
}
```

**Step 4: Run tests to verify they pass**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout && npm run test:client -- --reporter=verbose 2>&1 | grep -A3 "computeBfsPath"
```
Expected: all 5 `computeBfsPath` tests PASS

**Step 5: Run full client test suite**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout && npm run test:client
```
Expected: all passing (no regressions)

**Step 6: Commit**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout
git add client/src/game-utils.ts client/src/__tests__/game-utils.test.ts
git commit -m "feat: add computeBfsPath for full obstacle-aware movement paths"
```

---

### Task 2: Simplify `startMovingTo` and `doMoveStep` in `game.ts`

**Files:**
- Modify: `client/src/game.ts`

This task replaces the oscillation-prone Bresenham+BFS-reroute logic with the full BFS path.

**Step 1: Update the import** at the top of `game.ts`. Replace:

```ts
import {
  isTileBlocked,
  findNextStep,
  computeBresenhamPath,
  buildExitMap,
  ExitTile,
} from './game-utils';
```

With:

```ts
import {
  computeBfsPath,
  buildExitMap,
  ExitTile,
} from './game-utils';
```

(`isTileBlocked` and `findNextStep` and `computeBresenhamPath` are no longer used in `game.ts` after this change.)

**Step 2: Replace `startMovingTo`** (find the `private startMovingTo` method and replace its body):

```ts
private startMovingTo(x: number, y: number): void {
  this.stopMoving();
  this.moveTarget = { x, y };
  const room = this.mapData.rooms[this.currentRoom];
  if (x >= 0 && x < GRID && y >= 0 && y < GRID) {
    this.movePath = computeBfsPath(this.px, this.py, x, y, room, this.objects, this.exitKeys);
  } else {
    // Off-grid border exit: BFS to nearest edge tile, then step off
    const ex = Math.max(0, Math.min(GRID - 1, x));
    const ey = Math.max(0, Math.min(GRID - 1, y));
    this.movePath =
      this.px === ex && this.py === ey
        ? []
        : computeBfsPath(this.px, this.py, ex, ey, room, this.objects, this.exitKeys);
  }
  this.scheduleMoveStep();
}
```

**Step 3: Replace `doMoveStep`** (find the `private async doMoveStep` method and replace it entirely):

```ts
private async doMoveStep(): Promise<void> {
  if (this.isDead) {
    this.stopMoving();
    return;
  }
  if (!this.moveTarget) return;
  const target = this.moveTarget;

  let dx: number, dy: number;
  const offGrid = target.x < 0 || target.x >= GRID || target.y < 0 || target.y >= GRID;

  if (offGrid) {
    if (this.movePath.length > 0) {
      // Still navigating to the edge tile
      const next = this.movePath[0];
      dx = next.x - this.px;
      dy = next.y - this.py;
    } else {
      // At the edge tile — step off the grid to trigger room transition
      dx = Math.sign(target.x - this.px);
      dy = Math.sign(target.y - this.py);
    }
  } else {
    // In-room: follow pre-computed BFS path
    if (this.movePath.length === 0) {
      this.stopMoving();
      return;
    }
    const next = this.movePath[0];
    dx = next.x - this.px;
    dy = next.y - this.py;
  }

  if (dx === 0 && dy === 0) {
    this.stopMoving();
    return;
  }

  const prevRoom = this.currentRoom;
  const prevX = this.px;
  const prevY = this.py;

  await this.move(dx, dy);

  // Room changed: arrived in new room, stop
  if (this.currentRoom !== prevRoom) {
    this.stopMoving();
    return;
  }

  // Blocked by a dynamic obstacle (e.g. another player on that tile): stop
  if (this.px === prevX && this.py === prevY) {
    this.stopMoving();
    return;
  }

  // Consume the step we just completed
  if (this.movePath.length > 0 && this.movePath[0].x === this.px && this.movePath[0].y === this.py) {
    this.movePath.shift();
  }

  // Still have a target — keep going
  if (this.moveTarget) this.scheduleMoveStep();
}
```

**Step 4: Run lint/format check**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout/client && npm run lint && npm run format:check
```
If format fails: `npm run format` then recheck. Fix any lint errors (likely unused imports if any slipped through).

**Step 5: Run full test suite**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout && npm test
```
Expected: all client tests pass; 2 pre-existing server combat test failures unrelated to this change.

**Step 6: Commit**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout
git add client/src/game.ts
git commit -m "fix: replace Bresenham+BFS-reroute with full BFS path to fix movement oscillation"
```

---

### Task 3: Final lint/format check across both packages

**Step 1: Lint and format both packages**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout/server && npm run lint && npm run format:check
cd /Users/van/dev/griljor-2-lobby-and-timeout/client && npm run lint && npm run format:check
```
Expected: both pass with exit code 0.

**Step 2: Run full test suite**

```bash
cd /Users/van/dev/griljor-2-lobby-and-timeout && npm test
```
Expected: same result as before (all passing except the 2 pre-existing combat failures).
