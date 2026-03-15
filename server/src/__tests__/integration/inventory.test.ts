import { describe, it, expect, beforeEach } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';

describe('inventory', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildTestWorld());
  });

  it('pickup puts item in hand slot and sends YOUR_INVENTORY', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand).not.toBeNull();
    expect(inv!.leftHand!.type).toBe(2); // sword
  });

  it('pickup broadcasts ITEM_REMOVED to all players', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    expect(bob.ws.messagesOfType('ITEM_REMOVED').some((m) => m.x === 5 && m.y === 5)).toBe(true);
  });

  it('cannot pick up item at wrong position', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Sword is at (5,5), try to pick it up from (4,4) — nothing there
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 4 });
    alice.ws.receive({ type: 'PICKUP', x: 4, y: 4, hand: 'left' });

    // A failed pickup sends no ITEM_REMOVED and no YOUR_INVENTORY
    expect(alice.ws.messagesOfType('ITEM_REMOVED').length).toBe(0);
    expect(alice.ws.messagesOfType('YOUR_INVENTORY').length).toBe(0);
  });

  it('drop sends ITEM_ADDED broadcast with item details', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    const bob = joinPlayer(session, 'Bob');
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'DROP', source: 'left' });

    const addedMsgs = bob.ws.messagesOfType('ITEM_ADDED');
    expect(addedMsgs.some((m) => m.item.type === 2)).toBe(true);
  });

  it('drop clears hand slot in YOUR_INVENTORY', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
    alice.ws.flush();

    alice.ws.receive({ type: 'DROP', source: 'left' });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv!.leftHand).toBeNull();
  });

  it('second pickup goes to inventory when hand is occupied', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // sword → leftHand
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 6 });
    alice.ws.receive({ type: 'PICKUP', x: 6, y: 6, hand: 'left' }); // potion → inventory[0]

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv!.leftHand!.type).toBe(2); // sword still in hand
    expect(inv!.inventory.some((slot) => slot?.type === 3)).toBe(true); // potion in bag
  });

  it('INV_SWAP moves item from inventory slot to hand', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // sword in leftHand
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 6 });
    alice.ws.receive({ type: 'PICKUP', x: 6, y: 6, hand: 'left' }); // potion into inventory[0]

    alice.ws.flush();
    alice.ws.receive({ type: 'INV_SWAP', slot: 0, hand: 'right' }); // potion → rightHand

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv!.rightHand).not.toBeNull();
    expect(inv!.rightHand!.type).toBe(3); // potion
    expect(inv!.inventory[0]).toBeNull(); // slot now empty
  });

  it('INV_SWAP swaps hand item into slot when slot is empty', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // sword in leftHand
    alice.ws.flush();

    alice.ws.receive({ type: 'INV_SWAP', slot: 0, hand: 'left' }); // sword → inventory[0]

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv!.leftHand).toBeNull();
    expect(inv!.inventory[0]?.type).toBe(2); // sword in slot
  });

  it('new player receives ITEMS_SYNC with existing floor items', () => {
    const bob = joinPlayer(session, 'Bob');
    const sync = bob.ws.lastOfType('ITEMS_SYNC');
    expect(sync).toBeDefined();
    expect(sync!.items.some((i) => i.item.type === 2 && i.x === 5 && i.y === 5)).toBe(true);
    expect(sync!.items.some((i) => i.item.type === 3 && i.x === 6 && i.y === 6)).toBe(true);
  });

  it('ITEMS_SYNC does not include already-picked-up items', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // sword picked up

    const bob = joinPlayer(session, 'Bob');
    const sync = bob.ws.lastOfType('ITEMS_SYNC');
    expect(sync!.items.some((i) => i.item.type === 2 && i.x === 5 && i.y === 5)).toBe(false);
  });
});
