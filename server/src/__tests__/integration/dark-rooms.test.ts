import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { World, ObjDef, RoomData } from '../../world';
import { joinPlayer } from './helpers';

/**
 * Build a world with one dark room and a flashlight object.
 * Object indices:
 *   0 = null
 *   1 = floor (walkable)
 *   2 = flashlight item (flashlight: 6)
 *   3 = small flashlight (flashlight: 3)
 */
function buildDarkWorld(): World {
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );

  const objects: Array<ObjDef | null> = [
    null,
    { _index: 1, name: 'floor', movement: 5, permeable: true, transparent: true },
    {
      _index: 2,
      name: 'lasergun',
      takeable: true,
      weight: 5,
      flashlight: 6,
      transparent: true,
    },
    {
      _index: 3,
      name: 'small torch',
      takeable: true,
      weight: 2,
      flashlight: 3,
      transparent: true,
    },
  ];

  const room: RoomData = {
    name: 'dark-room',
    floor: 0,
    team: 0,
    recorded_objects: [
      { x: 5, y: 5, type: 2, detail: 0 }, // flashlight pickup
    ],
    spot,
    exitNorth: -1,
    exitEast: -1,
    exitSouth: 0,
    exitWest: -1,
    dark: 0, // dark room! (legacy: 0=DARK, 1=DAYLIT, 2=LIT)
  };

  return {
    mapName: 'dark-test',
    title: 'Dark Test',
    teams: 0,
    roomCount: 1,
    rooms: [room],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
    monsterDefs: [],
    placement: null,
  };
}

describe('dark room visibility', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildDarkWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  it('players close together in dark room can see each other', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Move both players close together (within BASE_DARK_RADIUS=2)
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 11, y: 10 });
    vi.advanceTimersByTime(100);

    // Bob should see Alice's position (distance=1, within base radius=2)
    const bobInfos = bob.ws.messagesOfType('PLAYER_INFO');
    const aliceInfo = bobInfos.find((m) => m.id === alice.id);
    expect(aliceInfo).toBeDefined();
  });

  it('players far apart in dark room cannot see each other without flashlight', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Move players far apart (distance > BASE_DARK_RADIUS=2)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 2 });
    vi.advanceTimersByTime(100);
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    vi.advanceTimersByTime(100);

    alice.ws.flush();
    bob.ws.flush();

    // Move Alice to trigger visibility recompute
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 2 });
    vi.advanceTimersByTime(100);

    // Bob should NOT receive Alice's PLAYER_INFO (distance=~8, beyond base radius=2)
    const bobInfos = bob.ws.messagesOfType('PLAYER_INFO');
    const aliceInfo = bobInfos.find((m) => m.id === alice.id);
    expect(aliceInfo).toBeUndefined();
  });

  it('player with flashlight in inventory can see further in dark room', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Move players apart by distance 5 (within flashlight radius 6, beyond base radius 2)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 10 });
    vi.advanceTimersByTime(100);
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    vi.advanceTimersByTime(100);

    // Give Alice a flashlight (radius 6) via pickup at (5,5)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    vi.advanceTimersByTime(100);
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
    vi.advanceTimersByTime(100);

    // Move Alice to position where distance to Bob = 5
    alice.ws.flush();
    bob.ws.flush();
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 10 });
    vi.advanceTimersByTime(100);

    // Alice has flashlight (radius 6), distance to Bob is 5 → should see Bob
    const aliceInfos = alice.ws.messagesOfType('PLAYER_INFO');
    const bobInfo = aliceInfos.find((m) => m.id === bob.id);
    expect(bobInfo).toBeDefined();
  });
});
