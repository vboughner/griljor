import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { World } from '../../world';
import { MonsterDef } from '../../monster-types';
import { joinPlayer, buildTestWorld } from './helpers';

// ── Test world with monsters ──────────────────────────────────────────────

function buildMonsterTestWorld(): World {
  const base = buildTestWorld();

  const dweebDef: MonsterDef = {
    id: 'dweeb',
    name: 'Dweeb',
    avatar: 'dweeb',
    hp: 30,
    maxHp: 30,
    team: 0,
    behavior: { type: 'wander', moveInterval: 2000, pauseChance: 0.3 },
    combat: { aggressive: false },
    chat: null,
    items: {
      drops: [{ type: 2, quantity: 1, chance: 1.0 }], // always drops a sword
      dropOnDeath: true,
      pickup: null,
      carry: null,
    },
    respawn: { delay: 5000 },
  };

  const guardDef: MonsterDef = {
    id: 'guard',
    name: 'Guard',
    avatar: 'robot',
    hp: 50,
    maxHp: 50,
    team: 0, // neutral — attacks all players
    behavior: { type: 'stationary', moveInterval: 1000 },
    combat: { aggressive: true, weaponType: 2, aggroRange: 8, fireInterval: 1200 },
    chat: null,
    items: {
      drops: null,
      dropOnDeath: false,
      pickup: null,
      carry: null,
    },
    respawn: { delay: 0 }, // no respawn
  };

  const thiefDef: MonsterDef = {
    id: 'thief',
    name: 'Thief',
    avatar: 'dodger',
    hp: 30,
    maxHp: 30,
    team: 0,
    behavior: { type: 'stationary', moveInterval: 1000 }, // stationary for predictable tests
    combat: { aggressive: false },
    chat: null,
    items: {
      drops: null,
      dropOnDeath: true,
      pickup: { range: 3, types: 'any', maxCarry: 2 },
      carry: { deliverTo: 'home' },
    },
    respawn: { delay: 5000 },
  };

  base.monsterDefs = [dweebDef, guardDef, thiefDef];
  base.rooms[0].monsterSpawns = [
    { monsterId: 'dweeb', count: 1, spawnX: 15, spawnY: 15, spawnRate: 0 },
  ];

  return base;
}

// ── Tests ─────────────────────────────────────────────────────────────────

describe('Monster system — Phase 2 (spawn, damage, death)', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    session?.destroy();
    vi.useRealTimers();
  });

  // ── Spawning ──────────────────────────────────────────────────────────

  it('spawns monsters at session start and sends MONSTER_INFO to joining player', () => {
    const world = buildMonsterTestWorld();
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');

    const monsterInfos = alice.ws.messagesOfType('MONSTER_INFO');
    expect(monsterInfos.length).toBe(1);

    const info = monsterInfos[0];
    expect(info.id).toBeLessThan(0); // negative ID
    expect(info.name).toBe('Dweeb');
    expect(info.avatar).toBe('dweeb');
    expect(info.hp).toBe(30);
    expect(info.maxHp).toBe(30);
    expect(info.dead).toBe(false);
    expect(info.monsterId).toBe('dweeb');
    expect(info.room).toBe(0);
  });

  it('spawns monster at preferred spawnX/spawnY', () => {
    const world = buildMonsterTestWorld();
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    const info = alice.ws.messagesOfType('MONSTER_INFO')[0];
    expect(info.x).toBe(15);
    expect(info.y).toBe(15);
  });

  it('uses negative IDs that decrement', () => {
    const world = buildMonsterTestWorld();
    // Add a second monster spawn
    world.rooms[0].monsterSpawns!.push({
      monsterId: 'dweeb',
      count: 1,
      spawnX: 16,
      spawnY: 16,
      spawnRate: 0,
    });
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    const infos = alice.ws.messagesOfType('MONSTER_INFO');
    expect(infos.length).toBe(2);
    expect(infos[0].id).toBe(-1);
    expect(infos[1].id).toBe(-2);
  });

  it('does not spawn if monsterId is unknown', () => {
    const world = buildMonsterTestWorld();
    world.rooms[0].monsterSpawns = [{ monsterId: 'nonexistent', count: 1, spawnRate: 0 }];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    const monsterInfos = alice.ws.messagesOfType('MONSTER_INFO');
    expect(monsterInfos.length).toBe(0);
  });

  // ── Damage and death ──────────────────────────────────────────────────

  it('player can damage a monster by firing a weapon at its tile', () => {
    const world = buildMonsterTestWorld();
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');

    // Move near sword, pick it up, then move near monster
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 12, y: 15 });
    alice.ws.flush();

    // Fire toward the monster (at 15,15)
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 15, targetY: 15 });

    // Advance time for missile travel
    vi.advanceTimersByTime(5000);

    // Check for PLAYER_HIT with the monster's negative ID
    const hits = alice.ws.messagesOfType('PLAYER_HIT');
    const monsterHit = hits.find((h) => h.victimId < 0);
    expect(monsterHit).toBeDefined();
    expect(monsterHit!.damage).toBe(30);
  });

  it('monster dies when HP reaches 0 and drops loot', () => {
    const world = buildMonsterTestWorld();
    // Make monster HP = 10 so one sword hit kills it
    world.monsterDefs[0].hp = 10;
    world.monsterDefs[0].maxHp = 10;
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');

    // Move Alice close to monster (monster at 15,15)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 12, y: 15 });
    // Pickup sword at (5,5) — need to be in range first
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
    // Move back near monster
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 12, y: 15 });
    alice.ws.flush();

    // Fire at monster
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 15, targetY: 15 });
    vi.advanceTimersByTime(5000);

    // Should get death announcement via GM message
    const messages = alice.ws.messagesOfType('MESSAGE');
    const deathMsg = messages.find(
      (m) => m.name === 'GM' && m.text.includes('Dweeb') && m.text.includes('slain'),
    );
    expect(deathMsg).toBeDefined();

    // Should get ITEM_ADDED for the loot drop (sword, 100% chance)
    const itemAdds = alice.ws.messagesOfType('ITEM_ADDED');
    const lootDrop = itemAdds.find((i) => i.item.type === 2);
    expect(lootDrop).toBeDefined();
    expect(lootDrop!.room).toBe(0);

    // Should get MONSTER_INFO with dead=true
    const monsterInfos = alice.ws.messagesOfType('MONSTER_INFO');
    const deadInfo = monsterInfos.find((m) => m.dead === true);
    expect(deadInfo).toBeDefined();
  });

  it('monster without respawn is removed after death', () => {
    const world = buildMonsterTestWorld();
    world.rooms[0].monsterSpawns = [
      { monsterId: 'guard', count: 1, spawnX: 15, spawnY: 15, spawnRate: 0 },
    ];
    // Make guard weak enough to kill
    world.monsterDefs[1].hp = 5;
    world.monsterDefs[1].maxHp = 5;
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');

    // Move to pickup sword, then fire at guard
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 12, y: 15 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 15, targetY: 15 });
    vi.advanceTimersByTime(5000);

    // Guard has respawn.delay = 0, so MONSTER_HIDDEN should be sent shortly after death
    const hiddens = alice.ws.messagesOfType('MONSTER_HIDDEN');
    expect(hiddens.length).toBeGreaterThan(0);
  });

  it('monster respawns after respawn delay', () => {
    const world = buildMonsterTestWorld();
    world.monsterDefs[0].hp = 5;
    world.monsterDefs[0].maxHp = 5;
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');

    // Kill the dweeb
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 12, y: 15 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 15, targetY: 15 });
    vi.advanceTimersByTime(2000); // missile arrives

    // Verify dead
    const deadInfos = alice.ws.messagesOfType('MONSTER_INFO').filter((m) => m.dead);
    expect(deadInfos.length).toBe(1);

    alice.ws.flush();

    // Advance past respawn delay (5000ms)
    vi.advanceTimersByTime(5000);

    // Should get a new MONSTER_INFO with dead=false (respawned)
    const respawnInfos = alice.ws.messagesOfType('MONSTER_INFO');
    const aliveInfo = respawnInfos.find((m) => !m.dead);
    expect(aliveInfo).toBeDefined();
    expect(aliveInfo!.hp).toBe(5); // full HP after respawn
  });

  // ── Punch ─────────────────────────────────────────────────────────────

  it('player can punch a monster on an adjacent tile', () => {
    const world = buildMonsterTestWorld();
    // Place monster at (10,11) — adjacent to default position
    world.rooms[0].monsterSpawns = [
      { monsterId: 'dweeb', count: 1, spawnX: 10, spawnY: 11, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    // Move next to monster
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    alice.ws.flush();

    // Punch (no weapon in hand → punch)
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 11 });

    const hits = alice.ws.messagesOfType('PLAYER_HIT');
    const monsterHit = hits.find((h) => h.victimId < 0);
    expect(monsterHit).toBeDefined();
    expect(monsterHit!.damage).toBe(10); // PUNCH_DAMAGE
  });

  // ── Cleanup ───────────────────────────────────────────────────────────

  it('session.destroy() clears monster timers without errors', () => {
    const world = buildMonsterTestWorld();
    world.monsterDefs[0].respawn.delay = 1000;
    session = new GameSession(world);

    // Just verify destroy doesn't throw
    expect(() => session.destroy()).not.toThrow();
  });

  // ── No monster spawns ─────────────────────────────────────────────────

  it('works fine with worlds that have no monster spawns', () => {
    const world = buildTestWorld(); // no monsterDefs or monsterSpawns
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    const monsterInfos = alice.ws.messagesOfType('MONSTER_INFO');
    expect(monsterInfos.length).toBe(0);
  });

  // ── Phase 3: Movement & Visibility ────────────────────────────────────

  it('wandering monster moves after moveInterval ticks', () => {
    const world = buildMonsterTestWorld();
    // Ensure monster always moves (no pause)
    world.monsterDefs[0].behavior.pauseChance = 0;
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    // Move Alice near the monster so she can see it
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 14, y: 15 });
    alice.ws.flush();

    // Advance by the wander moveInterval (2000ms)
    vi.advanceTimersByTime(2000);

    // Monster should have moved — check for MONSTER_LOCATION message
    const locations = alice.ws.messagesOfType('MONSTER_LOCATION');
    // May get MONSTER_INFO (if it walked into view) or MONSTER_LOCATION (already visible)
    const monsterMsgs = [
      ...locations,
      ...alice.ws.messagesOfType('MONSTER_INFO').filter((m) => !m.dead),
    ];
    expect(monsterMsgs.length).toBeGreaterThan(0);
  });

  it('stationary monster does not move', () => {
    const world = buildMonsterTestWorld();
    // Use the guard (stationary behavior)
    world.rooms[0].monsterSpawns = [
      { monsterId: 'guard', count: 1, spawnX: 15, spawnY: 15, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 14, y: 15 });
    alice.ws.flush();

    // Advance several intervals
    vi.advanceTimersByTime(5000);

    // Should NOT get any MONSTER_LOCATION (stationary doesn't move)
    const locations = alice.ws.messagesOfType('MONSTER_LOCATION');
    expect(locations.length).toBe(0);
  });

  it('monster behind a wall is not visible to player', () => {
    const world = buildMonsterTestWorld();
    // Place a wall at (13,15) between player and monster at (15,15)
    world.rooms[0].spot![13][15] = [4, 0]; // wall object (id=4, blocks LOS)
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    // Move to (12,15) — wall at (13,15) blocks LOS to monster at (15,15)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 12, y: 15 });
    alice.ws.flush();

    // Check: should NOT have MONSTER_INFO for the dweeb (blocked by wall)
    const infos = alice.ws.messagesOfType('MONSTER_INFO');
    const visible = infos.filter((m) => m.name === 'Dweeb');
    expect(visible.length).toBe(0);
  });

  it('monster becomes visible when player moves to LOS', () => {
    const world = buildMonsterTestWorld();
    // Place a wall between player start and monster
    world.rooms[0].spot![13][15] = [4, 0]; // wall blocks LOS
    // Use stationary guard so monster doesn't move during test
    world.rooms[0].monsterSpawns = [
      { monsterId: 'guard', count: 1, spawnX: 15, spawnY: 15, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    // Start behind wall — can't see monster
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 12, y: 15 });
    alice.ws.flush();

    let infos = alice.ws.messagesOfType('MONSTER_INFO');
    expect(infos.filter((m) => m.name === 'Guard').length).toBe(0);

    // Move to (14,15) — past the wall, should now see monster at (15,15)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 14, y: 15 });

    infos = alice.ws.messagesOfType('MONSTER_INFO');
    const guard = infos.find((m) => m.name === 'Guard' && !m.dead);
    expect(guard).toBeDefined();
    expect(guard!.x).toBe(15);
    expect(guard!.y).toBe(15);
  });

  it('MONSTER_HIDDEN sent when monster leaves player LOS', () => {
    const world = buildMonsterTestWorld();
    // Use stationary guard so it doesn't wander during test
    world.rooms[0].monsterSpawns = [
      { monsterId: 'guard', count: 1, spawnX: 15, spawnY: 15, spawnRate: 0 },
    ];
    // Pre-place a wall at (13,15) that we'll use to block LOS
    world.rooms[0].spot![13][15] = [4, 0]; // wall object blocks LOS

    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    // Move to (14,15) — can see monster at (15,15) (wall is behind us)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 14, y: 15 });

    // Verify monster is visible
    const infos = alice.ws.messagesOfType('MONSTER_INFO');
    expect(infos.filter((m) => m.name === 'Guard').length).toBeGreaterThan(0);

    alice.ws.flush();

    // Move behind the wall — LOS now blocked
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 12, y: 15 });

    const hiddens = alice.ws.messagesOfType('MONSTER_HIDDEN');
    expect(hiddens.length).toBeGreaterThan(0);
  });

  // ── Phase 4: Monster Combat ───────────────────────────────────────────

  it('aggressive monster fires at a nearby player', () => {
    const world = buildMonsterTestWorld();
    // Guard: aggressive, weaponType=2 (sword, damage 30, range 5), fireInterval=1200
    world.rooms[0].monsterSpawns = [
      { monsterId: 'guard', count: 1, spawnX: 10, spawnY: 10, spawnRate: 0 },
    ];
    session = new GameSession(world);

    // Place Alice in range of the guard (team 0, guard is team 1)
    const alice = joinPlayer(session, 'Alice', 'a', 0);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 13 });
    alice.ws.flush();

    // Advance past the guard's moveInterval (1000ms) to trigger aggro check
    vi.advanceTimersByTime(1000);

    // Should see MISSILE_START from the guard
    const missiles = alice.ws.messagesOfType('MISSILE_START');
    expect(missiles.length).toBeGreaterThan(0);

    const missile = missiles[0];
    expect(missile.id).toBeLessThan(0); // monster missile IDs are negative
    expect(missile.room).toBe(0);
    expect(missile.objType).toBe(2); // sword type

    // Advance time for missile to arrive
    vi.advanceTimersByTime(5000);

    // Player should take damage
    const hits = alice.ws.messagesOfType('PLAYER_HIT');
    const aliceHit = hits.find((h) => h.victimId === alice.id);
    expect(aliceHit).toBeDefined();
    expect(aliceHit!.damage).toBe(30);
  });

  it('aggressive monster respects fire interval cooldown', () => {
    const world = buildMonsterTestWorld();
    world.rooms[0].monsterSpawns = [
      { monsterId: 'guard', count: 1, spawnX: 10, spawnY: 10, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice', 'a', 0);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 13 });
    alice.ws.flush();

    // First tick: should fire
    vi.advanceTimersByTime(1000);
    expect(alice.ws.messagesOfType('MISSILE_START').length).toBeGreaterThan(0);

    // Second tick (1000ms later, but fireInterval is 1200ms): should NOT fire again
    alice.ws.flush();
    vi.advanceTimersByTime(1000);
    const missiles2 = alice.ws.messagesOfType('MISSILE_START').length;
    expect(missiles2).toBe(0);

    // Third tick (another 1000ms, total 2000ms since first fire > 1200ms): should fire
    alice.ws.flush();
    vi.advanceTimersByTime(1000);
    const missiles3 = alice.ws.messagesOfType('MISSILE_START').length;
    expect(missiles3).toBeGreaterThan(0);
  });

  it('aggressive monster does not fire at same-team players', () => {
    const world = buildMonsterTestWorld();
    world.teams = 2; // enable teams so player team assignment works
    // Create a team-specific guard (team 1)
    const teamGuard: MonsterDef = {
      ...world.monsterDefs.find((d) => d.id === 'guard')!,
      id: 'team-guard',
      team: 1,
    };
    world.monsterDefs.push(teamGuard);
    world.rooms[0].monsterSpawns = [
      { monsterId: 'team-guard', count: 1, spawnX: 10, spawnY: 10, spawnRate: 0 },
    ];
    session = new GameSession(world);

    // Alice joins team 1 (same as guard)
    const alice = joinPlayer(session, 'Alice', 'a', 1);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 13 });
    alice.ws.flush();

    // Advance several ticks
    vi.advanceTimersByTime(5000);

    // Should NOT fire at same-team player
    const missiles = alice.ws.messagesOfType('MISSILE_START');
    expect(missiles.length).toBe(0);
  });

  it('monster kills player and death is announced', () => {
    const world = buildMonsterTestWorld();
    world.rooms[0].monsterSpawns = [
      { monsterId: 'guard', count: 1, spawnX: 10, spawnY: 10, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice', 'a', 0);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 13 });
    alice.ws.flush();

    // Fire + arrive repeatedly until Alice dies (100 HP, 30 damage per hit)
    for (let i = 0; i < 10; i++) {
      vi.advanceTimersByTime(1500); // past fireInterval
      vi.advanceTimersByTime(5000); // missile arrives
    }

    // Alice should have died
    const deaths = alice.ws.messagesOfType('YOU_DIED');
    expect(deaths.length).toBeGreaterThan(0);

    // Death announcement from GM
    const msgs = alice.ws.messagesOfType('MESSAGE');
    const deathMsg = msgs.find(
      (m) => m.name === 'GM' && m.text.includes('Alice') && m.text.includes('slain'),
    );
    expect(deathMsg).toBeDefined();
  });

  it('passive monster does not fire even when player is nearby', () => {
    const world = buildMonsterTestWorld();
    // Dweeb is passive (aggressive: false)
    world.rooms[0].monsterSpawns = [
      { monsterId: 'dweeb', count: 1, spawnX: 10, spawnY: 10, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 11 });
    alice.ws.flush();

    vi.advanceTimersByTime(10000);

    const missiles = alice.ws.messagesOfType('MISSILE_START');
    expect(missiles.length).toBe(0);
  });

  // ── Phase 5: Item Pickup / Carry / Deliver ────────────────────────────

  it('thief monster picks up a nearby floor item', () => {
    const world = buildMonsterTestWorld();
    // Place thief at (5,5) — near the sword floor item at (5,5)
    world.rooms[0].monsterSpawns = [
      { monsterId: 'thief', count: 1, spawnX: 4, spawnY: 5, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance past moveInterval so the thief's onMoveTick fires
    vi.advanceTimersByTime(1000);

    // Thief should have picked up the sword at (5,5) — ITEM_REMOVED broadcast
    const removals = alice.ws.messagesOfType('ITEM_REMOVED');
    const swordRemoval = removals.find((r) => r.x === 5 && r.y === 5);
    expect(swordRemoval).toBeDefined();
  });

  it('thief respects maxCarry limit', () => {
    const world = buildMonsterTestWorld();
    const thief = world.monsterDefs.find((d) => d.id === 'thief')!;
    // Disable delivery so items stay carried
    thief.items.carry = null;
    // Thief has maxCarry: 2. Put 3 items within range.
    // Existing items: sword at (5,5), potion at (6,6)
    // Add another item at (4,4)
    world.rooms[0].recorded_objects.push({ x: 4, y: 4, type: 3, detail: 0 }); // another potion
    world.rooms[0].monsterSpawns = [
      { monsterId: 'thief', count: 1, spawnX: 5, spawnY: 5, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance enough ticks for 3 pickup attempts
    vi.advanceTimersByTime(3000);

    // Should only pick up 2 items (maxCarry)
    const removals = alice.ws.messagesOfType('ITEM_REMOVED');
    expect(removals.length).toBe(2);
  });

  it('thief with type filter only picks matching items', () => {
    const world = buildMonsterTestWorld();
    // Override thief to only pick up swords (type 2)
    const thief = world.monsterDefs.find((d) => d.id === 'thief')!;
    thief.items.pickup = { range: 3, types: [2], maxCarry: 5 };
    // Also disable delivery so the picked item stays carried
    thief.items.carry = null;
    // Spawn thief near the sword (5,5) but not on it
    world.rooms[0].monsterSpawns = [
      { monsterId: 'thief', count: 1, spawnX: 4, spawnY: 5, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance several ticks
    vi.advanceTimersByTime(3000);

    // Should only pick up the sword at (5,5), not the potion at (6,6)
    // or any non-type-2 items
    const removals = alice.ws.messagesOfType('ITEM_REMOVED');
    expect(removals.length).toBe(1);
    expect(removals[0].x).toBe(5);
    expect(removals[0].y).toBe(5);
  });

  it('killed thief drops carried items', () => {
    const world = buildMonsterTestWorld();
    const thief = world.monsterDefs.find((d) => d.id === 'thief')!;
    thief.hp = 5;
    thief.maxHp = 5;
    // Disable delivery so items stay carried until death
    thief.items.carry = null;
    world.rooms[0].monsterSpawns = [
      { monsterId: 'thief', count: 1, spawnX: 4, spawnY: 5, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');

    // Let thief pick up the sword at (5,5) — within range 3
    vi.advanceTimersByTime(1000);

    // Verify sword was picked up
    const removals = alice.ws.messagesOfType('ITEM_REMOVED');
    expect(removals.find((r) => r.x === 5 && r.y === 5)).toBeDefined();

    alice.ws.flush();

    // Move Alice adjacent to thief and punch it to death
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 5 });
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 4, targetY: 5 });

    // Thief should die and drop its carried sword
    const itemAdds = alice.ws.messagesOfType('ITEM_ADDED');
    const droppedSword = itemAdds.find((i) => i.item.type === 2);
    expect(droppedSword).toBeDefined();
  });

  it('thief delivers items when at home position', () => {
    const world = buildMonsterTestWorld();
    // Stationary thief spawned AT a floor item: picks up on first tick,
    // then delivers immediately since it's already at home.
    world.rooms[0].monsterSpawns = [
      { monsterId: 'thief', count: 1, spawnX: 5, spawnY: 5, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // First tick: thief picks up nearby item (sword at 5,5 — distance 0)
    vi.advanceTimersByTime(1000);

    // The thief spawned at (5,5) which is its home. It's stationary so it stays there.
    // On the same tick it picks up, then tryDeliverItems checks if at home — it is!
    // So it should immediately deliver (drop) the item.

    // Check: ITEM_REMOVED for pickup, then ITEM_ADDED for delivery
    const removals = alice.ws.messagesOfType('ITEM_REMOVED');
    const additions = alice.ws.messagesOfType('ITEM_ADDED');

    expect(removals.length).toBeGreaterThan(0);
    expect(additions.length).toBeGreaterThan(0);
  });

  // ── Phase 6: Monster Chat ─────────────────────────────────────────────

  it('chatty monster broadcasts messages at chat interval', () => {
    const world = buildMonsterTestWorld();
    // Create a chatty monster with 100% chat chance for deterministic testing
    const chattyDef: MonsterDef = {
      id: 'chatty',
      name: 'Chatty',
      avatar: 'dweeb',
      hp: 50,
      maxHp: 50,
      team: 0,
      behavior: { type: 'stationary', moveInterval: 5000 },
      combat: { aggressive: false },
      chat: { phrases: ['Hello!', 'Nice day!'], chatInterval: 2000, chatChance: 1.0 },
      items: { drops: null, dropOnDeath: false, pickup: null, carry: null },
      respawn: { delay: 0 },
    };
    world.monsterDefs.push(chattyDef);
    world.rooms[0].monsterSpawns = [
      { monsterId: 'chatty', count: 1, spawnX: 10, spawnY: 10, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance past chat interval
    vi.advanceTimersByTime(2000);

    const messages = alice.ws.messagesOfType('MESSAGE');
    const monsterChat = messages.filter((m) => m.from < 0 && m.name === 'Chatty');
    expect(monsterChat.length).toBe(1);
    expect(['Hello!', 'Nice day!']).toContain(monsterChat[0].text);
  });

  it('monster with chatChance 0 never chats', () => {
    const world = buildMonsterTestWorld();
    const silentDef: MonsterDef = {
      id: 'silent',
      name: 'Silent',
      avatar: 'dweeb',
      hp: 50,
      maxHp: 50,
      team: 0,
      behavior: { type: 'stationary', moveInterval: 5000 },
      combat: { aggressive: false },
      chat: { phrases: ['You should never see this'], chatInterval: 1000, chatChance: 0 },
      items: { drops: null, dropOnDeath: false, pickup: null, carry: null },
      respawn: { delay: 0 },
    };
    world.monsterDefs.push(silentDef);
    world.rooms[0].monsterSpawns = [
      { monsterId: 'silent', count: 1, spawnX: 10, spawnY: 10, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(5000);

    const messages = alice.ws.messagesOfType('MESSAGE');
    const monsterChat = messages.filter((m) => m.from < 0);
    expect(monsterChat.length).toBe(0);
  });

  it('dead monster stops chatting', () => {
    const world = buildMonsterTestWorld();
    const chattyDef: MonsterDef = {
      id: 'chatty',
      name: 'Chatty',
      avatar: 'dweeb',
      hp: 5,
      maxHp: 5,
      team: 0,
      behavior: { type: 'stationary', moveInterval: 5000 },
      combat: { aggressive: false },
      chat: { phrases: ['Hello!'], chatInterval: 1000, chatChance: 1.0 },
      items: { drops: null, dropOnDeath: false, pickup: null, carry: null },
      respawn: { delay: 0 },
    };
    world.monsterDefs.push(chattyDef);
    world.rooms[0].monsterSpawns = [
      { monsterId: 'chatty', count: 1, spawnX: 10, spawnY: 10, spawnRate: 0 },
    ];
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice');

    // Kill the chatty monster
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 9, y: 10 });
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 }); // punch kills (10 > 5 HP)
    alice.ws.flush();

    // Advance time — dead monster should not chat
    vi.advanceTimersByTime(5000);

    const messages = alice.ws.messagesOfType('MESSAGE');
    const monsterChat = messages.filter((m) => m.from < 0 && m.name === 'Chatty');
    expect(monsterChat.length).toBe(0);
  });
});
