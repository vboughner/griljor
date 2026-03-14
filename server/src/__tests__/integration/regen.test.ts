import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';

describe('health regeneration', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  // Arm alice with the sword from (5,5), position at (1,1), fire at (2,1).
  // Advance only 500ms — enough for the missile to land (~227ms) but NOT enough
  // for the 1-second regen interval to fire.  Bob must be at (2,1) before calling.
  function hitBobOnce(alice: ReturnType<typeof joinPlayer>) {
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
    vi.advanceTimersByTime(500); // missile lands; no regen tick yet
  }

  it('player heals 1 HP per regen tick when below max HP', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });

    hitBobOnce(alice); // Bob now at 70 HP

    bob.ws.flush();
    vi.advanceTimersByTime(1000); // one regen tick

    const health = bob.ws.lastOfType('PLAYER_HEALTH');
    expect(health).toBeDefined();
    expect(health!.hp).toBe(71);
  });

  it('regen broadcasts PLAYER_HEALTH', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    hitBobOnce(alice);
    bob.ws.flush();

    vi.advanceTimersByTime(1000);

    expect(bob.ws.messagesOfType('PLAYER_HEALTH').length).toBeGreaterThan(0);
  });

  it('regen ticks accumulate over multiple seconds', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });

    hitBobOnce(alice); // Bob at 70 HP

    bob.ws.flush();
    vi.advanceTimersByTime(5000); // 5 regen ticks → 70 + 5 = 75

    const health = bob.ws.lastOfType('PLAYER_HEALTH');
    expect(health!.hp).toBe(75);
  });

  it('regen does not heal above maxHp', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Alice is at full HP (100) — regen should not send any PLAYER_HEALTH
    vi.advanceTimersByTime(3000);

    expect(alice.ws.messagesOfType('PLAYER_HEALTH').length).toBe(0);
  });

  it('dead players do not regenerate', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Position Bob adjacent to alice's firing spot
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });

    // Kill Bob: sword does 30 dmg; each shot advances 900ms (> 850ms cooldown, < 1000ms regen).
    // After 4 shots × 30 = 120 damage, Bob is dead.
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    for (let i = 0; i < 4; i++) {
      alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(900); // missile lands; no regen tick (< 1000ms between ticks)
    }

    bob.ws.flush();

    // Bob is dead — regen should not send PLAYER_HEALTH with increasing HP
    vi.advanceTimersByTime(3000);

    const regenMsgs = bob.ws.messagesOfType('PLAYER_HEALTH').filter((m) => m.id === bob.id);
    expect(regenMsgs.length).toBe(0);
  });
});
