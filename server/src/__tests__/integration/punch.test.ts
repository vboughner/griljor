import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession, PUNCH_DAMAGE, PUNCH_COOLDOWN_MS } from '../../session';
import { buildTestWorld, buildTwoRoomWorld, joinPlayer, TestPlayer } from './helpers';

describe('punching', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  function place(p: TestPlayer, x: number, y: number) {
    p.ws.receive({ type: 'MY_LOCATION', room: 0, x, y });
  }

  it('deals PUNCH_DAMAGE to adjacent player', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    place(alice, 5, 5);
    place(bob, 6, 5);
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });

    const health = bob.ws.lastOfType('PLAYER_HEALTH');
    expect(health).toBeDefined();
    expect(health!.hp).toBe(100 - PUNCH_DAMAGE);
  });

  it('broadcasts PUNCH message to room at target tile', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    place(alice, 5, 5);
    place(bob, 6, 5);
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });

    const punch = alice.ws.lastOfType('PUNCH');
    expect(punch).toBeDefined();
    expect(punch!.x).toBe(6);
    expect(punch!.y).toBe(5);
    expect(punch!.room).toBe(0);
  });

  it('PUNCH message includes dx/dy direction', () => {
    const alice = joinPlayer(session, 'Alice');
    place(alice, 5, 5);
    alice.ws.flush();

    // Punch east
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });
    const east = alice.ws.lastOfType('PUNCH');
    expect(east!.dx).toBe(1);
    expect(east!.dy).toBe(0);

    vi.advanceTimersByTime(PUNCH_COOLDOWN_MS);

    // Punch southeast
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 7, targetY: 7 });
    const se = alice.ws.lastOfType('PUNCH');
    expect(se!.dx).toBe(1);
    expect(se!.dy).toBe(1);
  });

  it('broadcasts PUNCH even when punching empty space', () => {
    const alice = joinPlayer(session, 'Alice');
    place(alice, 5, 5);
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });

    const punch = alice.ws.lastOfType('PUNCH');
    expect(punch).toBeDefined();
  });

  it('does not reach non-adjacent player (distance > 1)', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    place(alice, 5, 5);
    place(bob, 7, 5); // 2 tiles east — punch snaps to (6,5), not (7,5)
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 7, targetY: 5 });

    const health = bob.ws.messagesOfType('PLAYER_HEALTH');
    expect(health).toHaveLength(0);
  });

  it('respects PUNCH_COOLDOWN_MS', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    place(alice, 5, 5);
    place(bob, 6, 5);
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });

    const healths = bob.ws.messagesOfType('PLAYER_HEALTH');
    expect(healths).toHaveLength(1);
  });

  it('can punch again after cooldown expires', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    place(alice, 5, 5);
    place(bob, 6, 5);
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });
    vi.advanceTimersByTime(PUNCH_COOLDOWN_MS);
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });

    const healths = bob.ws.messagesOfType('PLAYER_HEALTH');
    expect(healths).toHaveLength(2);
    expect(healths[1].hp).toBe(100 - PUNCH_DAMAGE * 2);
  });

  it('empty-space punch consumes cooldown', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    place(alice, 5, 5);
    place(bob, 6, 5);
    bob.ws.flush();

    // Punch empty space first (west)
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 4, targetY: 5 });
    // Now punch Bob — should be blocked by cooldown
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });

    const healths = bob.ws.messagesOfType('PLAYER_HEALTH');
    expect(healths).toHaveLength(0);
  });

  it('PUNCH_DAMAGE equals 10', () => {
    expect(PUNCH_DAMAGE).toBe(10);
  });

  it('PUNCH_COOLDOWN_MS equals 400', () => {
    expect(PUNCH_COOLDOWN_MS).toBe(400);
  });

  it('cross-room punch hits player on the adjacent border tile', () => {
    const s2 = new GameSession(buildTwoRoomWorld());
    const alice = joinPlayer(s2, 'Alice');
    const bob = joinPlayer(s2, 'Bob', 'b', 1);

    // Alice on the south border tile of room 0, Bob on the north border tile of room 1
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 19 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 1, x: 10, y: 0 });
    bob.ws.flush();

    // Alice punches south into room 1 (targetY > GRID-1)
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 25 });

    const health = bob.ws.lastOfType('PLAYER_HEALTH');
    expect(health).toBeDefined();
    expect(health!.hp).toBe(100 - PUNCH_DAMAGE);

    s2.destroy();
  });

  it('cross-room punch sends PUNCH to the next room', () => {
    const s2 = new GameSession(buildTwoRoomWorld());
    const alice = joinPlayer(s2, 'Alice');
    const bob = joinPlayer(s2, 'Bob', 'b', 1);

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 19 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 1, x: 10, y: 0 });
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 25 });

    const punch = bob.ws.lastOfType('PUNCH');
    expect(punch).toBeDefined();
    expect(punch!.room).toBe(1);
    expect(punch!.x).toBe(10);
    expect(punch!.y).toBe(0);

    s2.destroy();
  });

  it('punch kill is attributed to puncher', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    place(alice, 5, 5);
    place(bob, 6, 5);

    // Use 500ms increments (above 400ms cooldown, below 1000ms regen boundary per step).
    // 15 punches × 10 damage = 150 gross damage; regen heals ~7 HP over 7.5s → net ~143 > 100.
    for (let i = 0; i < 15; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });
      vi.advanceTimersByTime(500);
    }

    const died = bob.ws.lastOfType('YOU_DIED');
    expect(died).toBeDefined();
    expect(died!.killedBy).toBe(alice.id);
  });
});
