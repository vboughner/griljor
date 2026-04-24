import { describe, it, expect } from 'vitest';
import { tileIsVisible } from '../los';
import { ObjDef, RoomData } from '../types';

/**
 * Tests for dark room visibility logic.
 *
 * The Game class computes dark room visibility as:
 *   visible = hasLOS && (dist <= lightRadius || tileGlows)
 *
 * Since the LOS part is already tested in los.test.ts,
 * we test the distance + glow logic here using the same primitives.
 */

function makeObj(overrides: Partial<ObjDef> = {}): ObjDef {
  return { _index: 1, ...overrides } as ObjDef;
}

function openRoom(): RoomData {
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );
  return {
    name: 'test',
    floor: 1,
    team: 0,
    exit_north: -1,
    exit_east: -1,
    exit_south: -1,
    exit_west: -1,
    appearance: 0,
    dark: 1,
    spot,
    recorded_objects: [],
  };
}

function chebyshevDist(x1: number, y1: number, x2: number, y2: number): number {
  return Math.max(Math.abs(x2 - x1), Math.abs(y2 - y1));
}

/** Simulate the dark room visibility check the Game class uses. */
function isDarkTileVisible(
  room: RoomData,
  objects: ObjDef[],
  px: number,
  py: number,
  tx: number,
  ty: number,
  lightRadius: number,
): boolean {
  const hasLos = tileIsVisible(room, objects, px, py, tx, ty);
  if (!hasLos) return false;
  const dist = chebyshevDist(px, py, tx, ty);
  if (dist <= lightRadius) return true;
  // Check if tile glows
  const cell = room.spot?.[tx]?.[ty];
  if (cell) {
    const flObj = cell[0] > 0 ? objects[cell[0]] : null;
    if (flObj?.glows) return true;
    const wlObj = cell[1] > 0 ? objects[cell[1]] : null;
    if (wlObj?.glows) return true;
  }
  return false;
}

describe('dark room visibility', () => {
  const floor = makeObj({ _index: 1, movement: 5, transparent: true });
  const glowingStairs = makeObj({ _index: 2, movement: 2, transparent: true, glows: true });
  const objects: ObjDef[] = [makeObj({ _index: 0 }), floor, glowingStairs];

  it('tiles within light radius are visible', () => {
    const room = openRoom();
    expect(isDarkTileVisible(room, objects, 10, 10, 11, 10, 2)).toBe(true); // dist=1
    expect(isDarkTileVisible(room, objects, 10, 10, 12, 12, 2)).toBe(true); // dist=2
  });

  it('tiles beyond light radius are not visible', () => {
    const room = openRoom();
    expect(isDarkTileVisible(room, objects, 10, 10, 15, 10, 2)).toBe(false); // dist=5
    expect(isDarkTileVisible(room, objects, 10, 10, 13, 10, 2)).toBe(false); // dist=3
  });

  it('glowing tiles are visible beyond light radius if LOS exists', () => {
    const room = openRoom();
    // Place glowing stairs at (15, 10) in floor layer
    room.spot![15][10] = [2, 0]; // object index 2 = glowingStairs
    expect(isDarkTileVisible(room, objects, 10, 10, 15, 10, 2)).toBe(true);
  });

  it('larger flashlight extends visible range', () => {
    const room = openRoom();
    // dist=5, radius=6 → visible
    expect(isDarkTileVisible(room, objects, 10, 10, 15, 10, 6)).toBe(true);
    // dist=7, radius=6 → not visible
    expect(isDarkTileVisible(room, objects, 10, 10, 17, 10, 6)).toBe(false);
  });

  it('own tile is always visible', () => {
    const room = openRoom();
    expect(isDarkTileVisible(room, objects, 10, 10, 10, 10, 0)).toBe(true);
  });

  it('effective light radius picks best flashlight from inventory', () => {
    // Replicate the logic from Game.getEffectiveLightRadius
    const BASE_DARK_RADIUS = 2;
    const inv = [
      { type: 10, quantity: 1 }, // no flashlight
      { type: 11, quantity: 1 }, // flashlight: 3
      null,
      { type: 12, quantity: 1 }, // flashlight: 7
    ];
    const testObjects: Array<ObjDef | null> = Array(13).fill(null);
    testObjects[10] = makeObj({ name: 'sword' });
    testObjects[11] = makeObj({ name: 'small torch', flashlight: 3 });
    testObjects[12] = makeObj({ name: 'lasergun', flashlight: 7 });

    let best = 0;
    for (const item of inv) {
      if (!item) continue;
      const fl = testObjects[item.type]?.flashlight ?? 0;
      if (fl > best) best = fl;
    }
    const radius = best > 0 ? best : BASE_DARK_RADIUS;
    expect(radius).toBe(7);
  });

  it('returns base radius when no flashlight items in inventory', () => {
    const BASE_DARK_RADIUS = 2;
    const inv = [{ type: 10, quantity: 1 }];
    const testObjects: Array<ObjDef | null> = Array(11).fill(null);
    testObjects[10] = makeObj({ name: 'sword' });

    let best = 0;
    for (const item of inv) {
      if (!item) continue;
      const fl = testObjects[item.type]?.flashlight ?? 0;
      if (fl > best) best = fl;
    }
    const radius = best > 0 ? best : BASE_DARK_RADIUS;
    expect(radius).toBe(BASE_DARK_RADIUS);
  });
});
