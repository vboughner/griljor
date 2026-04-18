import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { World, RoomData, ObjDef, PlacementConfig, loadWorld } from '../../world';
import { joinPlayer } from './helpers';

function buildPlacementWorld(opts?: { placement?: PlacementConfig | null; teams?: number }): World {
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );

  const objects: Array<ObjDef | null> = [
    null,
    { _index: 1, name: 'floor', movement: 5, permeable: true, transparent: true },
    {
      _index: 2,
      name: 'sword',
      takeable: true,
      weight: 5,
      weapon: true,
      damage: 30,
      range: 5,
      transparent: true,
    },
    { _index: 3, name: 'potion', takeable: true, weight: 1, health: -20, transparent: true },
    { _index: 4, name: 'wall', movement: 0 },
  ];

  const room0: RoomData = {
    name: 'neutral-room',
    floor: 0,
    team: 0,
    recorded_objects: [],
    spot: spot.map((col) => col.map((cell) => [...cell])),
    exitNorth: -1,
    exitEast: -1,
    exitSouth: -1,
    exitWest: -1,
  };

  const room1: RoomData = {
    name: 'team1-room',
    floor: 0,
    team: 1,
    recorded_objects: [],
    spot: spot.map((col) => col.map((cell) => [...cell])),
    exitNorth: -1,
    exitEast: -1,
    exitSouth: -1,
    exitWest: -1,
  };

  const room2: RoomData = {
    name: 'team2-room',
    floor: 0,
    team: 2,
    recorded_objects: [],
    spot: spot.map((col) => col.map((cell) => [...cell])),
    exitNorth: -1,
    exitEast: -1,
    exitSouth: -1,
    exitWest: -1,
  };

  const defaultPlacement: PlacementConfig = {
    intervalSeconds: 10,
    rules: [
      { mode: 'r', objType: 2, quantity: 1, target: 0 },
      { mode: 't', objType: 3, quantity: 2, target: 1 },
    ],
  };

  return {
    mapName: 'placement-test',
    title: 'Placement Test',
    teams: 2,
    roomCount: 3,
    rooms: [room0, room1, room2],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
    placement: opts?.placement !== undefined ? opts.placement : defaultPlacement,
  };
}

describe('periodic item placement', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  it('places items after interval elapses', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 10,
        rules: [{ mode: 'r', objType: 2, quantity: 1, target: 0 }],
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(10_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBeGreaterThan(0);
    expect(added[0].room).toBe(0);
    expect(added[0].item.type).toBe(2);
  });

  it('does not place items before interval elapses', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 10,
        rules: [{ mode: 'r', objType: 2, quantity: 1, target: 0 }],
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(9_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBe(0);
  });

  it('scales interval by player count', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 10,
        rules: [{ mode: 'r', objType: 2, quantity: 1, target: 0 }],
      },
    });
    session = new GameSession(world);
    joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.flush();

    // 10s / 2 players = 5s effective interval
    vi.advanceTimersByTime(5_000);

    const added = bob.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBeGreaterThan(0);
  });

  it('does not place items when no players are connected', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 'r', objType: 2, quantity: 1, target: 0 }],
      },
    });
    session = new GameSession(world);

    vi.advanceTimersByTime(20_000);

    // Join after timer has been running — should not have accumulated placements
    const alice = joinPlayer(session, 'Alice');
    // No ITEM_ADDED in the ITEMS_SYNC or other initial messages about room 0 extra items
    // (only the initial room items from recorded_objects should be present)
    const sync = alice.ws.lastOfType('ITEMS_SYNC');
    expect(sync?.items.length ?? 0).toBe(0);
  });

  it('places correct quantity per rule', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 'r', objType: 3, quantity: 3, target: 0 }],
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(5_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBe(3);
    for (const msg of added) {
      expect(msg.item.type).toBe(3);
      expect(msg.room).toBe(0);
    }
  });

  it('team-based rules only place in matching team rooms', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 't', objType: 2, quantity: 1, target: 1 }],
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Run many cycles to get statistical confidence
    vi.advanceTimersByTime(50_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBeGreaterThan(0);
    for (const msg of added) {
      expect(msg.room).toBe(1); // room1 is team=1
    }
  });

  it('room-specific rules only place in specified room', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 'r', objType: 2, quantity: 1, target: 2 }],
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(50_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBeGreaterThan(0);
    for (const msg of added) {
      expect(msg.room).toBe(2);
    }
  });

  it('skips invalid object types gracefully', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 'r', objType: 999, quantity: 1, target: 0 }],
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(10_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBe(0);
  });

  it('skips non-takeable object types', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 'r', objType: 4, quantity: 1, target: 0 }], // wall, not takeable
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(10_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBe(0);
  });

  it('placement timer is cleaned up on destroy', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 'r', objType: 2, quantity: 1, target: 0 }],
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    session.destroy();

    vi.advanceTimersByTime(20_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBe(0);
  });

  it('no placement timer when world has no placement config', () => {
    const world = buildPlacementWorld({ placement: null });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(60_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBe(0);
  });

  it('does not place items on tiles that already have items', () => {
    // Use a room with only 1 walkable tile to force collision
    const tinySpot: number[][][] = Array.from({ length: 20 }, (_, x) =>
      Array.from(
        { length: 20 },
        (_, y) => (x === 0 && y === 0 ? [1, 0] : [4, 0]), // only (0,0) is walkable
      ),
    );
    const tinyRoom: RoomData = {
      name: 'tiny',
      floor: 0,
      team: 0,
      recorded_objects: [],
      spot: tinySpot,
      exitNorth: -1,
      exitEast: -1,
      exitSouth: -1,
      exitWest: -1,
    };
    const objects: Array<ObjDef | null> = [
      null,
      { _index: 1, name: 'floor', movement: 5, transparent: true },
      { _index: 2, name: 'sword', takeable: true, weight: 5, transparent: true },
      null,
      { _index: 4, name: 'wall', movement: 0 },
    ];
    const world: World = {
      mapName: 'tiny-test',
      title: 'Tiny Test',
      teams: 0,
      roomCount: 1,
      rooms: [tinyRoom],
      objects,
      resetOnEmpty: false,
      resetAfterSeconds: 30,
      maxPlayers: 16,
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 'r', objType: 2, quantity: 1, target: 0 }],
      },
    };
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    // Move alice off the only walkable tile
    alice.ws.flush();

    // First cycle should place an item
    vi.advanceTimersByTime(5_000);
    const firstBatch = alice.ws.messagesOfType('ITEM_ADDED');

    alice.ws.flush();
    // Second cycle: tile already occupied by an item — should not place another
    vi.advanceTimersByTime(5_000);
    const secondBatch = alice.ws.messagesOfType('ITEM_ADDED');

    // At most 1 item should ever be placed (the tile is taken after the first)
    expect(firstBatch.length + secondBatch.length).toBeLessThanOrEqual(1);
  });

  it('skips team rule when no rooms match the team', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 't', objType: 2, quantity: 1, target: 99 }], // no team 99
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(10_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBe(0);
  });

  it('skips room rule when room index is out of bounds', () => {
    const world = buildPlacementWorld({
      placement: {
        intervalSeconds: 5,
        rules: [{ mode: 'r', objType: 2, quantity: 1, target: 999 }],
      },
    });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(10_000);

    const added = alice.ws.messagesOfType('ITEM_ADDED');
    expect(added.length).toBe(0);
  });
});

describe('loadWorld placement validation', () => {
  it('filters out rules referencing unknown or non-takeable objects', async () => {
    // hack1.pla references default.obj IDs (106, 183, 201, 227) but the map uses main.obj,
    // where those IDs map to different objects. Of the 6 rules:
    //   obj 106 (no name) — not takeable → filtered
    //   obj 183 (stairs)  — not takeable → filtered (×3 rules)
    //   obj 201 (hand gun) — takeable → kept (×1 rule, originally meant to be hand grenade)
    //   obj 227 (no name) — not takeable → filtered
    const world = await loadWorld('hack1');
    expect(world.placement).not.toBeNull();
    // Only the obj 201 rule survives (wrong item, but takeable)
    expect(world.placement!.rules.length).toBe(1);
    expect(world.placement!.rules[0].objType).toBe(201);
  });

  it('keeps valid placement rules intact', async () => {
    // battle uses default.obj and its .pla references valid takeable items
    const world = await loadWorld('battle');
    expect(world.placement).not.toBeNull();
    expect(world.placement!.rules.length).toBeGreaterThan(0);
  });
});
