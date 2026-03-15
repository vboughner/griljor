import { describe, it, expect } from 'vitest';
import { chebyshevPath, tileViewBlocked, spotIsVisible } from '../session';
import { ObjDef, RoomData } from '../world';

// ── helpers ─────────────────────────────────────────────────────────────────

function makeObj(overrides: Partial<ObjDef> = {}): ObjDef {
  return { _index: 1, ...overrides } as ObjDef;
}

/** Minimal empty room with no spots and no recorded_objects */
function emptyRoom(): RoomData {
  return { name: 'test', floor: 0, team: 0, recorded_objects: [] };
}

/** Build a room whose spot array contains a single set cell at (x,y) */
function roomWithSpot(x: number, y: number, flId: number, wlId: number): RoomData {
  const spot: number[][][] = [];
  spot[x] = [];
  spot[x][y] = [flId, wlId];
  return { name: 'test', floor: 0, team: 0, recorded_objects: [], spot };
}

// ── chebyshevPath ────────────────────────────────────────────────────────────

describe('chebyshevPath', () => {
  it('straight horizontal path (0,0)→(3,0)', () => {
    expect(chebyshevPath(0, 0, 3, 0)).toEqual([
      { x: 1, y: 0 },
      { x: 2, y: 0 },
      { x: 3, y: 0 },
    ]);
  });

  it('straight vertical path (0,0)→(0,3)', () => {
    expect(chebyshevPath(0, 0, 0, 3)).toEqual([
      { x: 0, y: 1 },
      { x: 0, y: 2 },
      { x: 0, y: 3 },
    ]);
  });

  it('diagonal path (0,0)→(2,2)', () => {
    expect(chebyshevPath(0, 0, 2, 2)).toEqual([
      { x: 1, y: 1 },
      { x: 2, y: 2 },
    ]);
  });

  it('L-shaped path (0,0)→(3,1) — diagonal first, then straight', () => {
    expect(chebyshevPath(0, 0, 3, 1)).toEqual([
      { x: 1, y: 1 },
      { x: 2, y: 1 },
      { x: 3, y: 1 },
    ]);
  });

  it('same tile returns empty array', () => {
    expect(chebyshevPath(2, 2, 2, 2)).toEqual([]);
  });

  it('adjacent tile returns single step', () => {
    expect(chebyshevPath(0, 0, 1, 0)).toEqual([{ x: 1, y: 0 }]);
  });
});

// ── tileViewBlocked ──────────────────────────────────────────────────────────

describe('tileViewBlocked', () => {
  it('empty tile with no spot data returns false', () => {
    const room = emptyRoom();
    expect(tileViewBlocked(room, [], 5, 5)).toBe(false);
  });

  it('floor tile with transparent:true returns false', () => {
    const room = roomWithSpot(1, 1, 1, 0);
    const objects: Array<ObjDef | null> = [null, makeObj({ _index: 1, transparent: true })];
    expect(tileViewBlocked(room, objects, 1, 1)).toBe(false);
  });

  it('floor tile without transparent (opaque) returns true', () => {
    const room = roomWithSpot(1, 1, 1, 0);
    const objects: Array<ObjDef | null> = [null, makeObj({ _index: 1 })];
    expect(tileViewBlocked(room, objects, 1, 1)).toBe(true);
  });

  it('wall tile without transparent (opaque) returns true', () => {
    const room = roomWithSpot(2, 2, 0, 1);
    const objects: Array<ObjDef | null> = [null, makeObj({ _index: 1 })];
    expect(tileViewBlocked(room, objects, 2, 2)).toBe(true);
  });

  it('floor transparent + wall opaque returns true (wall blocks)', () => {
    const room = roomWithSpot(1, 1, 1, 2);
    const objects: Array<ObjDef | null> = [
      null,
      makeObj({ _index: 1, transparent: true }), // floor — transparent
      makeObj({ _index: 2 }), // wall — opaque
    ];
    expect(tileViewBlocked(room, objects, 1, 1)).toBe(true);
  });

  it('floor opaque + wall transparent returns true (floor blocks)', () => {
    const room = roomWithSpot(1, 1, 1, 2);
    const objects: Array<ObjDef | null> = [
      null,
      makeObj({ _index: 1 }), // floor — opaque
      makeObj({ _index: 2, transparent: true }), // wall — transparent
    ];
    expect(tileViewBlocked(room, objects, 1, 1)).toBe(true);
  });

  it('recorded_object without transparent returns true', () => {
    const room: RoomData = {
      name: 'test',
      floor: 0,
      team: 0,
      recorded_objects: [{ x: 3, y: 3, type: 1, detail: 0 }],
    };
    const objects: Array<ObjDef | null> = [null, makeObj({ _index: 1 })];
    expect(tileViewBlocked(room, objects, 3, 3)).toBe(true);
  });

  it('recorded_object with transparent:true returns false', () => {
    const room: RoomData = {
      name: 'test',
      floor: 0,
      team: 0,
      recorded_objects: [{ x: 3, y: 3, type: 1, detail: 0 }],
    };
    const objects: Array<ObjDef | null> = [null, makeObj({ _index: 1, transparent: true })];
    expect(tileViewBlocked(room, objects, 3, 3)).toBe(false);
  });

  it('object ID with null entry in objects array returns false', () => {
    const room = roomWithSpot(1, 1, 1, 0);
    const objects: Array<ObjDef | null> = [null, null]; // index 1 = null
    expect(tileViewBlocked(room, objects, 1, 1)).toBe(false);
  });
});

// ── spotIsVisible ────────────────────────────────────────────────────────────

describe('spotIsVisible', () => {
  // object 1: transparent floor; object 2: opaque terrain (no transparent field)
  const objects: Array<ObjDef | null> = [
    null,
    makeObj({ _index: 1, transparent: true }),
    makeObj({ _index: 2 }),
  ];

  /** 10×10 open room — all spots transparent (object 1) */
  function openRoom(): RoomData {
    const spot: number[][][] = [];
    for (let x = 0; x < 10; x++) {
      spot[x] = [];
      for (let y = 0; y < 10; y++) {
        spot[x][y] = [1, 0]; // transparent floor, no wall
      }
    }
    return { name: 'test', floor: 0, team: 0, recorded_objects: [], spot };
  }

  it('same tile is always visible', () => {
    expect(spotIsVisible(openRoom(), objects, 3, 3, 3, 3)).toBe(true);
  });

  it('adjacent tile (1 Chebyshev step) is always visible', () => {
    const room = openRoom();
    // put opaque object on the target tile — still visible because it's adjacent
    room.spot![5][5] = [2, 0];
    expect(spotIsVisible(room, objects, 4, 5, 5, 5)).toBe(true);
  });

  it('two players in open space with no blockers: visible', () => {
    expect(spotIsVisible(openRoom(), objects, 1, 1, 8, 8)).toBe(true);
  });

  it('opaque wall tile between two positions: not visible', () => {
    const room = openRoom();
    room.spot![5][5] = [2, 0]; // opaque tile in the path
    expect(spotIsVisible(room, objects, 3, 3, 7, 7)).toBe(false);
  });

  it('player on opaque floor tile 2 tiles away: not visible', () => {
    const room = openRoom();
    room.spot![3][0] = [2, 0]; // opaque at target (2 steps away)
    expect(spotIsVisible(room, objects, 1, 0, 3, 0)).toBe(false);
  });

  it('player on opaque floor tile 1 tile away (adjacent): visible', () => {
    const room = openRoom();
    room.spot![2][0] = [2, 0]; // opaque at target (1 step away — adjacent)
    expect(spotIsVisible(room, objects, 1, 0, 2, 0)).toBe(true);
  });

  it('looker on opaque tile, clear path to target: visible (own tile not checked)', () => {
    const room = openRoom();
    room.spot![1][1] = [2, 0]; // opaque at looker's own tile — must not block
    expect(spotIsVisible(room, objects, 1, 1, 5, 5)).toBe(true);
  });

  it('three tiles apart with opaque in middle: not visible', () => {
    const room = openRoom();
    room.spot![5][0] = [2, 0]; // opaque middle tile
    expect(spotIsVisible(room, objects, 3, 0, 7, 0)).toBe(false);
  });

  it('LOS is directional: target on opaque tile blocks observer, not the reverse', () => {
    // B is 3 tiles from A, standing on an opaque tile (simulates hiding in forest)
    const room = openRoom();
    room.spot![4][0] = [2, 0]; // opaque tile at B's position
    // A cannot see B: B's opaque tile is the target, it IS checked
    expect(spotIsVisible(room, objects, 1, 0, 4, 0)).toBe(false);
    // B can see A: B's own tile is excluded, A's tile is transparent
    expect(spotIsVisible(room, objects, 4, 0, 1, 0)).toBe(true);
  });
});
