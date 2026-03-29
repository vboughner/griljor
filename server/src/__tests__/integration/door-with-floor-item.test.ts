import { describe, it, expect, beforeEach } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';

// Objects added to buildTestWorld for these tests:
//   8 = key         (takeable, opens:1 — bitmask matches the closed-door type:1)
//   9 = closed door (swings, movement:0, type:1, alternate:10)
//  10 = open door   (swings, movement:5, alternate:9)
//
// Room layout:
//   key (type 8)    at (7,5)
//   closed door (9) at (4,5)
//   sword (type 2)  at (4,5)  ← floor item sharing the door tile

function buildWorldWithDoorAndFloorItem() {
  const world = buildTestWorld();
  world.objects[8] = { _index: 8, name: 'key', takeable: true, weight: 1, opens: 1 };
  world.objects[9] = {
    _index: 9,
    name: 'closed-door',
    swings: true,
    movement: 0,
    type: 1,
    alternate: 10,
  };
  world.objects[10] = { _index: 10, name: 'open-door', swings: true, movement: 5, alternate: 9 };
  // key to pick up, placed within PICKUP_RANGE of (7,5)
  world.rooms[0].recorded_objects.push({ x: 7, y: 5, type: 8, detail: 0 });
  // closed door at (4,5)
  world.rooms[0].recorded_objects.push({ x: 4, y: 5, type: 9, detail: 0 });
  // sword (floor item) on the same tile as the door
  world.rooms[0].recorded_objects.push({ x: 4, y: 5, type: 2, detail: 0 });
  return world;
}

describe('USE_ITEM on tile with both a door and a floor item', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildWorldWithDoorAndFloorItem());
  });

  it('toggles the door and leaves the floor item untouched', () => {
    const alice = joinPlayer(session, 'Alice');

    // Pick up the key
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 7, y: 5, hand: 'left' });
    alice.ws.flush();

    // Move adjacent to the door
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 5 });
    alice.ws.flush();

    // Use key on the door tile (which also has a sword on it)
    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 4, targetY: 5 });

    // Door should have toggled
    const changed = alice.ws.lastOfType('ROOM_OBJECT_CHANGED');
    expect(changed).toBeDefined();
    expect(changed!.x).toBe(4);
    expect(changed!.y).toBe(5);
    expect(changed!.newType).toBe(10); // now open-door

    // Floor item (sword) must still be on the tile — USE_ITEM must not remove it
    const world = (
      session as unknown as { world: ReturnType<typeof buildWorldWithDoorAndFloorItem> }
    ).world;
    const swordStillThere = world.rooms[0].recorded_objects.some(
      (ro) => ro.x === 4 && ro.y === 5 && ro.type === 2,
    );
    expect(swordStillThere).toBe(true);
  });

  it('can toggle the door back closed again after opening it', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 7, y: 5, hand: 'left' });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 5 });
    alice.ws.flush();

    // Open the door
    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 4, targetY: 5 });
    // Close it again
    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 4, targetY: 5 });

    const changes = alice.ws.messagesOfType('ROOM_OBJECT_CHANGED');
    expect(changes.length).toBe(2);
    expect(changes[0].newType).toBe(10); // opened
    expect(changes[1].newType).toBe(9); // closed again
  });
});
