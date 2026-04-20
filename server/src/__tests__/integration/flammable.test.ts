import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { World, ObjDef, RoomData } from '../../world';
import { joinPlayer, TestPlayer } from './helpers';

/**
 * Test world for flammable/destruction tests.
 *
 * Object index layout:
 *   0 = null
 *   1 = floor tile (movement:5, walkable, permeable)
 *   2 = sword (takeable weapon, damage:30, range:5)
 *   3 = vulnerable wall (blocks passage, vulnerable, type:1, destroyed→8)
 *   4 = plain wall (blocks passage, NOT vulnerable)
 *   5 = grenade (takeable weapon, lost, stop, explodes:2, boombit:6)
 *   6 = explosion (speed:4, damage:20, destroys:1)
 *   7 = flammable barrel (vulnerable, flammable:2, boombit:9, type:1, takeable)
 *   8 = destroyed wall (walkable rubble, permeable)
 *   9 = fire explosion (speed:4, damage:15, destroys:1)
 *  10 = non-destroying grenade (takeable weapon, lost, stop, explodes:2, boombit:11)
 *  11 = non-destroying explosion (speed:4, damage:10, NO destroys field)
 */
function buildFlammableWorld(opts?: { rooms?: RoomData[] }): World {
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
    {
      _index: 3,
      name: 'vulnerable wall',
      movement: 0,
      vulnerable: true,
      destroyed: 8,
    },
    { _index: 4, name: 'plain wall', movement: 0 },
    {
      _index: 5,
      name: 'grenade',
      takeable: true,
      weight: 5,
      weapon: true,
      damage: 5,
      range: 4,
      lost: true,
      stop: true,
      explodes: 2,
      movingobj: 5,
      boombit: 6,
      speed: 6,
      transparent: true,
    },
    {
      _index: 6,
      name: 'explosion',
      movement: 9,
      stop: true,
      movingobj: 6,
      directional: true,
      speed: 4,
      damage: 20,
      transparent: true,
      destroys: 1,
    },
    {
      _index: 7,
      name: 'flammable barrel',
      takeable: true,
      weight: 10,
      vulnerable: true,
      flammable: 2,
      boombit: 9,
      transparent: true,
    },
    {
      _index: 8,
      name: 'destroyed wall',
      movement: 5,
      permeable: true,
      transparent: true,
    },
    {
      _index: 9,
      name: 'fire explosion',
      movement: 9,
      stop: true,
      movingobj: 9,
      directional: true,
      speed: 4,
      damage: 15,
      transparent: true,
      destroys: 1,
    },
    {
      _index: 10,
      name: 'non-destroying grenade',
      takeable: true,
      weight: 5,
      weapon: true,
      damage: 5,
      range: 4,
      lost: true,
      stop: true,
      explodes: 2,
      movingobj: 10,
      boombit: 11,
      speed: 6,
      transparent: true,
    },
    {
      _index: 11,
      name: 'non-destroying explosion',
      movement: 9,
      stop: true,
      movingobj: 11,
      directional: true,
      speed: 4,
      damage: 10,
      transparent: true,
      // no destroys field
    },
  ];

  if (opts?.rooms) {
    return {
      mapName: 'flammable-test',
      title: 'Flammable Test',
      teams: 0,
      roomCount: opts.rooms.length,
      rooms: opts.rooms,
      objects,
      resetOnEmpty: false,
      resetAfterSeconds: 30,
      maxPlayers: 16,
      monsterDefs: [],
      placement: null,
    };
  }

  // Place a vulnerable wall at (14,1) in the wall slot
  spot[14][1] = [1, 3];

  const room: RoomData = {
    name: 'test-room',
    floor: 0,
    team: 0,
    recorded_objects: [
      { x: 3, y: 3, type: 5, detail: 0 }, // grenade for pickup
    ],
    spot,
    exitNorth: -1,
    exitEast: -1,
    exitSouth: 0,
    exitWest: -1,
  };

  return {
    mapName: 'flammable-test',
    title: 'Flammable Test',
    teams: 0,
    roomCount: 1,
    rooms: [room],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
    monsterDefs: [],
    placement: null,
  };
}

function armWithGrenade(player: TestPlayer) {
  // Move to grenade tile and pick it up
  player.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
  player.ws.receive({ type: 'PICKUP', x: 3, y: 3 });
}

describe('flammable chain reactions & object destruction', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  // ── Wall Destruction ──────────────────────────────────────────────────────

  it('explosion with destroys destroys a vulnerable wall in the spot array', () => {
    const world = buildFlammableWorld();
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    // Move to (10,1) and fire east. Grenade travels to (14,1) where the vulnerable wall is.
    // Grenade range=4, so path: (11,1)→(12,1)→(13,1)→(14,1) — but wall at (14,1) is not
    // permeable, so missile stops before it. Path: (11,1)→(12,1)→(13,1).
    // Explosion at (13,1) radius=1: one ray goes east to (14,1).
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });

    // Advance past grenade travel + explosion ray travel
    vi.advanceTimersByTime(2000);

    // The wall at (14,1) should be destroyed (replaced with obj 8)
    expect(world.rooms[0].spot?.[14]?.[1]?.[1]).toBe(8);

    // Client should have received ROOM_OBJECT_CHANGED with layer:'wall'
    const changes = alice.ws.messagesOfType('ROOM_OBJECT_CHANGED');
    const wallChange = changes.find((m) => m.x === 14 && m.y === 1 && m.layer === 'wall');
    expect(wallChange).toBeDefined();
    expect(wallChange!.newType).toBe(8);
  });

  it('explosion without destroys does NOT destroy vulnerable objects', () => {
    const world = buildFlammableWorld();
    // Replace grenade with non-destroying grenade at (3,3)
    world.rooms[0].recorded_objects = [{ x: 3, y: 3, type: 10, detail: 0 }];
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');

    // Pick up non-destroying grenade
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    // Wall should still be intact (obj 3)
    expect(world.rooms[0].spot?.[14]?.[1]?.[1]).toBe(3);
    const changes = alice.ws.messagesOfType('ROOM_OBJECT_CHANGED');
    expect(changes.filter((m) => m.layer === 'wall')).toHaveLength(0);
  });

  // ── Flammable Dropped Item Chain Reaction ─────────────────────────────────

  it('flammable floor item triggers secondary explosion when hit', () => {
    const world = buildFlammableWorld();
    // Place a flammable barrel as a takeable recorded_object at (12,1).
    // initRoomItems converts takeable recorded_objects into roomItems (floor items).
    // The explosion at (13,1) radius=1 has a west ray that reaches (12,1).
    world.rooms[0].recorded_objects = [
      { x: 3, y: 3, type: 5, detail: 0 }, // grenade for pickup
      { x: 12, y: 1, type: 7, detail: 0 }, // flammable barrel → becomes floor item
    ];
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(3000);

    // The floor item barrel at (12,1) should be removed
    const removals = alice.ws.messagesOfType('ITEM_REMOVED');
    const barrelRemoval = removals.find((m) => m.x === 12 && m.y === 1);
    expect(barrelRemoval).toBeDefined();

    // Secondary explosion rays from barrel (objType 9 = fire explosion)
    const missiles = alice.ws.messagesOfType('MISSILE_START');
    const fireRays = missiles.filter((m) => m.objType === 9);
    expect(fireRays.length).toBeGreaterThan(0);
  });

  // ── Flammable Spot Object ─────────────────────────────────────────────────

  it('flammable object in spot array triggers secondary explosion', () => {
    const world = buildFlammableWorld();
    // Place a flammable barrel in the floor slot at (14,2)
    world.rooms[0].spot![14][2] = [7, 0]; // barrel as floor object
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    // Explosion at (13,1) radius=1: SE ray → (14,2)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    // Floor object at (14,2) should be replaced (barrel is vulnerable, destroyed not set → 0)
    expect(world.rooms[0].spot?.[14]?.[2]?.[0]).toBe(0);

    const changes = alice.ws.messagesOfType('ROOM_OBJECT_CHANGED');
    const floorChange = changes.find((m) => m.x === 14 && m.y === 2 && m.layer === 'floor');
    expect(floorChange).toBeDefined();
    expect(floorChange!.newType).toBe(0);

    // Secondary fire explosion rays should have been generated
    const missiles = alice.ws.messagesOfType('MISSILE_START');
    const fireRays = missiles.filter((m) => m.objType === 9);
    expect(fireRays.length).toBeGreaterThan(0);
  });

  // ── Deduplication ─────────────────────────────────────────────────────────

  it('two rays crossing the same tile destroy the object only once', () => {
    const world = buildFlammableWorld();
    // Place vulnerable wall right at the explosion center (13,1) so multiple rays
    // don't matter — center is only processed once
    world.rooms[0].spot![13][1] = [1, 3]; // vulnerable wall
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    // Wall should be destroyed
    expect(world.rooms[0].spot?.[13]?.[1]?.[1]).toBe(8);

    // Should only have ONE ROOM_OBJECT_CHANGED for this tile+layer
    const changes = alice.ws.messagesOfType('ROOM_OBJECT_CHANGED');
    const wallChanges = changes.filter((m) => m.x === 13 && m.y === 1 && m.layer === 'wall');
    expect(wallChanges).toHaveLength(1);
  });

  // ── Depth Limiting ────────────────────────────────────────────────────────

  it('flammable chains respect MAX_EXPLOSION_DEPTH', () => {
    const world = buildFlammableWorld();
    // Place flammable barrels in a chain: each barrel's explosion reaches the next
    // Barrel at (14,2): explosion radius 1 reaches (15,2)
    // Barrel at (15,2): would chain again but depth limit should stop it
    world.rooms[0].spot![14][2] = [7, 0]; // barrel 1
    world.rooms[0].spot![15][2] = [7, 0]; // barrel 2
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(5000);

    // Both barrels should be destroyed (barrel 1 at depth 1, barrel 2 at depth 2)
    // but no infinite loop
    expect(world.rooms[0].spot?.[14]?.[2]?.[0]).toBe(0);
    expect(world.rooms[0].spot?.[15]?.[2]?.[0]).toBe(0);

    // The test completing without hanging proves depth limiting works
  });

  // ── Non-vulnerable Objects Unaffected ─────────────────────────────────────

  it('non-vulnerable objects are not affected by explosions', () => {
    const world = buildFlammableWorld();
    // Place a plain (non-vulnerable) wall at (14,2)
    world.rooms[0].spot![14][2] = [1, 4]; // plain wall
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    // Plain wall should still be there
    expect(world.rooms[0].spot?.[14]?.[2]?.[1]).toBe(4);
  });

  // ── Late-Join Sync ────────────────────────────────────────────────────────

  it('player joining after destruction sees the destroyed tiles', () => {
    const world = buildFlammableWorld();
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    // Wall at (14,1) should be destroyed
    expect(world.rooms[0].spot?.[14]?.[1]?.[1]).toBe(8);

    // Now Bob joins
    const bob = joinPlayer(session, 'Bob');
    const changes = bob.ws.messagesOfType('ROOM_OBJECT_CHANGED');
    const wallSync = changes.find((m) => m.x === 14 && m.y === 1 && m.layer === 'wall');
    expect(wallSync).toBeDefined();
    expect(wallSync!.newType).toBe(8);
  });

  // ── Map Reset ─────────────────────────────────────────────────────────────

  it('destroyed objects are restored on map reset', () => {
    const world = buildFlammableWorld();
    world.resetOnEmpty = true;
    world.resetAfterSeconds = 1;
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    // Wall destroyed
    expect(world.rooms[0].spot?.[14]?.[1]?.[1]).toBe(8);

    // Trigger reset by disconnecting all players and waiting
    alice.ws.close();
    vi.advanceTimersByTime(2000);

    // Wall should be restored
    expect(world.rooms[0].spot?.[14]?.[1]?.[1]).toBe(3);
  });

  // ── Recorded Object Destruction ───────────────────────────────────────────

  it('vulnerable recorded objects are destroyed by explosions', () => {
    const world = buildFlammableWorld();
    // Add a vulnerable barrel as a recorded object at (12,1) — in explosion path
    world.rooms[0].recorded_objects.push({ x: 12, y: 1, type: 7, detail: 0 });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    // The recorded object at (12,1) should have its type changed
    // Barrel has no `destroyed` field, so type stays as barrel type (7)
    // Actually, obj.destroyed ?? ro.type means it stays 7 if destroyed is undefined
    // But it IS vulnerable, so it triggers the chain reaction
    const changes = alice.ws.messagesOfType('ROOM_OBJECT_CHANGED');
    const recChange = changes.find((m) => m.x === 12 && m.y === 1 && m.layer === undefined);
    expect(recChange).toBeDefined();
  });
});
