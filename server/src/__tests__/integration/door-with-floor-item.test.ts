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

  it('dropped item lands on a non-door tile even when player is standing on a door tile', () => {
    // World: closed door at (4,5), all other tiles are plain floor.
    // Player stands at (3,5) (adjacent to the door). Pick up the sword from (5,5),
    // then move onto the door tile at (4,5) and drop it — the drop should land on
    // a neighbouring floor tile, not back on the door tile.
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
    alice.ws.flush();

    // Move onto the door tile
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 5 });
    alice.ws.flush();

    alice.ws.receive({ type: 'DROP', source: 'left' });

    const added = alice.ws.lastOfType('ITEM_ADDED');
    expect(added).toBeDefined();
    // The drop location must NOT be the door tile
    const world = (
      session as unknown as { world: ReturnType<typeof buildWorldWithDoorAndFloorItem> }
    ).world;
    const isOnDoor = world.rooms[0].recorded_objects.some(
      (ro) => ro.x === added!.x && ro.y === added!.y && world.objects[ro.type]?.swings,
    );
    expect(isOnDoor).toBe(false);
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

// ── Drop reachability tests ────────────────────────────────────────────────
//
// World layout: column x=3 is a solid wall for all y except y=5, where a
// closed door sits in recorded_objects (spot is floor-only there).  This
// partitions the room into a left side (x≤2) and a right side (x≥4).
// A potion is placed on the left side at (1,5) for the player to pick up.
//
// Any drop from the left side must land at x≤2 — it must not teleport
// through the wall/door to the right side.

function buildWorldWithWallAndDoor() {
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

  // Solid wall at column x=3 for every row except y=5 (where the door goes)
  for (let y = 0; y < 20; y++) {
    if (y !== 5) world.rooms[0].spot![3][y] = [1, 4]; // wall obj 4 = movement:0
  }
  // y=5 has a closed door in recorded_objects; spot stays as plain floor [1,0]
  world.rooms[0].recorded_objects.push({ x: 3, y: 5, type: 9, detail: 0 });

  // Potion on the left side of the wall for pickup
  world.rooms[0].recorded_objects.push({ x: 1, y: 5, type: 3, detail: 0 });

  return world;
}

describe('drop reachability: items cannot cross a closed door', () => {
  it('drops land on the player side of a closed door, not the other side', () => {
    const session = new GameSession(buildWorldWithWallAndDoor());
    const alice = joinPlayer(session, 'Alice');

    // Pick up the potion from the left side
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 1, y: 5, hand: 'left' });
    alice.ws.flush();

    // Stand adjacent to the closed door and drop the potion
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 5 });
    alice.ws.flush();

    alice.ws.receive({ type: 'DROP', source: 'left' });

    const added = alice.ws.lastOfType('ITEM_ADDED');
    expect(added).toBeDefined();
    // Must land at x≤2 — the closed door at (3,5) and walls at column 3 block the right side
    expect(added!.x).toBeLessThanOrEqual(2);
  });
});

describe('late-joining player sees current door state', () => {
  it('receives ROOM_OBJECT_CHANGED for doors opened before they joined', () => {
    const session = new GameSession(buildWorldWithDoorAndFloorItem());

    // Alice joins and opens the door
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 7, y: 5, hand: 'left' }); // pick up key
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 5 });
    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 4, targetY: 5 }); // open door

    // Bob joins after the door has already been opened
    const bob = joinPlayer(session, 'Bob');

    // Bob must receive a ROOM_OBJECT_CHANGED telling him the door is now open
    const changed = bob.ws.messagesOfType('ROOM_OBJECT_CHANGED');
    expect(changed.length).toBeGreaterThan(0);
    expect(changed[0].x).toBe(4);
    expect(changed[0].y).toBe(5);
    expect(changed[0].newType).toBe(10); // open-door
  });

  it('does not send ROOM_OBJECT_CHANGED when no doors have been toggled', () => {
    const session = new GameSession(buildWorldWithDoorAndFloorItem());

    // Bob joins with no prior door activity
    const bob = joinPlayer(session, 'Bob');

    expect(bob.ws.messagesOfType('ROOM_OBJECT_CHANGED')).toHaveLength(0);
  });
});
