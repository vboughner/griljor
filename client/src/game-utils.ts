import { RoomData, ObjDef } from './types';

const GRID = 20;

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
  // Recorded objects (doors, etc.): block if movement absent (=0) or explicitly 0
  for (const ro of room.recorded_objects ?? []) {
    if (ro.x === x && ro.y === y && ro.type > 0) {
      if (!((objects[ro.type]?.movement ?? 0) > 0)) return true;
    }
  }
  return false;
}

/**
 * BFS to find the first step from (sx,sy) toward (tx,ty) through walkable tiles.
 * Returns [dx,dy] for the next step, or null if no path or already at target.
 */
export function findNextStep(
  sx: number,
  sy: number,
  tx: number,
  ty: number,
  room: RoomData,
  objects: ObjDef[],
  exitKeys?: Set<string>,
): [number, number] | null {
  if (sx === tx && sy === ty) return null;
  const visited = new Uint8Array(GRID * GRID);
  type Node = { x: number; y: number; first: [number, number] };
  const queue: Node[] = [];
  visited[sy * GRID + sx] = 1;
  for (const [dx, dy] of STEP_DIRS) {
    const nx = sx + dx,
      ny = sy + dy;
    if (nx < 0 || nx >= GRID || ny < 0 || ny >= GRID) continue;
    if (isTileBlocked(nx, ny, room, objects, exitKeys)) continue;
    const k = ny * GRID + nx;
    if (visited[k]) continue;
    visited[k] = 1;
    if (nx === tx && ny === ty) return [dx, dy];
    queue.push({ x: nx, y: ny, first: [dx, dy] });
  }
  while (queue.length > 0) {
    const { x, y, first } = queue.shift()!;
    for (const [dx, dy] of STEP_DIRS) {
      const nx = x + dx,
        ny = y + dy;
      if (nx < 0 || nx >= GRID || ny < 0 || ny >= GRID) continue;
      if (isTileBlocked(nx, ny, room, objects, exitKeys)) continue;
      const k = ny * GRID + nx;
      if (visited[k]) continue;
      visited[k] = 1;
      if (nx === tx && ny === ty) return first;
      queue.push({ x: nx, y: ny, first });
    }
  }
  return null; // no path
}

/**
 * Bresenham path from (x0,y0) to (x1,y1) — returns the sequence of tile
 * positions to visit, distributing diagonal steps evenly throughout so the
 * path looks like a straight line.  Does NOT check for walls; caller handles
 * fallback when a step is blocked.
 */
export function computeBresenhamPath(
  x0: number,
  y0: number,
  x1: number,
  y1: number,
): Array<{ x: number; y: number }> {
  const path: Array<{ x: number; y: number }> = [];
  const adx = Math.abs(x1 - x0),
    ady = Math.abs(y1 - y0);
  const sx = x0 < x1 ? 1 : -1,
    sy = y0 < y1 ? 1 : -1;
  let err = adx - ady;
  let cx = x0,
    cy = y0;
  while (cx !== x1 || cy !== y1) {
    const e2 = 2 * err;
    if (e2 > -ady) {
      err -= ady;
      cx += sx;
    }
    if (e2 < adx) {
      err += adx;
      cy += sy;
    }
    path.push({ x: cx, y: cy });
  }
  return path;
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
