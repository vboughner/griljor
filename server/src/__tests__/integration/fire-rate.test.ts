import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer, TestPlayer } from './helpers';

describe('fire rate limiting', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    vi.useRealTimers();
  });

  function armPlayer(player: TestPlayer, x = 1, y = 1) {
    player.ws.receive({ type: 'MY_LOCATION', room: 0, x, y });
    player.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
    player.ws.receive({ type: 'MY_LOCATION', room: 0, x, y });
  }

  it('second shot within cooldown window is silently ignored', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    armPlayer(alice, 1, 1);
    alice.ws.flush();

    // Fire twice with no time elapsed — second shot should be dropped
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(1);
  });

  it('second shot is allowed after the cooldown elapses', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    armPlayer(alice, 1, 1);
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    // Advance past the 850ms default cooldown
    vi.advanceTimersByTime(900);
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(2);
  });

  it('rapid triple-click only produces one missile', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    armPlayer(alice, 1, 1);
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(1);
  });
});
