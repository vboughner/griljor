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
    team: 1,
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

  base.monsterDefs = [dweebDef, guardDef];
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
});
