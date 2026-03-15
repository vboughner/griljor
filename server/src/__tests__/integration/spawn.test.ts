import { describe, it, expect } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';
import type { RoomData, RecObj } from '../../world';

describe('spawn / respawn', () => {
  it('does not spawn a player on a tile blocked by a recorded_object', () => {
    // Build a world where every tile except (10,10) has a blocking recorded_object
    const world = buildTestWorld();
    const room = world.rooms[0] as RoomData;

    // Fill all tiles with blocking wall (type=4, movement=0) recorded_objects,
    // except (10,10) which stays clear — the only valid spawn tile.
    const blockingObjects: RecObj[] = [];
    for (let x = 0; x < 20; x++) {
      for (let y = 0; y < 20; y++) {
        if (x === 10 && y === 10) continue;
        blockingObjects.push({ x, y, type: 4, detail: 0 });
      }
    }
    room.recorded_objects = blockingObjects;

    const session = new GameSession(world);

    // Run many times to rule out lucky random picks
    for (let i = 0; i < 20; i++) {
      const p = joinPlayer(session, `Player${i}`);
      expect(
        p.x === 10 && p.y === 10,
        `Player spawned at (${p.x},${p.y}) which is blocked by a recorded_object`,
      ).toBe(true);
    }
  });
});
