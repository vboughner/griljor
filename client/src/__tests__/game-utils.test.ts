import { describe, it, expect } from 'vitest';
import { isTileBlocked, findNextStep, computeBresenhamPath, buildExitMap } from '../game-utils';
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

function makeRecObj(x: number, y: number, type: number, detail: number, infox: number, infoy: number) {
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

describe('computeBresenhamPath', () => {
  it('horizontal line', () => {
    const path = computeBresenhamPath(0, 0, 3, 0);
    expect(path).toEqual([{ x: 1, y: 0 }, { x: 2, y: 0 }, { x: 3, y: 0 }]);
  });

  it('vertical line', () => {
    const path = computeBresenhamPath(0, 0, 0, 3);
    expect(path).toEqual([{ x: 0, y: 1 }, { x: 0, y: 2 }, { x: 0, y: 3 }]);
  });

  it('diagonal line (45°)', () => {
    const path = computeBresenhamPath(0, 0, 2, 2);
    expect(path).toEqual([{ x: 1, y: 1 }, { x: 2, y: 2 }]);
  });

  it('same point returns empty path', () => {
    expect(computeBresenhamPath(5, 5, 5, 5)).toEqual([]);
  });

  it('last point in path equals target', () => {
    const path = computeBresenhamPath(1, 1, 7, 4);
    expect(path[path.length - 1]).toEqual({ x: 7, y: 4 });
  });
});

describe('findNextStep', () => {
  it('returns null when already at target', () => {
    const room = emptyRoom();
    expect(findNextStep(5, 5, 5, 5, room, [])).toBeNull();
  });

  it('returns single step for adjacent target', () => {
    const room = emptyRoom();
    const step = findNextStep(5, 5, 6, 5, room, []);
    expect(step).toEqual([1, 0]);
  });

  it('finds path through open space', () => {
    const room = emptyRoom();
    const step = findNextStep(0, 0, 5, 0, room, []);
    expect(step).toEqual([1, 0]);
  });

  it('returns null when target is completely surrounded by walls', () => {
    const room = emptyRoom();
    const objects = makeObjects({ 99: { movement: 0 } });
    // Wall off target (10,10) completely
    for (const [dx, dy] of [[0, 1], [0, -1], [1, 0], [-1, 0], [1, 1], [1, -1], [-1, 1], [-1, -1]]) {
      room.spot![10 + dx][10 + dy] = [0, 99];
    }
    room.spot![10][10] = [99, 0]; // target itself is open but surrounded
    const step = findNextStep(5, 5, 10, 10, room, objects);
    expect(step).toBeNull();
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
