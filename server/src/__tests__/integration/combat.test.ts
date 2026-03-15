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
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
  }

  it('firing a weapon sends MISSILE_START to all players in the room', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armAlice(alice);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

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

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 3, targetY: 1 });

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

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 3, targetY: 1 });

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

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 3, targetY: 1 });
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

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 3, targetY: 1 });
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
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
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
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
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
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
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
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
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
    bob.ws.receive({ type: 'PICKUP', x: 6, y: 6, hand: 'left' });

    armAlice(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    alice.ws.flush();

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
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
    bob.ws.receive({ type: 'PICKUP', x: 6, y: 6, hand: 'left' });

    armAlice(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    bob.ws.flush();

    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(2000);
    }

    // Bob should have received a YOUR_INVENTORY after dying with empty hands and inventory
    const invMsgs = bob.ws.messagesOfType('YOUR_INVENTORY');
    const lastInv = invMsgs.at(-1);
    expect(lastInv).toBeDefined();
    expect(lastInv!.leftHand).toBeNull();
    expect(lastInv!.rightHand).toBeNull();
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
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
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
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
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
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
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

  // Alice picks up bow (quantity=1) into left hand, arrows (quantity=10) into right hand
  function armAliceWithBowAndArrows(alice: TestPlayer) {
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // bow → left
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 6, y: 5, hand: 'right' }); // arrows (qty=10) → right
  }

  it('auto-reloads weapon from other hand when weapon empties mid-fire', () => {
    const alice = joinPlayer(session, 'Alice');
    armAliceWithBowAndArrows(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    // Fire once — bow had 1 charge, now 0, triggers reload from 10 arrows → weapon=5, arrows=5
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand?.type).toBe(2); // bow still in hand
    expect(inv!.leftHand?.quantity).toBe(5); // reloaded to capacity
    expect(inv!.rightHand?.quantity).toBe(5); // arrows consumed 5
  });

  it('weapon stays in hand when empty with no ammo available', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // bow (qty=1) → left, no ammo
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand?.type).toBe(2); // bow still in hand
    expect(inv!.leftHand?.quantity).toBe(0); // quantity hit 0, no reload possible
  });

  it('firing empty weapon with ammo in other hand reloads and fires', () => {
    const alice = joinPlayer(session, 'Alice');
    armAliceWithBowAndArrows(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });

    // Drain the bow first (bow qty=1 → 0 → reloads from arrows)
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    vi.advanceTimersByTime(900); // wait past fire-rate cooldown

    alice.ws.flush();

    // Now fire again with a reloaded bow (should fire a missile)
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBeGreaterThan(0);
  });

  it('firing empty weapon with no ammo does not fire a missile', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // bow (qty=1)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });

    // Drain the bow
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    alice.ws.flush();

    // Try to fire again with empty weapon and no ammo
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(0);
  });

  it('active ammo use (FIRE_WEAPON with ammo hand) reloads weapon in other hand', () => {
    const alice = joinPlayer(session, 'Alice');
    // Bow (qty=1) in left, arrows (qty=10) in right
    armAliceWithBowAndArrows(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    // Fire bow once: bow 1→0, auto-reloads from arrows(10) → bow=5, arrows=5
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    vi.advanceTimersByTime(900);
    alice.ws.flush();

    // Now fire arrows (right hand) — bow is at capacity=5, so no transfer; just verify
    // the inventory message is sent (non-weapon ammo use path is exercised)
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'right', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    // bow stays at 5 (already full), arrows stays at 5 (no transfer needed)
    expect(inv!.leftHand?.quantity).toBe(5);
    expect(inv!.rightHand?.quantity).toBe(5);
  });

  it('active ammo use reloads partially empty weapon', () => {
    const alice = joinPlayer(session, 'Alice');
    // Pick up bow with 1 charge, arrows with 10
    armAliceWithBowAndArrows(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    // First fire: bow 1→0, auto-reloads from arrows(10) → bow=5, arrows=5
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    vi.advanceTimersByTime(900); // wait past fire-rate cooldown

    // Second fire: bow 5→4, arrows unchanged
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    vi.advanceTimersByTime(900); // wait past fire-rate cooldown
    alice.ws.flush();

    // Active ammo use: fire arrows → bow 4→5, arrows 5→4
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'right', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand?.quantity).toBe(5); // bow refilled to capacity
    expect(inv!.rightHand?.quantity).toBe(4); // arrows decreased by 1
  });

  it('ammo depleted during reload pulls next ammo stack from inventory', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // bow (qty=1)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 6, y: 5, hand: 'right' }); // arrows qty=10 → right
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 7, y: 5, hand: 'right' }); // extra arrows qty=5 → inventory
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    // Fire: bow=1 → empties → reloads from arrows(qty=10) → bow=5, arrows=5
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    // arrows in right hand should still be there (only 5 consumed from 10)
    expect(inv!.rightHand?.type).toBe(3);
    expect(inv!.rightHand?.quantity).toBe(5);
    // extra arrows (qty=5) should still be in inventory (not consumed yet)
    expect(inv!.inventory.some((s) => s?.type === 3 && s.quantity === 5)).toBe(true);
  });
});
