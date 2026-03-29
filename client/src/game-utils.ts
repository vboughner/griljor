import { RoomData, ObjDef } from './types';
import { chebyshevPath } from './los';

const GRID = 20;
export const PICKUP_RANGE = 4; // max Chebyshev distance to pick up an item

export interface ExitTile {
  destRoom: number;
  landX: number;
  landY: number;
}

const STEP_DIRS: [number, number][] = [
  [0, -1],
  [1, 0],
  [0, 1],
  [-1, 0], // cardinal first (preferred for straight paths)
  [1, -1],
  [1, 1],
  [-1, 1],
  [-1, -1], // then diagonal
];

/** Returns true if the tile at (x, y) cannot be entered by players.
 *  movement>0 means walkable; movement=0/absent means blocked.
 *  Void tiles [0,0] are always walkable (speed 9); respawn exclusion is server-side.
 *  permeable controls missile passage only (not player movement). */
export function isTileBlocked(
  x: number,
  y: number,
  room: RoomData,
  objects: ObjDef[],
  exitKeys?: Set<string>,
): boolean {
  if (exitKeys?.has(`${x},${y}`)) return false; // exit tiles are always walkable
  const cell = room.spot?.[x]?.[y];
  if (cell) {
    const [flId, wlId] = cell;
    // Void tile [0,0]: always walkable (speed 9). Respawn exclusion is handled server-side.
    if (!flId && !wlId) {
      // nothing — fall through to return false (walkable)
    } else {
      // Non-void: block if any object lacks movement (absent = blocked)
      if (wlId > 0 && !((objects[wlId]?.movement ?? 0) > 0)) return true;
      if (flId > 0 && !((objects[flId]?.movement ?? 0) > 0)) return true;
    }
  }
  // Skip takeable items — they lie on the floor and don't block movement.
  for (const ro of room.recorded_objects ?? []) {
    if (ro.x === x && ro.y === y && ro.type > 0) {
      const obj = objects[ro.type];
      if (obj?.takeable) continue;
      if (!((obj?.movement ?? 0) > 0)) return true;
    }
  }
  return false;
}

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
  const path: Array<{ x: number; y: number }> = [];
  let cx = x1,
    cy = y1;
  while (cx !== x0 || cy !== y0) {
    path.unshift({ x: cx, y: cy });
    const p = prev[cy * GRID + cx];
    if (p === NONE) return [];
    cx = p % GRID;
    cy = Math.floor(p / GRID);
  }
  return path;
}

/**
 * Like computeBfsPath, but if the exact target is unreachable (blocked or
 * disconnected), returns a path to the closest reachable tile by Chebyshev
 * distance instead of returning [].
 */
export function computeBfsPathToNearest(
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
  let foundExact = false;

  while (queue.length > 0) {
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
        foundExact = true;
        // No early exit: BFS must run to exhaustion so the full visited set is
        // available for the fallback scan when the exact target is unreachable.
      }
      queue.push({ x: nx, y: ny });
    }
  }

  // Reconstruct path helper
  const buildPath = (tx: number, ty: number) => {
    const path: Array<{ x: number; y: number }> = [];
    let cx = tx,
      cy = ty;
    while (cx !== x0 || cy !== y0) {
      path.unshift({ x: cx, y: cy });
      const p = prev[cy * GRID + cx];
      if (p === NONE) return [];
      cx = p % GRID;
      cy = Math.floor(p / GRID);
    }
    return path;
  };

  if (foundExact) return buildPath(x1, y1);

  // Target unreachable — find closest visited tile to (x1, y1)
  let bestDist = Infinity;
  let bestX = -1,
    bestY = -1;
  for (let y = 0; y < GRID; y++) {
    for (let x = 0; x < GRID; x++) {
      if (!visited[y * GRID + x]) continue;
      if (x === x0 && y === y0) continue; // skip start
      const dist = Math.max(Math.abs(x - x1), Math.abs(y - y1));
      if (dist < bestDist) {
        bestDist = dist;
        bestX = x;
        bestY = y;
      }
    }
  }
  if (bestX === -1) return []; // no reachable tile found (shouldn't happen)
  return buildPath(bestX, bestY);
}

/**
 * Returns true if every tile along the Chebyshev path from (x1,y1) to (x2,y2)
 * is walkable. Catches transparent-but-unwalkable tiles (windows, closed doors)
 * that LOS checks alone would miss.
 */
export function pathIsWalkable(
  x1: number,
  y1: number,
  x2: number,
  y2: number,
  room: RoomData,
  objects: ObjDef[],
  exitKeys?: Set<string>,
): boolean {
  return chebyshevPath(x1, y1, x2, y2).every(
    ({ x, y }) => !isTileBlocked(x, y, room, objects, exitKeys),
  );
}

export function buildExitMap(room: RoomData, objects: ObjDef[]): Map<string, ExitTile> {
  const map = new Map<string, ExitTile>();
  for (const ro of room.recorded_objects ?? []) {
    if (ro.detail < 0) continue;
    const obj = objects[ro.type];
    if (!obj?.exit) continue;
    const landX = ro.infox >= 0 ? ro.infox : ro.x;
    const landY = ro.infoy >= 0 ? ro.infoy : ro.y;
    map.set(`${ro.x},${ro.y}`, { destRoom: ro.detail, landX, landY });
  }
  return map;
}
