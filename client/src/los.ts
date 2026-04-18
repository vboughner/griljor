import { ObjDef, RoomData } from './types';

// ── Path helpers ─────────────────────────────────────────────────────────────

/**
 * Walk from (x1,y1) toward (x2,y2) one Chebyshev step at a time.
 * Returns each step's tile, NOT including the start tile (x1,y1).
 * Stops when (x2,y2) is reached.
 * Used for movement pathfinding (not line-of-sight).
 */
export function chebyshevPath(
  x1: number,
  y1: number,
  x2: number,
  y2: number,
): Array<{ x: number; y: number }> {
  const path: Array<{ x: number; y: number }> = [];
  let cx = x1;
  let cy = y1;
  while (cx !== x2 || cy !== y2) {
    if (cx !== x2) cx += Math.sign(x2 - cx);
    if (cy !== y2) cy += Math.sign(y2 - cy);
    path.push({ x: cx, y: cy });
  }
  return path;
}

// ── Line-of-sight helpers ──────────────────────────────────────────────────

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
  // We use integers scaled by adx*ady to avoid floating point entirely.
  // tMaxX/tMaxY = distance to first vertical/horizontal grid crossing.
  // tDeltaX/tDeltaY = distance between successive crossings.
  let tMaxX = ady; // first vertical crossing at 0.5/adx scaled = ady
  let tMaxY = adx; // first horizontal crossing at 0.5/ady scaled = adx
  const tDeltaX = 2 * ady; // subsequent vertical crossings
  const tDeltaY = 2 * adx; // subsequent horizontal crossings

  while (cx !== x2 || cy !== y2) {
    if (tMaxX < tMaxY) {
      cx += sx;
      tMaxX += tDeltaX;
    } else if (tMaxY < tMaxX) {
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

/**
 * Returns true if any object on tile (x,y) does NOT have transparent:true,
 * meaning it blocks line of sight.
 */
export function tileViewBlocked(
  room: RoomData,
  objects: Array<ObjDef | null>,
  x: number,
  y: number,
): boolean {
  const cell = room.spot?.[x]?.[y];
  if (cell) {
    const [flId, wlId] = cell;
    const flObj = flId > 0 ? objects[flId] : null;
    if (flObj != null && flObj.transparent !== true) return true;
    const wlObj = wlId > 0 ? objects[wlId] : null;
    if (wlObj != null && wlObj.transparent !== true) return true;
  }
  for (const ro of room.recorded_objects ?? []) {
    if (ro.x === x && ro.y === y) {
      const roObj = objects[ro.type];
      if (roObj != null && roObj.transparent !== true) return true;
    }
  }
  return false;
}

/**
 * Returns true if tile (x2,y2) is visible from tile (x1,y1).
 * Adjacent tiles (1 Chebyshev step) are always visible.
 * The looker's own tile (x1,y1) is not checked; the target tile IS checked.
 * Used for player-to-player visibility (can you see someone hiding in a forest?).
 */
export function spotIsVisible(
  room: RoomData,
  objects: Array<ObjDef | null>,
  x1: number,
  y1: number,
  x2: number,
  y2: number,
): boolean {
  if (x1 === x2 && y1 === y2) return true;
  const path = losRayTiles(x1, y1, x2, y2);
  if (path.length <= 1) return true; // adjacent — always visible
  for (const { x, y } of path) {
    if (tileViewBlocked(room, objects, x, y)) return false;
  }
  return true;
}

/**
 * Returns true if tile (x2,y2) can be seen from tile (x1,y1) for rendering.
 * Unlike spotIsVisible, the target tile itself is NOT checked — a wall is
 * visible if the path to it is clear, even though you cannot see through it.
 */
export function tileIsVisible(
  room: RoomData,
  objects: Array<ObjDef | null>,
  x1: number,
  y1: number,
  x2: number,
  y2: number,
): boolean {
  if (x1 === x2 && y1 === y2) return true;
  const path = losRayTiles(x1, y1, x2, y2);
  if (path.length <= 1) return true; // adjacent — always visible
  // Check all intermediate tiles, but NOT the target itself
  for (const { x, y } of path.slice(0, -1)) {
    if (tileViewBlocked(room, objects, x, y)) return false;
  }
  return true;
}
