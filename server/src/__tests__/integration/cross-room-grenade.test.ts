import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { MockWebSocket, buildTwoRoomWorld, joinPlayer } from './helpers';

// Grenade: speed=6 → msPerStep = max(50, round(1500/(6*2.2))) = 114ms
// Explosion ray: speed=4 → msPerStep = max(50, round(1500/(4*2.2))) = 170ms
const GRENADE_MPS = Math.max(50, Math.round(1500 / (6 * 2.2)));
const EXPLOSION_MPS = Math.max(50, Math.round(1500 / (4 * 2.2)));

describe('cross-room grenades', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTwoRoomWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  function place(p: { ws: MockWebSocket }, room: number, x: number, y: number) {
    p.ws.receive({ type: 'MY_LOCATION', room, x, y });
  }

  function giveGrenade(p: { ws: MockWebSocket }) {
    // Move to the grenade tile first so proximity enforcement allows the pickup
    p.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    p.ws.receive({ type: 'PICKUP', x: 3, y: 3 });
  }

  it('sends MISSILE_START in next room when grenade crosses south border', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob', 'b', 1);
    // Give grenade first, then position Alice for the throw
    giveGrenade(alice);
    // Place Alice at y=17 so grenade range=4 hits y=19 (border) after only 2 steps,
    // leaving 2 remaining range to continue into the south room.
    place(alice, 0, 10, 17);
    place(bob, 1, 10, 1);
    bob.ws.flush();

    // Grenade range=4, from y=17: path y=18,19 (2 steps) → hits south border with 2 remaining
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 25 });
    vi.advanceTimersByTime(2 * GRENADE_MPS + 10);

    const starts = bob.ws.messagesOfType('MISSILE_START');
    expect(starts.length).toBeGreaterThan(0);
    expect(starts[0].room).toBe(1);
  });

  it('Bob in room 1 takes explosion damage from cross-room grenade', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob', 'b', 1);
    giveGrenade(alice);
    // Alice at y=17: grenade hits south border (y=19) after 2 steps, then continues into room 1
    place(alice, 0, 10, 17);
    place(bob, 1, 10, 1);
    bob.ws.flush();

    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 25 });
    // Grenade travel (2 tiles to border) + continuation (2 remaining) + explosion ray travel
    vi.advanceTimersByTime(2 * GRENADE_MPS + 2 * GRENADE_MPS + 4 * EXPLOSION_MPS + 50);

    const health = bob.ws.lastOfType('PLAYER_HEALTH');
    expect(health).toBeDefined();
    expect(health!.hp).toBeLessThan(100);
  });

  it('in-room grenade (does not reach border) explodes in room 0', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob', 'b', 1);
    giveGrenade(alice);
    place(alice, 0, 10, 5);
    place(bob, 1, 10, 1);
    bob.ws.flush();

    // range=4 from y=5 → path stops at y=9, nowhere near y=19
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 9 });
    vi.advanceTimersByTime(4 * GRENADE_MPS + 8 * EXPLOSION_MPS + 50);

    expect(bob.ws.messagesOfType('MISSILE_START')).toHaveLength(0);
  });

  it('grenade stays in room 0 when south exit is -1', () => {
    const world = buildTwoRoomWorld();
    world.rooms[0].exitSouth = -1;
    const s2 = new GameSession(world);

    const alice = joinPlayer(s2, 'Alice');
    const bob = joinPlayer(s2, 'Bob', 'b', 1);
    // Alice at y=17: grenade hits south border after 2 steps but won't cross (no exit)
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 3, y: 3 });
    alice.ws.receive({ type: 'PICKUP', x: 3, y: 3 });
    place(alice, 0, 10, 17);
    place(bob, 1, 10, 1);
    bob.ws.flush();

    // Just verify no cross-room by checking bob gets no MISSILE_START
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 25 });
    vi.advanceTimersByTime(2 * GRENADE_MPS + 8 * EXPLOSION_MPS + 50);

    expect(bob.ws.messagesOfType('MISSILE_START')).toHaveLength(0);
    s2.destroy();
  });

  it('grenade that hits a player on the border tile does not cross rooms', () => {
    const alice = joinPlayer(session, 'Alice');
    const charlie = joinPlayer(session, 'Charlie', 'c', 1);
    const bob = joinPlayer(session, 'Bob', 'b', 1);
    giveGrenade(alice);
    place(alice, 0, 10, 15);
    place(charlie, 0, 10, 19); // standing on the south border tile
    place(bob, 1, 10, 1); // in room 1
    charlie.ws.flush();
    bob.ws.flush();

    // Grenade flies from y=15 → y=16,17,18,19 (hits Charlie at border)
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 25 });
    vi.advanceTimersByTime(4 * GRENADE_MPS + 8 * EXPLOSION_MPS + 50);

    // Charlie takes direct hit damage
    expect(charlie.ws.lastOfType('PLAYER_HEALTH')).toBeDefined();
    // Bob in room 1 gets no missile
    expect(bob.ws.messagesOfType('MISSILE_START')).toHaveLength(0);
  });

  it('diagonal throw does not cross rooms', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob', 'b', 1);
    giveGrenade(alice);
    place(alice, 0, 15, 15);
    place(bob, 1, 0, 0);
    bob.ws.flush();

    // Diagonal SE: dx=1,dy=1 → getRoomExit returns -1
    alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 19, targetY: 19 });
    vi.advanceTimersByTime(4 * GRENADE_MPS + 8 * EXPLOSION_MPS + 50);

    expect(bob.ws.messagesOfType('MISSILE_START')).toHaveLength(0);
  });
});
