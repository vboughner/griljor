import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';

describe('AFK timeout', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  it('sends a warning after idle period', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance past AFK_IDLE_MS (5s in test config)
    vi.advanceTimersByTime(5001);

    const msgs = alice.ws.messagesOfType('MESSAGE');
    expect(
      msgs.some((m) => m.name === 'GM' && m.text.includes('kicked') && m.text.includes('5')),
    ).toBe(true);
  });

  it('counts down each warning interval', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(5001); // idle timer fires → first warning (5 mins left)
    vi.advanceTimersByTime(1001); // second warning (4 mins left)
    vi.advanceTimersByTime(1001); // third warning (3 mins left)

    const msgs = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(msgs.length).toBe(3);
    expect(msgs[0].text).toContain('5 minutes');
    expect(msgs[1].text).toContain('4 minutes');
    expect(msgs[2].text).toContain('3 minutes');
  });

  it('kicks the player after all warnings expire', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.flush();

    // Idle timer + 5 warning intervals
    vi.advanceTimersByTime(5001 + 5 * 1001);

    const leaving = bob.ws.messagesOfType('LEAVING_GAME');
    expect(leaving.some((m) => m.id === alice.id)).toBe(true);
  });

  it('singular "minute" in final warning', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Fire idle timer + 4 warning intervals to reach the "1 minute" warning
    vi.advanceTimersByTime(5001 + 4 * 1001);

    const msgs = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(msgs[4].text).toContain('1 minute');
    expect(msgs[4].text).not.toContain('minutes');
  });

  it('resets timer and does not warn when player sends a message during idle', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance most of idle period, then send activity
    vi.advanceTimersByTime(3000);
    alice.ws.receive({ type: 'MESSAGE', to: 'all', text: 'hello' });
    alice.ws.flush();

    // Advance past original idle expiry — but timer was reset, so no warning yet
    vi.advanceTimersByTime(3000);

    const gm = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gm.length).toBe(0);
  });

  it('sends welcome-back message when activity resumes during warning phase', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(5001); // first warning fires
    alice.ws.flush();

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });

    const gm = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gm.some((m) => m.text.toLowerCase().includes('welcome back'))).toBe(true);
  });

  it('does not send welcome-back if active before warning phase starts', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Activity before idle timer fires — no warning phase yet
    vi.advanceTimersByTime(3000);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });

    const gm = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gm.some((m) => m.text.toLowerCase().includes('welcome back'))).toBe(false);
  });

  it('PING does not reset the AFK timer', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance to just before idle expiry, then PING
    vi.advanceTimersByTime(4500);
    alice.ws.receive({ type: 'PING' });
    alice.ws.flush();

    // Advance remaining — idle timer should still fire
    vi.advanceTimersByTime(600);

    const gm = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gm.some((m) => m.text.includes('kicked'))).toBe(true);
  });
});
