import { describe, it, expect } from 'vitest';
import {
  isTileBlocked,
  buildExitMap,
  computeBfsPath,
  computeBfsPathToNearest,
} from '../game-utils';
import { RoomData, ObjDef } from '../types';

/** Helpers to build minimal test fixtures */
function emptyRoom(): RoomData {
  const spot: [number, number][][] = [];
  for (let x = 0; x < 20; x++) {
    spot[x] = [];
    for (let y = 0; y < 20; y++) spot[x][y] = [0, 0];
  }
  return { spot, recorded_objects: [] } as unknown as RoomData;
}

function makeObjects(defs: Record<number, Partial<ObjDef>>): ObjDef[] {
  const arr: ObjDef[] = [];
  for (const [id, def] of Object.entries(defs)) {
    arr[Number(id)] = { _index: Number(id), ...def } as ObjDef;
  }
  return arr;
}

function makeRecObj(
  x: number,
  y: number,
  type: number,
  detail: number,
  infox: number,
  infoy: number,
) {
  return { x, y, type, detail, infox, infoy, zinger: 0, extra: [] };
}

describe('isTileBlocked', () => {
  it('void tile [0,0] is walkable', () => {
    const room = emptyRoom();
    expect(isTileBlocked(5, 5, room, [])).toBe(false);
  });

  it('tile with wall object lacking movement is blocked', () => {
    const room = emptyRoom();
    room.spot![3][4] = [0, 42];
    const objects = makeObjects({ 42: { movement: 0 } });
    expect(isTileBlocked(3, 4, room, objects)).toBe(true);
  });

  it('tile with wall object with movement>0 is walkable', () => {
    const room = emptyRoom();
    room.spot![2][2] = [0, 7];
    const objects = makeObjects({ 7: { movement: 5 } });
    expect(isTileBlocked(2, 2, room, objects)).toBe(false);
  });

  it('floor object with no movement is blocked', () => {
    const room = emptyRoom();
    room.spot![1][1] = [10, 0];
    const objects = makeObjects({ 10: {} }); // no movement field
    expect(isTileBlocked(1, 1, room, objects)).toBe(true);
  });

  it('exit tile override: blocked tile becomes walkable', () => {
    const room = emptyRoom();
    room.spot![3][4] = [0, 42];
    const objects = makeObjects({ 42: { movement: 0 } });
    const exitKeys = new Set(['3,4']);
    expect(isTileBlocked(3, 4, room, objects, exitKeys)).toBe(false);
  });

  it('recorded object without movement blocks tile', () => {
    const room = emptyRoom();
    room.recorded_objects = [makeRecObj(5, 5, 99, 0, -1, -1)];
    const objects = makeObjects({ 99: { movement: 0 } });
    expect(isTileBlocked(5, 5, room, objects)).toBe(true);
  });
});

describe('buildExitMap', () => {
  it('returns empty map for room with no recorded objects', () => {
    const room = emptyRoom();
    expect(buildExitMap(room, [])).toEqual(new Map());
  });

  it('ignores non-exit objects', () => {
    const room = emptyRoom();
    room.recorded_objects = [makeRecObj(2, 3, 5, 10, -1, -1)];
    const objects = makeObjects({ 5: { exit: false } });
    expect(buildExitMap(room, objects)).toEqual(new Map());
  });

  it('maps exit tile to destination room and landing coords', () => {
    const room = emptyRoom();
    room.recorded_objects = [makeRecObj(3, 4, 7, 2, 10, 11)];
    const objects = makeObjects({ 7: { exit: true } });
    const map = buildExitMap(room, objects);
    expect(map.get('3,4')).toEqual({ destRoom: 2, landX: 10, landY: 11 });
  });

  it('uses tile coords as landing when infox/infoy are -1', () => {
    const room = emptyRoom();
    room.recorded_objects = [makeRecObj(5, 6, 7, 3, -1, -1)];
    const objects = makeObjects({ 7: { exit: true } });
    const map = buildExitMap(room, objects);
    expect(map.get('5,6')).toEqual({ destRoom: 3, landX: 5, landY: 6 });
  });

  it('ignores objects with negative detail (detail < 0)', () => {
    const room = emptyRoom();
    room.recorded_objects = [makeRecObj(1, 1, 7, -1, -1, -1)];
    const objects = makeObjects({ 7: { exit: true } });
    expect(buildExitMap(room, objects)).toEqual(new Map());
  });
});

describe('computeBfsPath', () => {
  it('returns empty array when already at target', () => {
    const room = emptyRoom();
    expect(computeBfsPath(5, 5, 5, 5, room, [])).toEqual([]);
  });

  it('returns direct path in open space', () => {
    const room = emptyRoom();
    const path = computeBfsPath(5, 5, 8, 5, room, []);
    expect(path[path.length - 1]).toEqual({ x: 8, y: 5 });
    expect(path[0]).toEqual({ x: 6, y: 5 });
    expect(path.length).toBe(3);
  });

  it('returns empty array when target is unreachable', () => {
    const room = emptyRoom();
    const objects = makeObjects({ 99: { movement: 0 } });
    for (const [dx, dy] of [
      [0, 1],
      [0, -1],
      [1, 0],
      [-1, 0],
      [1, 1],
      [1, -1],
      [-1, 1],
      [-1, -1],
    ]) {
      room.spot![10 + dx][10 + dy] = [0, 99];
    }
    expect(computeBfsPath(5, 5, 10, 10, room, objects)).toEqual([]);
  });

  it('navigates around a wall — no oscillation', () => {
    const room = emptyRoom();
    const objects = makeObjects({ 99: { movement: 0 } });
    for (let y = 3; y <= 7; y++) room.spot![5][y] = [0, 99];
    const path = computeBfsPath(4, 5, 7, 5, room, objects);
    expect(path[path.length - 1]).toEqual({ x: 7, y: 5 });
    for (const step of path) {
      expect(isTileBlocked(step.x, step.y, room, objects)).toBe(false);
    }
    let cx = 4,
      cy = 5;
    for (const step of path) {
      expect(Math.abs(step.x - cx)).toBeLessThanOrEqual(1);
      expect(Math.abs(step.y - cy)).toBeLessThanOrEqual(1);
      cx = step.x;
      cy = step.y;
    }
  });

  it('path does not include the starting tile', () => {
    const room = emptyRoom();
    const path = computeBfsPath(3, 3, 5, 3, room, []);
    expect(path.every((p) => !(p.x === 3 && p.y === 3))).toBe(true);
  });
});

describe('computeBfsPathToNearest', () => {
  it('returns direct path when target is reachable', () => {
    const room = emptyRoom();
    const path = computeBfsPathToNearest(5, 5, 8, 5, room, []);
    expect(path[path.length - 1]).toEqual({ x: 8, y: 5 });
  });

  it('returns empty array when already at target', () => {
    const room = emptyRoom();
    expect(computeBfsPathToNearest(5, 5, 5, 5, room, [])).toEqual([]);
  });

  it('navigates to adjacent tile when target is a wall', () => {
    const room = emptyRoom();
    const objects = makeObjects({ 99: { movement: 0 } });
    // Place a wall at (10, 10)
    room.spot![10][10] = [0, 99];
    // Player at (8, 10), clicking the wall at (10, 10)
    const path = computeBfsPathToNearest(8, 10, 10, 10, room, objects);
    // Should end at the closest reachable tile to the wall (e.g. (9, 10))
    expect(path.length).toBeGreaterThan(0);
    const dest = path[path.length - 1];
    expect(isTileBlocked(dest.x, dest.y, room, objects)).toBe(false);
    const chebyDist = Math.max(Math.abs(dest.x - 10), Math.abs(dest.y - 10));
    expect(chebyDist).toBe(1);
  });

  it('finds nearest reachable tile when completely surrounded by walls', () => {
    const room = emptyRoom();
    const objects = makeObjects({ 99: { movement: 0 } });
    // Surround (10, 10) with walls on all 8 neighbors — player can't get adjacent
    for (const [dx, dy] of [
      [0, 0],
      [1, 0],
      [-1, 0],
      [0, 1],
      [0, -1],
      [1, 1],
      [1, -1],
      [-1, 1],
      [-1, -1],
    ]) {
      room.spot![10 + dx][10 + dy] = [0, 99];
    }
    const path = computeBfsPathToNearest(5, 5, 10, 10, room, objects);
    expect(path.length).toBeGreaterThan(0);
    const dest = path[path.length - 1];
    expect(isTileBlocked(dest.x, dest.y, room, objects)).toBe(false);
    // Should be at Chebyshev distance 2 from (10,10) since distance-1 tiles are walled
    const chebyDist = Math.max(Math.abs(dest.x - 10), Math.abs(dest.y - 10));
    expect(chebyDist).toBe(2);
  });

  it('navigates adjacent to a closed door when the door tile is clicked', () => {
    // A closed door occupies the wall slot with movement: 0 — identical to a wall
    // for pathfinding purposes. Clicking the door should move the player to the
    // nearest adjacent tile so they can get into position to open it.
    const room = emptyRoom();
    // Object 99: closed door (movement:0, blocked)
    const objects = makeObjects({ 99: { movement: 0 } });
    // Place door at (10, 8) in the wall slot; all surrounding tiles are void (walkable)
    room.spot![10][8] = [0, 99];
    // Verify door is blocked and adjacent tile is walkable
    expect(isTileBlocked(10, 8, room, objects)).toBe(true);
    expect(isTileBlocked(10, 7, room, objects)).toBe(false);
    // Player at (10, 5), clicks the door at (10, 8)
    const path = computeBfsPathToNearest(10, 5, 10, 8, room, objects);
    expect(path.length).toBeGreaterThan(0);
    const dest = path[path.length - 1];
    // Destination must be walkable
    expect(isTileBlocked(dest.x, dest.y, room, objects)).toBe(false);
    // Destination must be adjacent (Chebyshev distance 1) to the door at (10, 8)
    const chebyDist = Math.max(Math.abs(dest.x - 10), Math.abs(dest.y - 8));
    expect(chebyDist).toBe(1);
    // Player should have moved closer to the door (not stayed at y=5)
    expect(dest.y).toBeGreaterThan(5);
  });
});
