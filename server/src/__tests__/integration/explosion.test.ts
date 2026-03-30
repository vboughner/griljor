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
    player.ws.receive({ type: 'PICKUP', x: 3, y: 3 });
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

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });

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

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
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

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });

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

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });

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
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 15, y: 1 });

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    vi.advanceTimersByTime(2000);

    const youDied = bob.ws.lastOfType('YOU_DIED');
    expect(youDied).toBeDefined();
    expect(youDied!.killedBy).toBe(alice.id);

    s.destroy();
  });

  // T6 — grenade without boombit falls back to movingobj for explosion object type
  it('T6: grenade without boombit uses movingobj as explosion type', () => {
    const world = buildTestWorld();
    // Create a custom explosive whose movingobj points to obj 7 (explosion, no
    // chain) and has no boombit.  The fallback should use movingobj (7).
    world.objects.push({
      _index: 8,
      name: 'test bomb',
      takeable: true,
      weight: 5,
      weapon: true,
      damage: 5,
      range: 4,
      lost: true,
      stop: true,
      explodes: 2,
      movingobj: 7, // fallback to explosion obj (no explodes → no chain)
      speed: 6,
      transparent: true,
    });
    world.rooms[0].recorded_objects.push({ x: 4, y: 4, type: 8, detail: 0 });
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 4 });
    alice.ws.receive({ type: 'PICKUP', x: 4, y: 4 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 1 });
    // Flush the primary MISSILE_START so only explosion rays are counted below
    alice.ws.flush();

    vi.advanceTimersByTime(1000);

    // movingobj fallback → all 8 explosion rays use objType 7
    const explosionMissiles = alice.ws
      .messagesOfType('MISSILE_START')
      .filter((m) => m.objType === 7);
    expect(explosionMissiles.length).toBe(8);

    s.destroy();
  });
});

// ── Secondary (chain) explosions ──────────────────────────────────────────

describe('secondary explosions', () => {
  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    vi.useRealTimers();
  });

  // Build a world with a "nuclear grenade" (obj 8) whose boombit is the hand
  // grenade (obj 6). Object 6 has explodes:2, boombit:7. Object 7 has no
  // explodes → chain terminates at depth 1.
  //
  // Chain: nuke (8) → primary explosion rays (obj 6, 8 dirs, radius 2)
  //        each ray endpoint → secondary explosion rays (obj 7, 8 dirs, radius 1)
  //
  // Object 8: nuclear grenade — explodes:3, boombit:6, range:4, speed:6
  function buildNukeWorld() {
    const world = buildTestWorld();
    world.objects.push({
      _index: 8,
      name: 'nuclear grenade',
      takeable: true,
      weight: 10,
      weapon: true,
      damage: 5,
      range: 4,
      lost: true,
      stop: true,
      explodes: 3,
      movingobj: 8,
      boombit: 6,
      speed: 6,
      transparent: true,
    });
    // Place a nuke at (4,4) for pickup
    world.rooms[0].recorded_objects.push({ x: 4, y: 4, type: 8, detail: 0 });
    return world;
  }

  function armWithNuke(player: TestPlayer) {
    player.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 4 });
    player.ws.receive({ type: 'PICKUP', x: 4, y: 4 });
  }

  // T7 — nuclear grenade produces primary + secondary explosion rays
  //
  // Nuke (obj 8): range 4, speed 6 → msPerStep=114, travel=456ms, explodes:3 → radius 2
  // First-level boombit (obj 6): speed 6 → msPerStep=114, radius 2 → 228ms per ray
  // Second-level boombit (obj 7): speed 4 → msPerStep=170, radius 1 → 170ms per ray
  // Obj 7 has no explodes → chain stops.
  //
  // Timeline:
  //   t=0:    1 primary MISSILE_START
  //   t=456:  primary resolves → 8 first-level rays (obj 6)
  //   t=684:  first-level rays resolve → 8×8=64 secondary rays (obj 7)
  //   t=854:  secondary rays resolve → no further chaining
  it('T7: nuclear grenade produces chained explosion rays', () => {
    const world = buildNukeWorld();
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    armWithNuke(alice);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 10 });

    // Before any timers: only the primary missile
    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(1);

    // Advance past all explosion stages
    vi.advanceTimersByTime(1000);

    // 1 primary + 8 first-level (obj 6) + 64 secondary (obj 7) = 73
    const missiles = alice.ws.messagesOfType('MISSILE_START');
    expect(missiles.length).toBe(73);

    // Verify the chain used the correct object types
    const primaryCount = missiles.filter((m) => m.objType === 8).length;
    const firstLevelCount = missiles.filter((m) => m.objType === 6).length;
    const secondLevelCount = missiles.filter((m) => m.objType === 7).length;
    expect(primaryCount).toBe(1);
    expect(firstLevelCount).toBe(8);
    expect(secondLevelCount).toBe(64);

    // No further explosions after all resolve
    vi.advanceTimersByTime(1000);
    expect(alice.ws.messagesOfType('MISSILE_START').length).toBe(73);

    s.destroy();
  });

  // T8 — player in secondary blast radius takes damage
  it('T8: player in secondary blast radius takes damage', () => {
    const world = buildNukeWorld();
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    const bob = joinPlayer(s, 'Bob');
    armWithNuke(alice);

    // Alice at (10,10) fires east. Nuke lands at (14,10).
    // First-level ray east (radius 2): endpoint (16,10).
    // Secondary explosion at (16,10), radius 1: ray east reaches (17,10).
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 17, y: 10 });
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 10 });

    // Advance through all explosion stages
    vi.advanceTimersByTime(3000);

    const healthMsgs = bob.ws.messagesOfType('PLAYER_HEALTH');
    expect(healthMsgs.length).toBeGreaterThan(0);
    expect(healthMsgs.at(-1)!.hp).toBeLessThan(100);

    s.destroy();
  });

  // T9 — depth limit prevents runaway recursion with self-referencing boombit
  it('T9: depth limit caps chain explosions', () => {
    const world = buildTestWorld();
    // Make a self-referencing explosive: obj 8 explodes with boombit 8
    world.objects.push({
      _index: 8,
      name: 'infinite bomb',
      takeable: true,
      weight: 5,
      weapon: true,
      damage: 5,
      range: 2,
      lost: true,
      stop: true,
      explodes: 2,
      movingobj: 8,
      boombit: 8,
      speed: 6,
      transparent: true,
    });
    world.rooms[0].recorded_objects.push({ x: 4, y: 4, type: 8, detail: 0 });
    const s = new GameSession(world);

    const alice = joinPlayer(s, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 4 });
    alice.ws.receive({ type: 'PICKUP', x: 4, y: 4 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 10 });

    // Advance well past all possible explosions
    vi.advanceTimersByTime(10000);

    // Self-referencing chain with MAX_EXPLOSION_DEPTH=2:
    // 1 primary + 8 depth-0 rays + 64 depth-1 rays + 512 depth-2 rays = 585
    // (depth-2 rays fire but don't chain further since depth 2 < 2 is false)
    const missiles = alice.ws.messagesOfType('MISSILE_START').length;
    expect(missiles).toBe(585); // bounded by depth limit

    s.destroy();
  });
});
