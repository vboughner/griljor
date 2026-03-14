import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer, TestPlayer } from './helpers';
import { World, ObjDef, RoomData } from '../../world';

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

  it('weapon with refire=255 (unsigned byte for -1) is NOT treated as instant fire', () => {
    // refire=255 is the pipeline's unsigned-byte representation of signed -1.
    // Without sign-extension this gets clamped to 5 → 0ms cooldown → instant fire.
    // With correct sign-extension: -1 → 850 * 1.2 = 1020ms cooldown.
    const spot: number[][][] = Array.from({ length: 20 }, () =>
      Array.from({ length: 20 }, () => [1, 0]),
    );
    const objects: Array<ObjDef | null> = [
      null,
      { _index: 1, name: 'floor', movement: 5, permeable: true },
      { _index: 2, name: 'grenade', takeable: true, weight: 5, weapon: true, lost: true, damage: 20, range: 5, refire: 255 },
    ];
    const room: RoomData = {
      name: 'test', floor: 0, team: 0,
      recorded_objects: [{ x: 5, y: 5, type: 2, detail: 0 }],
      spot,
    };
    const world: World = {
      mapName: 'test', title: 'Test', teams: 0, roomCount: 1,
      rooms: [room], objects, resetOnEmpty: false, resetAfterSeconds: 30, maxPlayers: 16,
    };
    const grenadeSession = new GameSession(world);
    const alice = joinPlayer(grenadeSession, 'Alice');
    const bob = joinPlayer(grenadeSession, 'Bob');
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    // Two rapid shots — second must be blocked by the ~1020ms cooldown
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(1);
  });
});
