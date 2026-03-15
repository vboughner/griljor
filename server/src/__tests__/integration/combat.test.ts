import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer, TestPlayer } from './helpers';

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
