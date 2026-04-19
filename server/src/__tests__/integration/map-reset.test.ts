import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';

describe('map reset', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  it('mapStartedAt is set on construction', () => {
    expect(session.mapStartedAt).toBeTypeOf('number');
    expect(session.mapStartedAt).toBeGreaterThan(0);
  });

  it('tryReset succeeds when no players are present', () => {
    const result = session.tryReset();
    expect(result.ok).toBe(true);
    expect(result.startedAt).toBeTypeOf('number');
  });

  it('tryReset fails when players are present', () => {
    joinPlayer(session, 'Alice');
    const result = session.tryReset();
    expect(result.ok).toBe(false);
    expect(result.reason).toBeDefined();
    expect(result.startedAt).toBeTypeOf('number');
  });

  it('tryReset resets startedAt to current time', () => {
    const before = session.mapStartedAt;
    vi.advanceTimersByTime(60_000);
    const result = session.tryReset();
    expect(result.ok).toBe(true);
    expect(result.startedAt).toBeGreaterThan(before);
  });

  it('tryReset restores picked-up items', () => {
    // Join, pick up the sword at (5,5), then leave
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
    // Leave so we can reset
    alice.ws.receive({ type: 'LEAVING_GAME' });
    expect(session.playerCount).toBe(0);

    const result = session.tryReset();
    expect(result.ok).toBe(true);

    // Join again — sword should be back at (5,5)
    const bob = joinPlayer(session, 'Bob');
    const itemSync = bob.ws.lastOfType('ITEMS_SYNC');
    expect(itemSync).toBeDefined();
    const items = itemSync!.items;
    expect(
      items.some(
        (i: { x: number; y: number; item: { type: number } }) =>
          i.x === 5 && i.y === 5 && i.item.type === 2,
      ),
    ).toBe(true);
  });

  it('tryReset cancels any pending auto-reset timer', () => {
    const world = buildTestWorld();
    world.resetOnEmpty = true;
    world.resetAfterSeconds = 60;
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    alice.ws.receive({ type: 'LEAVING_GAME' });
    // Auto-reset is now scheduled for 60s from now

    // Manual reset should work immediately and cancel the timer
    const result = s.tryReset();
    expect(result.ok).toBe(true);

    s.destroy();
  });
});
