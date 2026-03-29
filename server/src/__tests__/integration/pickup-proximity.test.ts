import { describe, it, expect, beforeEach } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';

// buildTestWorld room 0 layout (all tiles are floor obj 1, walkable + transparent):
//   sword  (type 2) at (5,5)
//   potion (type 3) at (6,6)
//   potted plant (type 5) at (2,2)
//   grenade (type 6) at (3,3)
// PICKUP_RANGE = 4 (Chebyshev)

describe('pickup proximity enforcement', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildTestWorld());
  });

  it('rejects pickup when item is out of range (distance > 3)', () => {
    const alice = joinPlayer(session, 'Alice');
    // Sword is at (5,5); Alice at (0,0) → Chebyshev distance = max(5,5) = 5
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 0, y: 0 });
    alice.ws.flush();

    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    expect(alice.ws.messagesOfType('ITEM_REMOVED').length).toBe(0);
    expect(alice.ws.messagesOfType('YOUR_INVENTORY').length).toBe(0);
  });

  it('allows pickup when item is exactly at range boundary (distance = 3)', () => {
    const alice = joinPlayer(session, 'Alice');
    // Sword is at (5,5); Alice at (2,5) → Chebyshev distance = max(3,0) = 3
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 5 });
    alice.ws.flush();

    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand?.type).toBe(2); // sword
  });

  it('allows pickup at same tile (distance = 0)', () => {
    const alice = joinPlayer(session, 'Alice');
    // Sword is at (5,5); Alice also at (5,5)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.flush();

    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand?.type).toBe(2); // sword
  });

  it('rejects pickup through a transparent-but-unwalkable tile (window)', () => {
    const world = buildTestWorld();
    // Window: transparent (LOS passes) but no movement (can't walk through).
    // LOS check passes; only a full-path walkability check catches this.
    world.objects[8] = { _index: 8, name: 'window', transparent: true }; // no movement field
    // Place window at (4,5) — in the path from (2,5) to (5,5)
    world.rooms[0].spot![4][5] = [1, 8]; // floor=1, wall=window(8)
    const proximitySession = new GameSession(world);

    const alice = joinPlayer(proximitySession, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 5 });
    alice.ws.flush();

    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    // LOS is clear (window is transparent), destination tile is walkable —
    // only path walkability catches the window between player and item.
    expect(alice.ws.messagesOfType('ITEM_REMOVED').length).toBe(0);
    expect(alice.ws.messagesOfType('YOUR_INVENTORY').length).toBe(0);
  });

  it('rejects pickup when LOS is blocked by an opaque wall tile', () => {
    const world = buildTestWorld();
    // Place wall object 4 (opaque, movement:0) at (4,5) — directly in the
    // Chebyshev path from (2,5) to (5,5): path = [(3,5),(4,5),(5,5)]
    world.rooms[0].spot![4][5] = [1, 4]; // floor=1, wall=4 (opaque)
    const proximitySession = new GameSession(world);

    const alice = joinPlayer(proximitySession, 'Alice');
    // Distance max(3,0) = 3 — would be in range, but wall at (4,5) blocks LOS
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 5 });
    alice.ws.flush();

    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    expect(alice.ws.messagesOfType('ITEM_REMOVED').length).toBe(0);
    expect(alice.ws.messagesOfType('YOUR_INVENTORY').length).toBe(0);
  });

  it('rejects pickup when item tile is not walkable', () => {
    const world = buildTestWorld();
    // Place wall object 4 (movement:0) in the wall slot at (5,5) — the item tile.
    // The floor object (1, transparent, walkable) remains, but the wall blocks movement.
    world.rooms[0].spot![5][5] = [1, 4]; // floor=1, wall=4 (blocks movement)
    const proximitySession = new GameSession(world);

    const alice = joinPlayer(proximitySession, 'Alice');
    // Distance max(3,0) = 3, LOS clear (wall obj 4 is at destination tile which is the
    // target of tileViewBlocked — but spotIsVisible checks target too, and wall obj 4
    // has no transparent so this would also block LOS).
    // Either way, pickup must fail.
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 5 });
    alice.ws.flush();

    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    expect(alice.ws.messagesOfType('ITEM_REMOVED').length).toBe(0);
    expect(alice.ws.messagesOfType('YOUR_INVENTORY').length).toBe(0);
  });
});
