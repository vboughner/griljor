import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { World, ObjDef, RoomData } from '../../world';
import { joinPlayer } from './helpers';

// Extended test world that adds:
//   3 = potion  (takeable, lost, health:-20, weight:2)  — placed at (3,3) and (4,3)
//   5 = grenade (takeable, lost, weapon, damage:40)      — placed at (7,7)
function buildConsumableWorld(): World {
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );

  const objects: Array<ObjDef | null> = [
    null,
    { _index: 1, name: 'floor', movement: 5, permeable: true },
    { _index: 2, name: 'sword', takeable: true, weight: 5, weapon: true, damage: 30, range: 5 },
    { _index: 3, name: 'potion', takeable: true, weight: 2, health: -20, lost: true },
    { _index: 4, name: 'wall', movement: 0 },
    {
      _index: 5,
      name: 'grenade',
      takeable: true,
      weight: 5,
      weapon: true,
      lost: true,
      damage: 40,
      range: 5,
    },
  ];

  const room: RoomData = {
    name: 'test-room',
    floor: 0,
    team: 0,
    recorded_objects: [
      { x: 5, y: 5, type: 2, detail: 0 }, // sword
      { x: 3, y: 3, type: 3, detail: 0 }, // potion A
      { x: 4, y: 3, type: 3, detail: 0 }, // potion B (for auto-reload tests)
      { x: 7, y: 7, type: 5, detail: 0 }, // grenade
    ],
    spot,
  };

  return {
    mapName: 'consumable-test',
    title: 'Consumable Test',
    teams: 0,
    roomCount: 1,
    rooms: [room],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
  };
}

describe('consumables', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildConsumableWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  // Bob picks up the sword and fires once at alice (at x=10, y=10).
  // Bob must be at (9,10), alice at (10,10) before calling.
  // Advance 500ms: missile lands (~227ms), no regen tick fires (< 1000ms).
  function bobShotsAliceOnce(
    bob: ReturnType<typeof joinPlayer>,
    alice: ReturnType<typeof joinPlayer>,
  ) {
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    bob.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' }); // sword
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 9, y: 10 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 });
    bob.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });
    vi.advanceTimersByTime(500);
  }

  // ── Healing ─────────────────────────────────────────────────────────────

  it('using a potion heals the player', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Alice picks up a potion into left hand
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' });

    // Bob damages Alice by 30 HP → she's at 70
    bobShotsAliceOnce(bob, alice);
    alice.ws.flush();

    // Alice uses potion on herself (heals 20 → 90 HP)
    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 10, targetY: 10 });

    const stats = alice.ws.lastOfType('YOUR_STATS');
    expect(stats).toBeDefined();
    expect(stats!.hp).toBe(90); // 70 + 20
  });

  it('using a potion broadcasts PLAYER_HEAL to the room', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' });
    bobShotsAliceOnce(bob, alice);
    bob.ws.flush();

    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 10, targetY: 10 });

    const heals = bob.ws.messagesOfType('PLAYER_HEAL');
    expect(heals.some((m) => m.playerId === alice.id)).toBe(true);
  });

  // ── Consumption / item removal ───────────────────────────────────────────

  it('lost potion is removed from hand after use', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' });
    bobShotsAliceOnce(bob, alice);
    alice.ws.flush();

    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand).toBeNull();
  });

  it('potion not consumed when HP is already full', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' });
    alice.ws.flush();

    // Alice is at full HP — use should be silently rejected
    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 3, targetY: 3 });

    expect(alice.ws.messagesOfType('YOUR_INVENTORY').length).toBe(0);
  });

  // ── Burden ───────────────────────────────────────────────────────────────

  it('burden decreases when a lost potion is consumed', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' }); // potion weight=2
    bobShotsAliceOnce(bob, alice);
    alice.ws.flush();

    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv!.currentWeight).toBe(0);
  });

  // ── Auto-reload ──────────────────────────────────────────────────────────

  it('auto-reloads matching potion from inventory after hand is emptied', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Pick up potion A into left hand, potion B spills into inventory
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3, hand: 'left' }); // potion A → left hand
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 4, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 4, y: 3, hand: 'left' }); // potion B → inventory[0]

    // Damage alice enough to use two potions (40 HP so both potions restore up to cap)
    bobShotsAliceOnce(bob, alice); // -30 → 70 HP
    alice.ws.flush();

    alice.ws.receive({ type: 'USE_ITEM', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    // Potion A consumed → potion B should auto-move to left hand
    expect(inv!.leftHand).not.toBeNull();
    expect(inv!.leftHand!.type).toBe(3);
    expect(inv!.inventory.every((s) => s === null)).toBe(true);
  });

  // ── PLAYER_HIT broadcast ─────────────────────────────────────────────────

  it('PLAYER_HIT is broadcast to room when a player takes damage', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    bob.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });
    alice.ws.flush();

    bob.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
    vi.advanceTimersByTime(500);

    const hits = alice.ws.messagesOfType('PLAYER_HIT');
    expect(hits.some((m) => m.victimId === alice.id && m.damage > 0)).toBe(true);
  });

  it('PLAYER_HIT is not sent when the target is already dead', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Arm and position bob to kill alice (4 × 30 = 120 > 100)
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    bob.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 2, y: 1 });

    for (let i = 0; i < 4; i++) {
      bob.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
      vi.advanceTimersByTime(900);
    }

    alice.ws.flush();

    // Alice is dead — fire at her again
    bob.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 2, targetY: 1 });
    vi.advanceTimersByTime(500);

    expect(alice.ws.messagesOfType('PLAYER_HIT').length).toBe(0);
  });

  // ── Lost weapons (grenades) ──────────────────────────────────────────────

  it('lost weapon is removed from hand after being fired', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 7 });
    alice.ws.receive({ type: 'PICKUP', x: 7, y: 7, hand: 'left' }); // grenade
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv).toBeDefined();
    expect(inv!.leftHand).toBeNull();
  });

  it('firing a lost weapon decrements burden', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 7 });
    alice.ws.receive({ type: 'PICKUP', x: 7, y: 7, hand: 'left' }); // grenade weight=5
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv!.currentWeight).toBe(0);
  });

  it('auto-reloads matching grenade from inventory after firing', () => {
    // Pick up one grenade into left hand, sword into right (different type — no reload),
    // and verify that after firing the grenade, no reload happens (nothing matching in inv).
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 7 });
    alice.ws.receive({ type: 'PICKUP', x: 7, y: 7, hand: 'left' }); // grenade → left hand
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'right' }); // sword → right hand
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
    alice.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', hand: 'left', targetX: 10, targetY: 10 });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    // No second grenade in inventory → left hand stays null after firing
    expect(inv!.leftHand).toBeNull();
    // Right hand (sword) is untouched
    expect(inv!.rightHand?.type).toBe(2);
  });

  // ── Pickup blocked when tile occupied ────────────────────────────────────

  it('cannot pick up item when another player is standing on that tile', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    // Bob stands on the sword tile
    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.flush();

    // Alice tries to pick up the sword from (5,5) where Bob is standing
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    expect(alice.ws.messagesOfType('YOUR_INVENTORY').length).toBe(0);
    expect(alice.ws.messagesOfType('ITEM_REMOVED').length).toBe(0);
  });

  it('can pick up item when no other player is on the tile', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');

    bob.ws.receive({ type: 'MY_LOCATION', room: 0, x: 10, y: 10 }); // Bob elsewhere
    alice.ws.flush();

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
    alice.ws.receive({ type: 'PICKUP', x: 5, y: 5, hand: 'left' });

    const inv = alice.ws.lastOfType('YOUR_INVENTORY');
    expect(inv!.leftHand?.type).toBe(2); // sword
  });
});
