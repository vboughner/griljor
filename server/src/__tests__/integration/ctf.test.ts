import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer, TestPlayer } from './helpers';
import { World, ObjDef, RoomData } from '../../world';

// ── buildCtfWorld ──────────────────────────────────────────────────────────
//
// 3-room CTF world with teams:2 and a flag object.
// Object layout:
//   0 = null
//   1 = floor (movement:9, walkable)
//   2 = sword (takeable, weapon, damage:30, range:5) — for combat tests
//   8 = flag  (flag:true, flagteams:3, takeable, movement:9)
//
// Room layout:
//   Room 0: team=1 "Team 1 Base"    exitEast→1
//   Room 1: team=2 "Team 2 Base"    exitWest→0, exitEast→2
//   Room 2: team=0 "Neutral Zone"   exitWest→1  — flag at (5,5)

function makeSpot(): number[][][] {
  return Array.from({ length: 20 }, () => Array.from({ length: 20 }, () => [1, 0]));
}

function buildCtfWorld(): World {
  const objects: Array<ObjDef | null> = [
    null,
    {
      _index: 1,
      name: 'floor',
      movement: 9,
      permeable: true,
      transparent: true,
    },
    {
      _index: 2,
      name: 'sword',
      takeable: true,
      weight: 5,
      weapon: true,
      damage: 30,
      range: 5,
      transparent: true,
    },
    null, // 3
    null, // 4
    null, // 5
    null, // 6
    null, // 7
    {
      _index: 8,
      name: 'flag',
      flag: true,
      flagteams: 3,
      takeable: true,
      weight: 5,
      movement: 9,
      transparent: true,
      permeable: true,
    },
  ];

  const room0: RoomData = {
    name: 'Team 1 Base',
    floor: 0,
    team: 1,
    recorded_objects: [],
    spot: makeSpot(),
    exitNorth: -1,
    exitEast: 1,
    exitSouth: -1,
    exitWest: -1,
  };

  const room1: RoomData = {
    name: 'Team 2 Base',
    floor: 0,
    team: 2,
    recorded_objects: [],
    spot: makeSpot(),
    exitNorth: -1,
    exitEast: 2,
    exitSouth: -1,
    exitWest: 0,
  };

  const room2: RoomData = {
    name: 'Neutral Zone',
    floor: 0,
    team: 0,
    recorded_objects: [{ x: 5, y: 5, type: 8, detail: 1 }],
    spot: makeSpot(),
    exitNorth: -1,
    exitEast: -1,
    exitSouth: -1,
    exitWest: 1,
  };

  return {
    mapName: 'ctf-test',
    title: 'CTF Test Map',
    teams: 2,
    roomCount: 3,
    rooms: [room0, room1, room2],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
    monsterDefs: [],
    placement: null,
  };
}

// Two-flag variant: flags at indices 8 and 9, both with flagteams:3
function buildTwoFlagWorld(): World {
  const world = buildCtfWorld();
  world.objects.push({
    _index: 9,
    name: 'banner',
    flag: true,
    flagteams: 3,
    takeable: true,
    weight: 5,
    movement: 9,
    transparent: true,
    permeable: true,
  });
  // Second flag also in neutral room at (7,7)
  world.rooms[2].recorded_objects.push({ x: 7, y: 7, type: 9, detail: 1 });
  return world;
}

// Single-flag world with flagteams:1 (only team 1 needs it)
function buildTeam1OnlyFlagWorld(): World {
  const world = buildCtfWorld();
  // Override flagteams so only team 1 needs the flag (bit 0)
  const flagObj = world.objects[8]!;
  flagObj.flagteams = 1;
  return world;
}

// Adds a 4th room (index 3, neutral) that is solid wall except for the single
// tile (5,5). A player standing there has no free reachable tile to drop onto,
// which is the only way nearbyFreeTile returns null.
function buildSealedRoomWorld(): World {
  const world = buildCtfWorld();
  world.objects[4] = {
    _index: 4,
    name: 'wall',
    movement: 0,
  };

  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [0, 4]),
  );
  spot[5][5] = [1, 0]; // the lone walkable tile

  world.rooms.push({
    name: 'Sealed Vault',
    floor: 0,
    team: 0,
    recorded_objects: [],
    spot,
    exitNorth: -1,
    exitEast: -1,
    exitSouth: -1,
    exitWest: -1,
  });
  world.roomCount = 4;
  return world;
}

// ── helpers ────────────────────────────────────────────────────────────────

/** Move a player to (room, x, y) by sending MY_LOCATION */
function moveTo(player: TestPlayer, room: number, x: number, y: number) {
  player.ws.receive({ type: 'MY_LOCATION', room, x, y });
  player.room = room;
  player.x = x;
  player.y = y;
}

/** Pick up floor item at the player's current position */
function pickup(player: TestPlayer) {
  player.ws.receive({ type: 'PICKUP', x: player.x, y: player.y });
}

/** Drop from active (left) hand */
function dropActive(player: TestPlayer) {
  player.ws.receive({ type: 'DROP', source: 'active' });
}

// ── tests ──────────────────────────────────────────────────────────────────

describe('capture-the-flag', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  // ── CTF disabled ─────────────────────────────────────────────────────

  it('CTF disabled on non-team maps (teams=0)', () => {
    const world = buildTestWorld(); // teams:0
    // Add a flag object to the world so we can verify it's never checked
    world.objects.push({
      _index: 8,
      name: 'flag',
      flag: true,
      flagteams: 3,
      takeable: true,
      weight: 5,
      movement: 9,
      transparent: true,
      permeable: true,
    });
    world.rooms[0].recorded_objects.push({ x: 10, y: 10, type: 8, detail: 1 });
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice', 'a', 1);
    moveTo(alice, 0, 10, 10);
    pickup(alice);
    dropActive(alice);

    expect(alice.ws.messagesOfType('GAME_OVER').length).toBe(0);
    expect(alice.ws.messagesOfType('FLAG_STATUS').length).toBe(0);
  });

  it('CTF disabled on single-team maps (teams=1)', () => {
    const world = buildCtfWorld();
    world.teams = 1; // downgrade to single team
    session = new GameSession(world);

    const alice = joinPlayer(session, 'Alice', 'a', 1);
    // Move to flag location and try to interact
    moveTo(alice, 2, 5, 5);
    pickup(alice);
    moveTo(alice, 0, 5, 5);
    dropActive(alice);

    expect(alice.ws.messagesOfType('GAME_OVER').length).toBe(0);
    expect(alice.ws.messagesOfType('FLAG_STATUS').length).toBe(0);
  });

  // ── FLAG_STATUS ──────────────────────────────────────────────────────

  it('sends FLAG_STATUS on player join with initial flag position', () => {
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    const flagMsgs = alice.ws.messagesOfType('FLAG_STATUS');
    expect(flagMsgs.length).toBeGreaterThanOrEqual(1);

    const status = flagMsgs[0];
    expect(status.flags).toHaveLength(1);
    expect(status.flags[0]).toEqual({
      objType: 8,
      room: 2,
      x: 5,
      y: 5,
      heldBy: 0,
      heldByName: '',
      teamHolding: 0, // neutral room
    });
  });

  it('FLAG_STATUS updates on pickup (heldBy set)', () => {
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);
    moveTo(alice, 2, 5, 5);
    alice.ws.flush();

    pickup(alice);

    const flagMsgs = alice.ws.messagesOfType('FLAG_STATUS');
    expect(flagMsgs.length).toBeGreaterThanOrEqual(1);
    const latest = flagMsgs.at(-1)!;
    expect(latest.flags).toHaveLength(1);
    expect(latest.flags[0].heldBy).toBe(alice.id);
    expect(latest.flags[0].room).toBe(-1);
  });

  it('FLAG_STATUS updates on drop (flag back on floor)', () => {
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);
    moveTo(alice, 2, 5, 5);
    pickup(alice);
    alice.ws.flush();

    dropActive(alice);

    const flagMsgs = alice.ws.messagesOfType('FLAG_STATUS');
    expect(flagMsgs.length).toBeGreaterThanOrEqual(1);
    const latest = flagMsgs.at(-1)!;
    expect(latest.flags).toHaveLength(1);
    expect(latest.flags[0].heldBy).toBe(0);
    expect(latest.flags[0].room).toBe(2); // dropped in neutral zone
  });

  // ── Win condition ────────────────────────────────────────────────────

  it('win condition met: flag dropped in team room', () => {
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    // Pick up flag from neutral zone
    moveTo(alice, 2, 5, 5);
    pickup(alice);

    // Move to team 1 base and drop
    moveTo(alice, 0, 10, 10);
    alice.ws.flush();
    dropActive(alice);

    const gameOver = alice.ws.messagesOfType('GAME_OVER');
    expect(gameOver.length).toBe(1);
    expect(gameOver[0].winningTeam).toBe(1);
    expect(gameOver[0].winnerName).toBe('Alice');
    expect(gameOver[0].endsInMs).toBe(30000);
  });

  it('win condition NOT met: flag dropped in neutral room', () => {
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    moveTo(alice, 2, 5, 5);
    pickup(alice);

    // Drop back in neutral room
    moveTo(alice, 2, 10, 10);
    dropActive(alice);

    expect(alice.ws.messagesOfType('GAME_OVER').length).toBe(0);
  });

  it('win condition NOT met for dropper team when flag dropped in enemy room', () => {
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    moveTo(alice, 2, 5, 5);
    pickup(alice);

    // Drop in team 2's room — team 1 hasn't won, but team 2 has
    // (the flag with flagteams:3 is needed by both teams; dropping it in
    // team 2's room satisfies team 2's requirement)
    moveTo(alice, 1, 10, 10);
    dropActive(alice);

    const gameOver = alice.ws.messagesOfType('GAME_OVER');
    expect(gameOver.length).toBe(1);
    expect(gameOver[0].winningTeam).toBe(2); // team 2 wins, not team 1
  });

  // ── Grace period ─────────────────────────────────────────────────────

  function triggerWin(): { alice: TestPlayer; bob: TestPlayer } {
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);
    const bob = joinPlayer(session, 'Bob', 'b', 2);

    // Place a sword for Bob in room 0
    moveTo(alice, 2, 5, 5);
    pickup(alice);
    moveTo(alice, 0, 10, 10);
    dropActive(alice);

    // Verify game over happened
    expect(alice.ws.messagesOfType('GAME_OVER').length).toBe(1);
    alice.ws.flush();
    bob.ws.flush();
    return { alice, bob };
  }

  it('grace period: combat disabled after GAME_OVER', () => {
    const { bob } = triggerWin();

    // Give Bob a sword to try to fire
    // We'll use the sword at (5,5) from buildTestWorld... but we're in ctf world.
    // Instead, just try to fire with bare hands (punch)
    moveTo(bob, 0, 9, 10);
    bob.ws.flush();
    bob.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

    // No missile or punch damage should happen
    expect(bob.ws.messagesOfType('MISSILE_START').length).toBe(0);
  });

  it('grace period: movement still allowed after GAME_OVER', () => {
    const { alice, bob } = triggerWin();

    // Bob moves — should be visible to Alice if in same room
    moveTo(bob, 0, 5, 5);

    // Alice should see Bob's location (PLAYER_INFO with position)
    const playerInfos = alice.ws.messagesOfType('PLAYER_INFO');
    const bobInfo = playerInfos.find((m) => m.id === bob.id);
    expect(bobInfo).toBeDefined();
  });

  it('grace period: chat still allowed after GAME_OVER', () => {
    const { alice, bob } = triggerWin();

    bob.ws.receive({ type: 'MESSAGE', to: 'all', text: 'gg' });

    const msgs = alice.ws.messagesOfType('MESSAGE');
    const chat = msgs.find((m) => m.text === 'gg');
    expect(chat).toBeDefined();
  });

  it('grace period ends with disconnect after 30s', () => {
    const { alice, bob } = triggerWin();

    // Both sockets should still be open
    expect(alice.ws.readyState).not.toBe(3); // 3 = CLOSED
    expect(bob.ws.readyState).not.toBe(3);

    vi.advanceTimersByTime(30000);

    // After 30s, all players should be disconnected
    expect(alice.ws.readyState).toBe(3); // WebSocket.CLOSED = 3
    expect(bob.ws.readyState).toBe(3);
  });

  // ── Multiple flags ───────────────────────────────────────────────────

  it('multiple flags required: one flag in team room is not enough', () => {
    session = new GameSession(buildTwoFlagWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    // Pick up first flag and drop in team 1 base
    moveTo(alice, 2, 5, 5);
    pickup(alice);
    moveTo(alice, 0, 10, 10);
    dropActive(alice);

    // Only one flag placed — should not win yet
    expect(alice.ws.messagesOfType('GAME_OVER').length).toBe(0);
  });

  it('multiple flags required: both flags in team room wins', () => {
    session = new GameSession(buildTwoFlagWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    // Pick up first flag, bring to base
    moveTo(alice, 2, 5, 5);
    pickup(alice);
    moveTo(alice, 0, 10, 10);
    dropActive(alice);

    expect(alice.ws.messagesOfType('GAME_OVER').length).toBe(0);

    // Pick up second flag, bring to base
    moveTo(alice, 2, 7, 7);
    pickup(alice);
    moveTo(alice, 0, 12, 12);
    dropActive(alice);

    const gameOver = alice.ws.messagesOfType('GAME_OVER');
    expect(gameOver.length).toBe(1);
    expect(gameOver[0].winningTeam).toBe(1);
  });

  // ── flagteams bitmask ────────────────────────────────────────────────

  it('flagteams bitmask: team with no required flags cannot win', () => {
    // flagteams:1 means only team 1 (bit 0) needs the flag.
    // Team 2 has zero required flags, so dropping in team 2 room should NOT win.
    session = new GameSession(buildTeam1OnlyFlagWorld());
    const bob = joinPlayer(session, 'Bob', 'b', 2);

    moveTo(bob, 2, 5, 5);
    pickup(bob);
    moveTo(bob, 1, 10, 10); // team 2 base
    dropActive(bob);

    const gameOver = bob.ws.messagesOfType('GAME_OVER');
    expect(gameOver.length).toBe(0);
  });

  it('flagteams bitmask: team 1 dropping team-1-only flag in team 1 room wins', () => {
    session = new GameSession(buildTeam1OnlyFlagWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    moveTo(alice, 2, 5, 5);
    pickup(alice);
    moveTo(alice, 0, 10, 10); // team 1 base
    dropActive(alice);

    const gameOver = alice.ws.messagesOfType('GAME_OVER');
    expect(gameOver.length).toBe(1);
    expect(gameOver[0].winningTeam).toBe(1);
  });

  // ── Flag on death ────────────────────────────────────────────────────

  it('flag dropped on death triggers win check', () => {
    // Put a sword in team 1 base so Bob can kill Alice there
    const world = buildCtfWorld();
    world.rooms[0].recorded_objects.push({ x: 3, y: 3, type: 2, detail: 0 });
    session = new GameSession(world);

    // Alice picks up flag and carries it to team 1 base
    const alice = joinPlayer(session, 'Alice', 'a', 1);
    moveTo(alice, 2, 5, 5);
    pickup(alice);
    moveTo(alice, 0, 10, 10);

    // Bob picks up sword in team 1 base
    const bob = joinPlayer(session, 'Bob', 'b', 2);
    moveTo(bob, 0, 3, 3);
    pickup(bob);

    // Bob kills Alice — firing sword at her
    moveTo(bob, 0, 9, 10);
    alice.ws.flush();
    bob.ws.flush();

    // Fire at Alice (at 10,10), advancing time for missile travel
    bob.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });
    vi.advanceTimersByTime(500); // let missile land

    // Alice might need multiple hits to die. Check if she died.
    // If not dead, keep shooting.
    let attempts = 0;
    while (alice.ws.messagesOfType('YOU_DIED').length === 0 && attempts < 10) {
      vi.advanceTimersByTime(900); // fire rate cooldown
      bob.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });
      vi.advanceTimersByTime(500);
      attempts++;
    }

    // Alice should be dead and her flag dropped
    expect(alice.ws.messagesOfType('YOU_DIED').length).toBeGreaterThanOrEqual(1);

    // The flag was dropped in team 1's room → team 1 should win
    // (dropPlayerItems calls checkFlagWin)
    const gameOverMsgs = bob.ws.messagesOfType('GAME_OVER');
    expect(gameOverMsgs.length).toBe(1);
    expect(gameOverMsgs[0].winningTeam).toBe(1);
  });

  // ── Regressions ──────────────────────────────────────────────────────

  it('carried flag is not destroyed when no free tile is available', () => {
    // Regression: dropPlayerItems used to skip the drop entirely when
    // nearbyFreeTile returned null, deleting the flag and making the match
    // permanently unwinnable.
    session = new GameSession(buildSealedRoomWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);
    const bob = joinPlayer(session, 'Bob', 'b', 2);

    moveTo(alice, 2, 5, 5);
    pickup(alice);
    expect(bob.ws.lastOfType('FLAG_STATUS')!.flags.some((f) => f.heldBy === alice.id)).toBe(true);

    // Sealed vault: the only reachable tile is the one Alice stands on
    moveTo(alice, 3, 5, 5);
    bob.ws.flush();
    alice.ws.close();

    const flags = bob.ws.lastOfType('FLAG_STATUS')!.flags;
    expect(flags.length).toBe(1);
    expect(flags[0]).toMatchObject({ objType: 8, room: 3, x: 5, y: 5, heldBy: 0 });
  });

  it('flags are dropped before ordinary items so they claim free tiles first', () => {
    // A sword occupying the one free tile must not push the flag out of the world.
    const world = buildSealedRoomWorld();
    world.rooms[3].recorded_objects.push({ x: 5, y: 5, type: 2, detail: 0 });
    session = new GameSession(world);
    const alice = joinPlayer(session, 'Alice', 'a', 1);
    const bob = joinPlayer(session, 'Bob', 'b', 2);

    moveTo(alice, 2, 5, 5);
    pickup(alice); // flag into left hand
    moveTo(alice, 3, 5, 5);
    pickup(alice); // sword too

    bob.ws.flush();
    alice.ws.close();

    const flags = bob.ws.lastOfType('FLAG_STATUS')!.flags;
    expect(flags.length).toBe(1);
    expect(flags[0]).toMatchObject({ objType: 8, heldBy: 0 });
  });

  it('player joining during the grace period receives GAME_OVER', () => {
    // Regression: the join path sent FLAG_STATUS but never replayed GAME_OVER,
    // leaving the new client in normal mode until it was abruptly disconnected.
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    moveTo(alice, 2, 5, 5);
    pickup(alice);
    moveTo(alice, 0, 10, 10);
    dropActive(alice);
    expect(alice.ws.messagesOfType('GAME_OVER').length).toBe(1);

    vi.advanceTimersByTime(10_000);

    const bob = joinPlayer(session, 'Bob', 'b', 2);
    const gameOver = bob.ws.messagesOfType('GAME_OVER');
    expect(gameOver.length).toBe(1);
    expect(gameOver[0].winningTeam).toBe(1);
    expect(gameOver[0].winnerName).toBe('Alice');
    // Countdown reflects time already elapsed, not a fresh 30s
    expect(gameOver[0].endsInMs).toBeLessThanOrEqual(20_000);
    expect(gameOver[0].endsInMs).toBeGreaterThan(19_000);
  });

  it('player joining a normal CTF game receives no GAME_OVER', () => {
    session = new GameSession(buildCtfWorld());
    joinPlayer(session, 'Alice', 'a', 1);
    const bob = joinPlayer(session, 'Bob', 'b', 2);
    expect(bob.ws.messagesOfType('GAME_OVER').length).toBe(0);
    expect(bob.ws.messagesOfType('FLAG_STATUS').length).toBe(1);
  });

  it('FLAG_STATUS carries the carrier name so clients need not resolve it', () => {
    // Regression: the client resolved the carrier from its otherPlayers map,
    // which excludes the local player and players in other rooms, so the HUD
    // showed "carried by ???".
    session = new GameSession(buildCtfWorld());
    const alice = joinPlayer(session, 'Alice', 'a', 1);

    moveTo(alice, 2, 5, 5);
    pickup(alice);

    const carried = alice.ws.lastOfType('FLAG_STATUS')!.flags.find((f) => f.heldBy === alice.id);
    expect(carried?.heldByName).toBe('Alice');

    dropActive(alice);
    const onFloor = alice.ws.lastOfType('FLAG_STATUS')!.flags[0];
    expect(onFloor.heldBy).toBe(0);
    expect(onFloor.heldByName).toBe('');
  });
});
