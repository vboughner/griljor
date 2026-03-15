import { describe, it, expect, beforeEach, afterEach } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';
import { World, ObjDef, RoomData } from '../../world';

// ── LOS Test World ────────────────────────────────────────────────────────────
//
// Object layout:
//   0 = null
//   1 = floor (movement:5, transparent:true) — walkable, sight-passable
//   4 = wall  (movement:0, no transparent) — impassable, blocks LOS
//
// Room layout: 20×20 grid with a vertical wall column at x=10.
// Left half  (x 0–9):  open floor
// Wall column (x=10):  wall tiles [4, 0] — blocks LOS
// Right half (x 11–19): open floor
//
// This means a player at (5, 5) cannot see a player at (15, 5).
// Two players both on the left (or both on the right) can see each other.

function buildLosTestWorld(): World {
  const spot: number[][][] = Array.from({ length: 20 }, (_, x) =>
    Array.from({ length: 20 }, () => {
      if (x === 10) return [4, 0]; // wall tile at x=10 — blocks LOS
      return [1, 0]; // floor tile
    }),
  );

  const objects: Array<ObjDef | null> = [
    null, // 0
    { _index: 1, name: 'floor', movement: 5, permeable: true, transparent: true }, // 1 — walkable, sight-passable
    null, // 2
    null, // 3
    { _index: 4, name: 'wall', movement: 0 }, // 4 — opaque wall, blocks LOS
  ];

  const room: RoomData = {
    name: 'los-test-room',
    floor: 0,
    team: 0,
    recorded_objects: [],
    spot,
  };

  return {
    mapName: 'los-test',
    title: 'LOS Test Map',
    teams: 0,
    roomCount: 1,
    rooms: [room],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
  };
}

// ── Two-room world for cross-room tests ───────────────────────────────────────
// Room 0 and room 1 are both open floor. Players in different rooms always
// receive each other's MY_LOCATION updates regardless of position.

function buildTwoRoomWorld(): World {
  const makeSpot = (): number[][][] =>
    Array.from({ length: 20 }, () => Array.from({ length: 20 }, () => [1, 0]));

  const objects: Array<ObjDef | null> = [
    null,
    { _index: 1, name: 'floor', movement: 5, permeable: true, transparent: true },
  ];

  return {
    mapName: 'two-room-test',
    title: 'Two Room Test',
    teams: 0,
    roomCount: 2,
    rooms: [
      { name: 'room-0', floor: 0, team: 0, recorded_objects: [], spot: makeSpot() },
      { name: 'room-1', floor: 0, team: 0, recorded_objects: [], spot: makeSpot() },
    ],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
  };
}

// ── Helpers ───────────────────────────────────────────────────────────────────

function joinAt(
  session: GameSession,
  name: string,
  room: number,
  x: number,
  y: number,
): { ws: MockWebSocket; id: number } {
  const p = joinPlayer(session, name);
  // Move player to the desired position
  p.ws.receive({ type: 'MY_LOCATION', room, x, y });
  return { ws: p.ws, id: p.id };
}

// ── Tests ─────────────────────────────────────────────────────────────────────

describe('LOS visibility — wall blocks initial PLAYER_INFO on join', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildLosTestWorld());
  });

  afterEach(() => {
    session.destroy();
  });

  it('player behind a wall does NOT receive PLAYER_INFO for the other player on join', () => {
    // B joins and moves to the right side (x=15, behind the wall at x=10)
    const b = joinPlayer(session, 'Bob');
    b.ws.receive({ type: 'MY_LOCATION', room: 0, x: 15, y: 5 });

    // A joins on the left side (x=5) — wall at x=10 blocks LOS to B
    const a = joinPlayer(session, 'Alice');
    a.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });

    // Flush all accumulated messages (spawn position was random so Alice may have
    // briefly been visible to Bob during join; we want to test the *settled* state).
    a.ws.flush();
    b.ws.flush();

    // B moves a step on the right side — triggers a fresh visibility update.
    // The wall at x=10 should block LOS, so A must NOT receive PLAYER_INFO for B.
    b.ws.receive({ type: 'MY_LOCATION', room: 0, x: 15, y: 6 });

    const infos = a.ws.messagesOfType('PLAYER_INFO');
    expect(infos.some((m) => m.id === b.id)).toBe(false);
  });
});

describe('LOS visibility — move to reveal and re-hide', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildLosTestWorld());
  });

  afterEach(() => {
    session.destroy();
  });

  it('both players receive PLAYER_INFO when mover reaches a visible position', () => {
    // B is on the right side behind the wall
    const b = joinPlayer(session, 'Bob');
    b.ws.receive({ type: 'MY_LOCATION', room: 0, x: 15, y: 5 });

    // A starts on the left side — can't see B
    const a = joinPlayer(session, 'Alice');
    a.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });

    // Flush accumulated messages: spawn positions are random so there may be
    // incidental PLAYER_INFO from the initial placement before players reached
    // their intended positions.
    a.ws.flush();
    b.ws.flush();

    // Move B to the left side (x=7) where A at (5,5) can see B at (7,5) — no wall in between
    b.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 5 });

    // A should now receive PLAYER_INFO for B (B is revealed to A)
    expect(a.ws.messagesOfType('PLAYER_INFO').some((m) => m.id === b.id)).toBe(true);

    // B should receive PLAYER_INFO for A (A is revealed to B)
    expect(b.ws.messagesOfType('PLAYER_INFO').some((m) => m.id === a.id)).toBe(true);
  });

  it('both players receive PLAYER_HIDDEN when mover moves behind a wall', () => {
    // Both players start on the left side — they can see each other
    const a = joinAt(session, 'Alice', 0, 5, 5);
    const b = joinAt(session, 'Bob', 0, 7, 5);

    // Verify they can see each other
    expect(a.ws.messagesOfType('PLAYER_INFO').some((m) => m.id === b.id)).toBe(true);

    // Clear message queues
    a.ws.flush();
    b.ws.flush();

    // Move B behind the wall to the right side (x=15)
    b.ws.receive({ type: 'MY_LOCATION', room: 0, x: 15, y: 5 });

    // A should receive PLAYER_HIDDEN for B
    const aHidden = a.ws.messagesOfType('PLAYER_HIDDEN');
    expect(aHidden.some((m) => m.id === b.id)).toBe(true);

    // B should receive PLAYER_HIDDEN for A
    const bHidden = b.ws.messagesOfType('PLAYER_HIDDEN');
    expect(bHidden.some((m) => m.id === a.id)).toBe(true);
  });
});

describe('LOS visibility — open room always visible during movement', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    session.destroy();
  });

  it('two players in open space receive MY_LOCATION updates (not PLAYER_HIDDEN) as one moves', () => {
    // Both players on the same side in the open test world
    const a = joinAt(session, 'Alice', 0, 1, 1);
    const b = joinAt(session, 'Bob', 0, 3, 3);

    a.ws.flush();
    b.ws.flush();

    // Move A to several different open positions; B should get MY_LOCATION each time
    const positions = [
      { x: 2, y: 2 },
      { x: 4, y: 1 },
      { x: 1, y: 5 },
    ];

    for (const pos of positions) {
      b.ws.flush();
      a.ws.receive({ type: 'MY_LOCATION', room: 0, x: pos.x, y: pos.y });

      // B should receive MY_LOCATION for A — not PLAYER_HIDDEN
      const locMsgs = b.ws.messagesOfType('MY_LOCATION');
      expect(locMsgs.some((m) => m.id === a.id)).toBe(true);
      expect(b.ws.messagesOfType('PLAYER_HIDDEN').some((m) => m.id === a.id)).toBe(false);
    }
  });
});

describe('LOS visibility — players in different rooms', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildTwoRoomWorld());
  });

  afterEach(() => {
    session.destroy();
  });

  it('player in different room always receives MY_LOCATION updates', () => {
    // A joins and moves to room 0
    const a = joinPlayer(session, 'Alice');
    a.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });

    // B joins and moves to room 1
    const b = joinPlayer(session, 'Bob');
    b.ws.receive({ type: 'MY_LOCATION', room: 1, x: 5, y: 5 });

    // Clear queues
    a.ws.flush();
    b.ws.flush();

    // Move A within room 0
    a.ws.receive({ type: 'MY_LOCATION', room: 0, x: 8, y: 8 });

    // B in room 1 should receive A's MY_LOCATION update regardless of walls
    const locMsgs = b.ws.messagesOfType('MY_LOCATION');
    expect(locMsgs.some((m) => m.id === a.id && m.room === 0 && m.x === 8 && m.y === 8)).toBe(true);

    // B should NOT receive PLAYER_HIDDEN for A
    expect(b.ws.messagesOfType('PLAYER_HIDDEN').some((m) => m.id === a.id)).toBe(false);
  });

  it('PLAYER_INFO is sent to both players on join even when in different rooms', () => {
    // A joins and moves to room 0
    const a = joinPlayer(session, 'Alice');
    a.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });

    // B joins and moves to room 1
    const b = joinPlayer(session, 'Bob');

    // A should have received PLAYER_INFO for B (different rooms always share info)
    expect(a.ws.messagesOfType('PLAYER_INFO').some((m) => m.id === b.id)).toBe(true);

    // B should have received PLAYER_INFO for A
    expect(b.ws.messagesOfType('PLAYER_INFO').some((m) => m.id === a.id)).toBe(true);
  });
});
