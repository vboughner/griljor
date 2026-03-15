import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer, TestPlayer } from './helpers';

describe('grenade explosion', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  // Move player to grenade tile (3,3) and pick it up into left hand
  function armWithGrenade(player: TestPlayer) {
    player.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    player.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' });
  }

  // Alice at (10,1), fires east at (19,1).
  // Grenade (range=4) travels (11,1)→(12,1)→(13,1)→(14,1).
  // Explosion radius=1 from (14,1), 8 rays each 1 tile.
  //
  // Timing (fake timers):
  //   grenade msPerStep = round(2500/(6*2.2)) = 189ms → fires at 4*189 = 756ms
  //   explosion msPerStep = round(2500/(4*2.2)) = 284ms → fires at 756+284 = 1040ms

  // T1 — firing a grenade sends 8 additional MISSILE_START messages after the missile resolves
  it('T1: firing a grenade produces 8 explosion MISSILE_START rays', () => {
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 19, targetY: 1 });

    // Before travel resolves: only the primary missile has started
    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(1);

    // Advance past grenade travel (756ms); explosion rays are dispatched synchronously
    vi.advanceTimersByTime(1000);

    // 1 primary + 8 explosion rays
    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(9);
  });

  // T2 — explosion MISSILE_START messages use objType === 7 (the boombit type)
  it('T2: explosion MISSILE_START messages use the boombit object type', () => {
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(1000);

    const explosionMissiles = alice.ws
      .messagesOfType('MISSILE_START')
      .filter((m) => m.objType === 7);
    expect(explosionMissiles.length).toBe(8);
  });

  // T3 — MISSILE_END is broadcast for each explosion ray
  it('T3: each explosion ray has a corresponding MISSILE_END', () => {
    const alice = joinPlayer(session, 'Alice');
    armWithGrenade(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 19, targetY: 1 });

    // Advance past grenade travel (756ms) + explosion travel (284ms) = 1040ms
    vi.advanceTimersByTime(2000);

    // 1 primary MISSILE_END + 8 explosion MISSILE_ENDs
    expect(alice.ws.messagesOfType('MISSILE_END').length).toBe(9);
  });

  // T4 — player in blast radius takes damage
  it('T4: player in blast radius takes explosion damage', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    armWithGrenade(alice);

    // Alice at (10,1) fires east. Grenade lands at (14,1).
    // Explosion dir (1,0) reaches (15,1) — put Bob there.
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 15, y: 1 });
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 19, targetY: 1 });

    // Advance past grenade + explosion travel
    vi.advanceTimersByTime(2000);

    const healthMsgs = bob.ws.messagesOfType('PLAYER_HEALTH');
    expect(healthMsgs.length).toBeGreaterThan(0);
    expect(healthMsgs.at(-1)!.hp).toBeLessThan(100);
  });

  // T5 — explosion kill is attributed to the grenade thrower
  it('T5: explosion kill credit goes to the grenade thrower', () => {
    // Boost explosion damage for a guaranteed kill
    const world = buildTestWorld();
    world.objects[7]!.damage = 200;
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    const bob = joinPlayer(s, 'Bob');

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 15, y: 1 });

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    const youDied = bob.ws.lastOfType('YOU_DIED');
    expect(youDied).toBeDefined();
    expect(youDied!.killedBy).toBe(alice.id);

    s.destroy();
  });

  // T6 — grenade without boombit falls back to movingobj for explosion object type
  it('T6: grenade without boombit uses movingobj as explosion type', () => {
    const world = buildTestWorld();
    delete (world.objects[6] as unknown as Record<string, unknown>).boombit;
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 19, targetY: 1 });
    // Flush the primary MISSILE_START so only explosion rays are counted below
    alice.ws.flush();

    vi.advanceTimersByTime(1000);

    // movingobj on the grenade is 6 (the grenade itself); all 8 explosion rays use it
    const explosionMissiles = alice.ws
      .messagesOfType('MISSILE_START')
      .filter((m) => m.objType === 6);
    expect(explosionMissiles.length).toBe(8);

    s.destroy();
  });
});
