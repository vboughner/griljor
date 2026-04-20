import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer, TestPlayer } from './helpers';
import { World, ObjDef, RoomData } from '../../world';

// Test world with a numbered weapon (bow, type:2, capacity:5) and ammo (arrows, charges:2).
// Object layout:
//   1 = floor tile (movement:5)
//   2 = bow       (takeable, weapon, numbered, type:2, capacity:5, damage:10, range:5)
//   3 = arrows    (takeable, numbered, charges:2, weight:1) — quantity set via detail
function buildAmmoWorld(): World {
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );

  const objects: Array<ObjDef | null> = [
    null,
    { _index: 1, name: 'floor', movement: 5, permeable: true },
    {
      _index: 2,
      name: 'bow',
      takeable: true,
      weight: 3,
      weapon: true,
      numbered: true,
      type: 2,
      capacity: 5,
      damage: 10,
      range: 5,
    },
    { _index: 3, name: 'arrows', takeable: true, weight: 1, numbered: true, charges: 2 },
  ];

  const room: RoomData = {
    name: 'ammo-test-room',
    floor: 0,
    team: 0,
    // bow at (5,5) with 1 arrow loaded (detail=1)
    // arrows at (6,5) with 10 charges (detail=10)
    // extra arrows at (7,5) with 5 charges (detail=5)
    recorded_objects: [
      { x: 5, y: 5, type: 2, detail: 1 },
      { x: 6, y: 5, type: 3, detail: 10 },
      { x: 7, y: 5, type: 3, detail: 5 },
    ],
    spot,
    exitNorth: -1,
    exitEast: -1,
    exitSouth: -1,
    exitWest: -1,
  };

  return {
    mapName: 'ammo-test',
    title: 'Ammo Test',
    teams: 0,
    roomCount: 1,
    rooms: [room],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
    placement: null,
  };
}

describe('combat', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    vi.useRealTimers();
  });

  // Pick up the sword at (5,5) into Alice's left hand
  function armAlice(alice: TestPlayer) {
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
  }

  it('firing a weapon sends MISSILE_START to all players in the room', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBeGreaterThan(0);
    expect(bob.ws.messagesOfType('MISSILE_START').length).toBeGreaterThan(0);
  });

  it('MISSILE_START includes the correct target direction', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 3, targetY: 1 });

    const missile = alice.ws.lastOfType('MISSILE_START');
    expect(missile).toBeDefined();
    expect(missile!.dx).toBe(1); // firing right
    expect(missile!.dy).toBe(0);
  });

  it('damage is applied after missile travel — target loses HP', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 1 });
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 3, targetY: 1 });

    // No health update before travel completes
    expect(bob.ws.messagesOfType('PLAYER_HEALTH').length).toBe(0);

    vi.advanceTimersByTime(2000);

    const healthMsgs = bob.ws.messagesOfType('PLAYER_HEALTH');
    expect(healthMsgs.length).toBeGreaterThan(0);
    expect(healthMsgs.at(-1)!.hp).toBeLessThan(100);
  });

  it('MISSILE_END is broadcast after missile resolves', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 3, targetY: 1 });
    vi.advanceTimersByTime(2000);

    expect(alice.ws.messagesOfType('MISSILE_END').length).toBeGreaterThan(0);
  });

  it('firing without a weapon does nothing', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    // Alice has no weapon — bare hands

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 1 });
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 3, targetY: 1 });
    vi.advanceTimersByTime(2000);

    expect(bob.ws.messagesOfType('MISSILE_START').length).toBe(0);
    expect(bob.ws.messagesOfType('PLAYER_HEALTH').length).toBe(0);
  });

  it('killing a player sends YOU_DIED to victim', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    bob.ws.flush();

    // 4 × 30 damage = 120 > 100 HP
    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    expect(bob.ws.messagesOfType('YOU_DIED').length).toBeGreaterThan(0);
  });

  it('killing a player increments killer kill count', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    alice.ws.flush();

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    const aliceStats = alice.ws.messagesOfType('PLAYER_STATS').find((m) => m.id === alice.id);
    expect(aliceStats?.kills).toBeGreaterThanOrEqual(1);
  });

  it('death increments victim death count', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    const bobStats = alice.ws.messagesOfType('PLAYER_STATS').find((m) => m.id === bob.id);
    expect(bobStats?.deaths).toBeGreaterThanOrEqual(1);
  });

  it('death is announced in global chat', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    bob.ws.flush();

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    const gmMessages = bob.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gmMessages.some((m) => m.text.includes('Bob') && m.text.includes('Alice'))).toBe(true);
  });

  it('killed player drops inventory on death', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Give Bob the potion
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 6 });
    bob.ws.receive({ type: 'PICKUP', x: 6, y: 6 });

    armAlice(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    alice.ws.flush();

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    // Bob's potion should be dropped as an ITEM_ADDED broadcast
    const drops = alice.ws.messagesOfType('ITEM_ADDED');
    expect(drops.some((m) => m.item.type === 3)).toBe(true); // potion
  });

  it('victim receives YOUR_INVENTORY with empty slots immediately on death', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Give Bob the potion (picked up into left hand)
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 6 });
    bob.ws.receive({ type: 'PICKUP', x: 6, y: 6 });

    armAlice(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    bob.ws.flush();

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    // Bob should have received a YOUR_INVENTORY after dying with empty hands and inventory
    const invMsgs = bob.ws.messagesOfType('YOUR_INVENTORY');
    const lastInv = invMsgs.at(-1);
    expect(lastInv).toBeDefined();
    expect(lastInv!.leftHand).toBeNull();
    expect(lastInv!.inventory.every((slot) => slot === null)).toBe(true);
  });

  it('killed player respawns after delay and receives YOU_RESPAWNED', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    bob.ws.flush();

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    // Advance past the 5000ms respawn delay
    vi.advanceTimersByTime(6000);

    expect(bob.ws.messagesOfType('YOU_RESPAWNED').length).toBeGreaterThan(0);
  });

  it('respawned player has full HP', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    bob.ws.flush();

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    vi.advanceTimersByTime(6000);

    const healthAfterRespawn = bob.ws.messagesOfType('PLAYER_HEALTH').at(-1);
    expect(healthAfterRespawn?.hp).toBe(100);
  });

  it('dead player movement is ignored', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    alice.ws.flush();
    // Bob is dead — MY_LOCATION should be rejected
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 15, y: 15 });

    const bobMoved = alice.ws
      .messagesOfType('MY_LOCATION')
      .filter((m) => m.id === bob.id && m.x === 15 && m.y === 15);
    expect(bobMoved.length).toBe(0);
  });

  it('thrown item (lost+stop) drops at landing tile after missile resolves', () => {
    const alice = joinPlayer(session, 'Alice');
    // Pick up potted plant from (2,2)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 2 });
    alice.ws.receive({ type: 'PICKUP', x: 2, y: 2 });
    // Move to (1,1) and fire right — range:3 → path (2,1),(3,1),(4,1), lands at (4,1)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 1 });

    // Missile not yet resolved
    expect(alice.ws.messagesOfType('ITEM_ADDED').length).toBe(0);

    // Advance past travel time (3 steps × ~284ms ≈ 852ms)
    vi.advanceTimersByTime(2000);

    const drops = alice.ws.messagesOfType('ITEM_ADDED');
    expect(drops.some((m) => m.item.type === 5)).toBe(true); // potted plant dropped
  });

  it('exploding item (lost+stop+explodes) does NOT drop on the floor after travel', () => {
    const alice = joinPlayer(session, 'Alice');
    // Pick up grenade from (3,3)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 1 });
    vi.advanceTimersByTime(2000);

    const drops = alice.ws.messagesOfType('ITEM_ADDED');
    expect(drops.some((m) => m.item.type === 6)).toBe(false); // grenade must NOT drop
  });

  it('thrown item lands near target player when hit tile is occupied', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    // Alice picks up potted plant
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 2 });
    alice.ws.receive({ type: 'PICKUP', x: 2, y: 2 });
    // Alice at (1,1), Bob at (4,1) — plant flies into Bob at step 3 of range 3
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 1 });
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 1 });
    vi.advanceTimersByTime(2000);

    // Plant should land somewhere near (4,1) even though Bob is standing there
    const drops = alice.ws.messagesOfType('ITEM_ADDED').filter((m) => m.item.type === 5);
    expect(drops.length).toBeGreaterThan(0);
  });

  it('missile stops before a dropped non-permeable item on the floor', () => {
    // Add a sword-like weapon (8) and a non-permeable dropped wall (9)
    const world = buildTestWorld();
    world.objects.push(
      {
        _index: 8,
        name: 'block gun',
        takeable: true,
        weight: 5,
        weapon: true,
        damage: 5,
        range: 10,
        lost: true,
        stop: true,
        movingobj: 9,
        speed: 5,
        transparent: true,
      },
      // thrown block: NOT permeable, NOT takeable → should block missiles
      { _index: 9, name: 'thrown block', weight: 3 },
    );
    // Two block guns for two shots
    world.rooms[0].recorded_objects.push(
      { x: 4, y: 4, type: 8, detail: 0 },
      { x: 4, y: 5, type: 8, detail: 0 },
    );
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    // Pick up first block gun, fire east from (1,1) — range 10, block lands at (11,1)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 4 });
    alice.ws.receive({ type: 'PICKUP', x: 4, y: 4 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(3000);

    const firstDrop = alice.ws.messagesOfType('ITEM_ADDED').find((m) => m.item.type === 9);
    expect(firstDrop).toBeDefined();
    const blockX = firstDrop!.x;

    // Pick up second block gun, fire east along same row
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 4, y: 5 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();
    vi.advanceTimersByTime(1000);

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(3000);

    // The second missile's path should NOT include the tile with the dropped block
    const missileStarts = alice.ws.messagesOfType('MISSILE_START');
    const secondMissile = missileStarts[missileStarts.length - 1];
    const pathXs = secondMissile.path.map((p: { x: number }) => p.x);
    expect(pathXs).not.toContain(blockX);

    // Second block should land before the first
    const secondDrop = alice.ws
      .messagesOfType('ITEM_ADDED')
      .filter((m) => m.item.type === 9)
      .at(-1);
    expect(secondDrop).toBeDefined();
    expect(secondDrop!.x).toBeLessThan(blockX);
  });

  it('block gun (lost+stop, movingobj differs) drops movingobj, not the gun itself', () => {
    // Add block gun (8) and thrown block (9) to a fresh test world
    const world = buildTestWorld();
    world.objects.push(
      {
        _index: 8,
        name: 'block gun',
        takeable: true,
        weight: 5,
        weapon: true,
        damage: 5,
        range: 5,
        lost: true,
        stop: true,
        movingobj: 9,
        speed: 5,
        transparent: true,
      },
      { _index: 9, name: 'thrown block', weight: 3 },
    );
    world.rooms[0].recorded_objects.push({ x: 4, y: 4, type: 8, detail: 0 });
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    // Pick up block gun from (4,4)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 4 });
    alice.ws.receive({ type: 'PICKUP', x: 4, y: 4 });
    // Move to (1,1) and fire right — range:5 → lands at end of path
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 1 });
    vi.advanceTimersByTime(2000);

    const drops = alice.ws.messagesOfType('ITEM_ADDED');
    // Should drop the thrown block (type 9), NOT the block gun (type 8)
    expect(drops.some((m) => m.item.type === 9)).toBe(true);
    expect(drops.some((m) => m.item.type === 8)).toBe(false);
  });
});

describe('ammo reload', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildAmmoWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  // Alice picks up bow (quantity=1) into left hand, arrows (quantity=10) into inventory[0]
  function armAliceWithBowAndArrows(alice: TestPlayer) {
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 }); // bow → leftHand
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 6, y: 5 }); // arrows (qty=10) → inventory[0]
  }

  it('auto-reloads weapon from inventory when weapon empties mid-fire', () => {
    const alice = joinPlayer(session, 'Alice');
    armAliceWithBowAndArrows(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand?.type).toBe(2); // bow still in hand
    expect(inv!.leftHand?.quantity).toBe(5); // reloaded to capacity
    expect(inv!.inventory[0]?.quantity).toBe(5); // arrows partially consumed
  });

  it('weapon stays in hand when empty with no ammo in inventory', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 }); // bow (qty=1), no arrows picked up
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv!.leftHand?.type).toBe(2);
    expect(inv!.leftHand?.quantity).toBe(0); // empty, no reload possible
  });

  it('firing empty weapon with ammo in inventory reloads and fires', () => {
    const alice = joinPlayer(session, 'Alice');
    armAliceWithBowAndArrows(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });

    // Drain the bow first (bow qty=1 → 0 → reloads from inventory arrows)
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });
    vi.advanceTimersByTime(900); // wait past fire-rate cooldown

    alice.ws.flush();

    // Now fire again with a reloaded bow (should fire a missile)
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBeGreaterThan(0);
  });

  it('firing empty weapon with no ammo does not fire a missile', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 }); // bow (qty=1)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });

    // Drain the bow
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });
    alice.ws.flush();

    // Try to fire again with empty weapon and no ammo
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(0);
  });

  it('reloads from slot 0 before slot 1', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 }); // bow → leftHand
    // Pick up arrows at (6,5) into inventory[0], and extra arrows at (7,5) into inventory[1]
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 6, y: 5 }); // 10 arrows → inventory[0]
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 7, y: 5 }); // 5 arrows → inventory[1]
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    // Slot 0 arrows (qty=10) used to reload, not slot 1 (qty=5)
    expect(inv!.inventory[0]?.quantity).toBe(5); // slot 0 consumed 5 to fill bow
    expect(inv!.inventory[1]?.quantity).toBe(5); // slot 1 untouched
  });
});
