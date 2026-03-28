import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession, PUNCH_DAMAGE, PUNCH_COOLDOWN_MS } from '../../session';
import { buildTestWorld, joinPlayer, TestPlayer } from './helpers';

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

  it('punch kill is attributed to puncher', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    place(alice, 5, 5);
    place(bob, 6, 5);

    // Use 900ms increments to stay under the 1000ms regen boundary per step.
    // 30 punches × 5 damage = 150 gross damage; regen heals ~27 HP over 27s → net ~123 > 100.
    for (let i = 0; i < 30; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 6, targetY: 5 });
      vi.advanceTimersByTime(900);
    }

    const died = bob.ws.lastOfType('YOU_DIED');
    expect(died).toBeDefined();
    expect(died!.killedBy).toBe(alice.id);
  });
});
