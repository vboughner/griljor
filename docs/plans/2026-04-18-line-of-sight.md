# Line-of-Sight DDA Fix — Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the incorrect Chebyshev-stepping LOS ray with a geometrically correct DDA supercover algorithm that traces the actual center-to-center line between tiles.

**Architecture:** A single pure function `losRayTiles(x1, y1, x2, y2)` replaces `chebyshevPath`. It uses DDA grid traversal from tile center to tile center, returning all tiles whose interior the line passes through (excluding start). Boundary-touching tiles (line grazes edge/corner) are excluded (permissive). The function is duplicated in `client/src/los.ts` and `server/src/session.ts` (existing pattern). Callers (`spotIsVisible`, `tileIsVisible`, pickup walkability) switch to the new function.

**Tech Stack:** TypeScript, Vitest

---

### Task 1: Write failing tests for `losRayTiles` in client

**Files:**
- Modify: `client/src/__tests__/los.test.ts`
- Reference: `client/src/los.ts`

**Step 1: Write the failing tests**

Replace the `chebyshevPath` test block with tests for `losRayTiles`. Update the import at line 2 to import `losRayTiles` instead of `chebyshevPath`.

```typescript
import { losRayTiles, tileViewBlocked, spotIsVisible, tileIsVisible } from '../los';
```

Replace the `describe('chebyshevPath', ...)` block (lines 41–80) with:

```typescript
describe('losRayTiles', () => {
  it('same tile returns empty array', () => {
    expect(losRayTiles(2, 2, 2, 2)).toEqual([]);
  });

  it('adjacent horizontal returns single tile', () => {
    expect(losRayTiles(0, 0, 1, 0)).toEqual([{ x: 1, y: 0 }]);
  });

  it('straight horizontal path (0,0)→(3,0)', () => {
    expect(losRayTiles(0, 0, 3, 0)).toEqual([
      { x: 1, y: 0 },
      { x: 2, y: 0 },
      { x: 3, y: 0 },
    ]);
  });

  it('straight vertical path (0,0)→(0,3)', () => {
    expect(losRayTiles(0, 0, 0, 3)).toEqual([
      { x: 0, y: 1 },
      { x: 0, y: 2 },
      { x: 0, y: 3 },
    ]);
  });

  it('perfect diagonal (0,0)→(2,2) — corner crossings, steps diagonally', () => {
    expect(losRayTiles(0, 0, 2, 2)).toEqual([
      { x: 1, y: 1 },
      { x: 2, y: 2 },
    ]);
  });

  it('off-axis (0,0)→(1,3) — follows actual geometric line', () => {
    // Line from (0.5,0.5) to (1.5,3.5): passes through (0,1), (0,2)/(1,2), (1,2), (1,3)
    // NOT through (1,1) like chebyshevPath would
    const tiles = losRayTiles(0, 0, 1, 3);
    expect(tiles).not.toContainEqual({ x: 1, y: 1 });
    expect(tiles).toContainEqual({ x: 0, y: 1 });
    expect(tiles).toContainEqual({ x: 1, y: 3 });
  });

  it('off-axis (0,0)→(3,1) — follows actual geometric line', () => {
    // Line from (0.5,0.5) to (3.5,1.5): passes through (1,0), (2,0)/(2,1), (2,1), (3,1)
    // NOT through (1,1) like chebyshevPath would
    const tiles = losRayTiles(0, 0, 3, 1);
    expect(tiles).not.toContainEqual({ x: 1, y: 1 });
    expect(tiles).toContainEqual({ x: 1, y: 0 });
    expect(tiles).toContainEqual({ x: 3, y: 1 });
  });

  it('boundary crossing is permissive — corner-touching tiles excluded', () => {
    // (0,0)→(2,2): line goes through exact corners at (1,1) grid intersection
    // Only diagonal tiles (1,1) and (2,2) should appear; (1,0) and (0,1) should NOT
    const tiles = losRayTiles(0, 0, 2, 2);
    expect(tiles).not.toContainEqual({ x: 1, y: 0 });
    expect(tiles).not.toContainEqual({ x: 0, y: 1 });
  });

  it('negative direction (3,3)→(0,0)', () => {
    expect(losRayTiles(3, 3, 0, 0)).toEqual([
      { x: 2, y: 2 },
      { x: 1, y: 1 },
      { x: 0, y: 0 },
    ]);
  });

  it('steep angle (0,0)→(1,5) traces correct tiles', () => {
    const tiles = losRayTiles(0, 0, 1, 5);
    // Should pass through column 0 for several rows before crossing to column 1
    expect(tiles).toContainEqual({ x: 0, y: 1 });
    expect(tiles).toContainEqual({ x: 0, y: 2 });
    expect(tiles).toContainEqual({ x: 1, y: 5 });
    // Should NOT contain (1,1) — the line is far from that tile
    expect(tiles).not.toContainEqual({ x: 1, y: 1 });
  });
});
```

**Step 2: Run tests to verify they fail**

Run: `cd client && npx vitest run src/__tests__/los.test.ts`
Expected: FAIL — `losRayTiles` is not exported from `../los`

---

### Task 2: Implement `losRayTiles` in client

**Files:**
- Modify: `client/src/los.ts:1-25` (replace `chebyshevPath` with `losRayTiles`)
- Modify: `client/src/los.ts:69,91` (update callers)

**Step 1: Replace `chebyshevPath` with `losRayTiles`**

Replace the `chebyshevPath` function (lines 6–25) with:

```typescript
/**
 * DDA supercover ray: returns every tile whose interior the line segment
 * from center of (x1,y1) to center of (x2,y2) passes through.
 * Excludes the start tile. Includes the target tile.
 * When the ray passes exactly along a tile edge or corner (boundary),
 * that boundary tile is excluded (permissive LOS).
 */
export function losRayTiles(
  x1: number,
  y1: number,
  x2: number,
  y2: number,
): Array<{ x: number; y: number }> {
  if (x1 === x2 && y1 === y2) return [];

  const dx = x2 - x1;
  const dy = y2 - y1;
  const sx = Math.sign(dx);
  const sy = Math.sign(dy);
  const adx = Math.abs(dx);
  const ady = Math.abs(dy);

  const path: Array<{ x: number; y: number }> = [];
  let cx = x1;
  let cy = y1;

  if (adx === 0) {
    // Pure vertical
    for (let i = 0; i < ady; i++) {
      cy += sy;
      path.push({ x: cx, y: cy });
    }
    return path;
  }
  if (ady === 0) {
    // Pure horizontal
    for (let i = 0; i < adx; i++) {
      cx += sx;
      path.push({ x: cx, y: cy });
    }
    return path;
  }

  // General DDA: track fractional progress to next grid line crossing.
  // tMaxX/tMaxY = parameter value at which ray crosses the next vertical/horizontal
  // grid line. tDeltaX/tDeltaY = parameter increment per grid cell in each axis.
  // We use integers scaled by adx*ady to avoid floating point entirely.
  // t is parameterized so that full ray length = adx * ady.
  let tMaxX = ady; // first vertical crossing: 0.5 cell / adx * (adx*ady) = ady/2, but we start at center so first crossing is at 0.5/adx * scale = ady * 0.5... let me use half-steps
  let tMaxY = adx;
  const tDeltaX = 2 * ady; // each subsequent vertical crossing
  const tDeltaY = 2 * adx;

  while (cx !== x2 || cy !== y2) {
    if (tMaxX < tMaxY) {
      // Cross vertical grid line next
      cx += sx;
      tMaxX += tDeltaX;
    } else if (tMaxY < tMaxX) {
      // Cross horizontal grid line next
      cy += sy;
      tMaxY += tDeltaY;
    } else {
      // Simultaneous crossing (corner) — step diagonally, skip grazing tiles
      cx += sx;
      cy += sy;
      tMaxX += tDeltaX;
      tMaxY += tDeltaY;
    }
    path.push({ x: cx, y: cy });
  }

  return path;
}
```

**Step 2: Update `spotIsVisible` to use `losRayTiles`**

In `spotIsVisible` (line 69), change `chebyshevPath` to `losRayTiles`:

```typescript
  const path = losRayTiles(x1, y1, x2, y2);
```

**Step 3: Update `tileIsVisible` to use `losRayTiles`**

In `tileIsVisible` (line 91), change `chebyshevPath` to `losRayTiles`:

```typescript
  const path = losRayTiles(x1, y1, x2, y2);
```

**Step 4: Run client LOS tests**

Run: `cd client && npx vitest run src/__tests__/los.test.ts`
Expected: All tests PASS

**Step 5: Run full client test suite**

Run: `cd client && npx vitest run`
Expected: All tests PASS

**Step 6: Commit**

```bash
git add client/src/los.ts client/src/__tests__/los.test.ts
git commit -m "fix(client): replace chebyshevPath with DDA supercover ray for line-of-sight

The chebyshev stepping algorithm visited tiles the actual geometric line
didn't pass through, causing false LOS blocks on diagonal sightlines.
The new losRayTiles uses DDA grid traversal to trace the real
center-to-center line. Boundary-touching tiles are excluded (permissive)."
```

---

### Task 3: Write failing tests for `losRayTiles` in server

**Files:**
- Modify: `server/src/__tests__/los.test.ts`

**Step 1: Write the failing tests**

Update the import at line 2:

```typescript
import { losRayTiles, tileViewBlocked, spotIsVisible } from '../session';
```

Replace the `describe('chebyshevPath', ...)` block (lines 45–84) with the same `losRayTiles` test suite as Task 1 Step 1 (identical tests, just different import).

**Step 2: Run tests to verify they fail**

Run: `cd server && npx vitest run src/__tests__/los.test.ts`
Expected: FAIL — `losRayTiles` is not exported from `../session`

---

### Task 4: Implement `losRayTiles` in server

**Files:**
- Modify: `server/src/session.ts:98-113` (replace `chebyshevPath`)
- Modify: `server/src/session.ts:156` (update `spotIsVisible` caller)
- Modify: `server/src/session.ts:738` (update pickup walkability caller)

**Step 1: Replace `chebyshevPath` with `losRayTiles`**

Replace the `chebyshevPath` function at lines 98–113 with the identical `losRayTiles` implementation from Task 2 Step 1.

**Step 2: Update `spotIsVisible` caller**

At line 156, change `chebyshevPath` to `losRayTiles`.

**Step 3: Update pickup walkability caller**

At line 738, change `chebyshevPath` to `losRayTiles`:

```typescript
    for (const { x, y } of losRayTiles(player.x, player.y, msg.x, msg.y)) {
```

**Step 4: Run server LOS tests**

Run: `cd server && npx vitest run src/__tests__/los.test.ts`
Expected: All tests PASS

**Step 5: Run server integration tests**

Run: `cd server && npx vitest run src/__tests__/integration/`
Expected: All tests PASS (existing wall-blocking and open-room tests should still pass)

**Step 6: Run full test suite**

Run: `npm test`
Expected: All tests PASS

**Step 7: Commit**

```bash
git add server/src/session.ts server/src/__tests__/los.test.ts
git commit -m "fix(server): replace chebyshevPath with DDA supercover ray for line-of-sight

Mirror the client-side fix: use losRayTiles for server visibility checks
and pickup walkability path. Same DDA algorithm, same permissive boundary
rule."
```

---

### Task 5: Lint, format, and verify

**Files:**
- All changed files

**Step 1: Run lint and format checks**

```bash
cd client && npm run lint && npm run format:check
cd server && npm run lint && npm run format:check
```

Expected: Both pass with exit 0. If format issues, run `npm run format` in the relevant directory.

**Step 2: Run full test suite one final time**

```bash
npm test
```

Expected: All tests PASS

**Step 3: Fix any issues and commit if needed**

Only commit if lint/format changes were required.
