import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer, TestPlayer } from './helpers';
import { World, ObjDef, RoomData } from '../../world';

// Build a world with an arc weapon (flamethrower-style).
// Object layout:
//   1 = floor tile
//   2 = flamethrower (weapon, arc:22, spread:3, range:5, numbered, capacity:15)
//   3 = flame projectile (movingobj for flamethrower)
//   4 = fuel pack (ammo: charges matches flamethrower type)
function buildArcWorld(): World {
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );

  const objects: Array<ObjDef | null> = [
    null,
    { _index: 1, name: 'floor', movement: 5, permeable: true, transparent: true },
    {
      _index: 2,
      name: 'Flame Thrower',
      takeable: true,
      weight: 50,
      weapon: true,
      damage: 10,
      range: 5,
      arc: 22,
      spread: 3,
      numbered: true,
      capacity: 15,
      type: 256,
      movingobj: 3,
      refire: 4,
      transparent: true,
    },
    {
      _index: 3,
      name: 'flame',
      movingobj: 3,
      speed: 5,
      damage: 10,
      transparent: true,
      permeable: true,
    },
    {
      _index: 4,
      name: 'Fuel Pack',
      takeable: true,
      weight: 30,
      numbered: true,
      capacity: 5,
      charges: 256,
      transparent: true,
    },
  ];

  const room: RoomData = {
    name: 'arc-test-room',
    floor: 0,
    team: 0,
    recorded_objects: [
      { x: 5, y: 5, type: 2, detail: 10 }, // flamethrower with 10 charges
    ],
    spot,
    exitNorth: -1,
    exitEast: -1,
    exitSouth: -1,
    exitWest: -1,
  };

  return {
    mapName: 'arc-test',
    title: 'Arc Weapon Test',
    teams: 0,
    roomCount: 1,
    rooms: [room],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
  };
}

// Build a world with a wide-arc weapon (boom box style, 180-degree arc)
function buildWideArcWorld(): World {
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );

  const objects: Array<ObjDef | null> = [
    null,
    { _index: 1, name: 'floor', movement: 5, permeable: true, transparent: true },
    {
      _index: 2,
      name: 'Wave Gun',
      takeable: true,
      weight: 30,
      weapon: true,
      damage: 15,
      range: 4,
      arc: 100,
      spread: 5,
      numbered: true,
      capacity: 20,
      type: 64,
      movingobj: 3,
      transparent: true,
    },
    {
      _index: 3,
      name: 'wave',
      movingobj: 3,
      speed: 5,
      damage: 15,
      transparent: true,
      permeable: true,
    },
  ];

  const room: RoomData = {
    name: 'wide-arc-room',
    floor: 0,
    team: 0,
    recorded_objects: [
      { x: 5, y: 5, type: 2, detail: 20 }, // wave gun with 20 charges
    ],
    spot,
    exitNorth: -1,
    exitEast: -1,
    exitSouth: -1,
    exitWest: -1,
  };

  return {
    mapName: 'wide-arc-test',
    title: 'Wide Arc Test',
    teams: 0,
    roomCount: 1,
    rooms: [room],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
  };
}

describe('Arc weapons', () => {
  let session: GameSession;
  let alice: TestPlayer;

  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  describe('flamethrower (arc:22, spread:3)', () => {
    beforeEach(() => {
      session = new GameSession(buildArcWorld());
      alice = joinPlayer(session, 'Alice');

      // Move to flamethrower and pick it up
      alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
      alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
      alice.ws.flush();
    });

    it('fires multiple missiles in a cone pattern', () => {
      // Fire north
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });

      const starts = alice.ws.messagesOfType('MISSILE_START');
      // arc:22 spread:3 should produce 2-3 missiles (some may dedup at short angles)
      expect(starts.length).toBeGreaterThanOrEqual(2);
      expect(starts.length).toBeLessThanOrEqual(3);

      // All missiles should use the flame projectile object type
      for (const m of starts) {
        expect(m.objType).toBe(3); // flame projectile
        expect(m.room).toBe(0);
        expect(m.path.length).toBeGreaterThan(0);
      }
    });

    it('only consumes 1 ammo per fire action regardless of missile count', () => {
      alice.ws.flush();
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });

      const inv = alice.ws.lastOfType('YOUR_INVENTORY');
      expect(inv).toBeDefined();
      // Started with 10 charges, fired once -> 9
      expect(inv!.leftHand?.quantity).toBe(9);
    });

    it('respects fire rate cooldown for arc weapons', () => {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });
      const firstCount = alice.ws.messagesOfType('MISSILE_START').length;
      expect(firstCount).toBeGreaterThanOrEqual(2);

      alice.ws.flush();
      // Fire again immediately — should be blocked by cooldown
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });
      const secondCount = alice.ws.messagesOfType('MISSILE_START').length;
      expect(secondCount).toBe(0);

      // Advance past cooldown (refire:4 → very short cooldown)
      vi.advanceTimersByTime(500);
      alice.ws.flush();

      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });
      const thirdCount = alice.ws.messagesOfType('MISSILE_START').length;
      expect(thirdCount).toBeGreaterThanOrEqual(2);
    });

    it('arc missiles can hit players in the cone', () => {
      const bob = joinPlayer(session, 'Bob');
      // Place Bob slightly off-center but within the 22-degree cone
      bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 2 });
      bob.ws.flush();
      alice.ws.flush();

      // Fire north from Alice's position
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });

      // Advance time for missiles to land
      vi.advanceTimersByTime(500);

      // Bob should have taken damage from at least one flame in the cone
      const hits = bob.ws.messagesOfType('PLAYER_HIT');
      expect(hits.length).toBeGreaterThanOrEqual(1);
    });
  });

  describe('wide arc weapon (arc:100, spread:5)', () => {
    beforeEach(() => {
      session = new GameSession(buildWideArcWorld());
      alice = joinPlayer(session, 'Alice');

      // Move to wave gun and pick it up
      alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
      alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
      alice.ws.flush();
    });

    it('fires more missiles with a wider arc', () => {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });

      const starts = alice.ws.messagesOfType('MISSILE_START');
      // 100-degree arc with spread:5 should produce 4-5 unique missiles
      expect(starts.length).toBeGreaterThanOrEqual(3);
      expect(starts.length).toBeLessThanOrEqual(5);
    });

    it('missiles spread across different directions', () => {
      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });

      const starts = alice.ws.messagesOfType('MISSILE_START');
      // Collect final positions to verify spread
      const endPositions = starts.map((m) => {
        const last = m.path[m.path.length - 1];
        return `${last.x},${last.y}`;
      });
      // At least some missiles should end at different tiles
      const uniqueEnds = new Set(endPositions);
      expect(uniqueEnds.size).toBeGreaterThanOrEqual(2);
    });
  });

  describe('non-arc weapon fires single missile', () => {
    it('sword with no arc fires exactly one missile', () => {
      session = new GameSession(buildTestWorld());
      alice = joinPlayer(session, 'Alice');

      alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
      alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 });
      alice.ws.flush();

      alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 5, targetY: 0 });
      const starts = alice.ws.messagesOfType('MISSILE_START');
      expect(starts.length).toBe(1);
    });
  });
});
