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
