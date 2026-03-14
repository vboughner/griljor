import { EventEmitter } from 'events';
import WebSocket from 'ws';
import { GameSession } from '../../session';
import { C2SMessage, S2CMessage } from '../../protocol';
import { World, ObjDef, RoomData } from '../../world';

// ── MockWebSocket ──────────────────────────────────────────────────────────

export class MockWebSocket extends EventEmitter {
  readyState = WebSocket.OPEN as number;
  private _sent: string[] = [];

  // Called by GameSession to send a message to this "client"
  send(data: string) {
    this._sent.push(data);
  }

  // Test helper: simulate the client sending a C→S message
  receive(msg: C2SMessage) {
    this.emit('message', JSON.stringify(msg));
  }

  // Test helper: simulate disconnect
  close() {
    this.readyState = WebSocket.CLOSED as number;
    this.emit('close');
  }

  // All S→C messages received so far
  messages(): S2CMessage[] {
    return this._sent.map((s) => JSON.parse(s) as S2CMessage);
  }

  messagesOfType<T extends S2CMessage['type']>(type: T): Extract<S2CMessage, { type: T }>[] {
    return this.messages().filter((m): m is Extract<S2CMessage, { type: T }> => m.type === type);
  }

  lastOfType<T extends S2CMessage['type']>(type: T): Extract<S2CMessage, { type: T }> | undefined {
    return this.messagesOfType(type).at(-1);
  }

  // Flush recorded messages (useful to clear before an action)
  flush() {
    this._sent = [];
  }
}

// ── buildTestWorld ─────────────────────────────────────────────────────────
//
// Object index layout:
//   0 = null (no object)
//   1 = floor tile (movement:5, walkable)
//   2 = sword    (takeable, weapon, damage:30, range:5)
//   3 = potion   (takeable, weight:1, health:-20)
//   4 = wall     (movement:0, blocks passage)

export function buildTestWorld(): World {
  // 20×20 grid; all cells are [floorId=1, wallId=0] → walkable
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );

  const objects: Array<ObjDef | null> = [
    null, // index 0
    { _index: 1, name: 'floor', movement: 5, permeable: true }, // walkable, missile-passable floor
    { _index: 2, name: 'sword', takeable: true, weight: 5, weapon: true, damage: 30, range: 5 },
    { _index: 3, name: 'potion', takeable: true, weight: 1, health: -20 },
    { _index: 4, name: 'wall', movement: 0 }, // impassable wall
  ];

  const room: RoomData = {
    name: 'test-room',
    floor: 0,
    team: 0,
    // sword placed at (5,5), potion at (6,6)
    recorded_objects: [
      { x: 5, y: 5, type: 2, detail: 0 },
      { x: 6, y: 6, type: 3, detail: 0 },
    ],
    spot,
  };

  return {
    mapName: 'test',
    title: 'Test Map',
    teams: 0,
    roomCount: 1,
    rooms: [room],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
  };
}

// ── joinPlayer ─────────────────────────────────────────────────────────────

export interface TestPlayer {
  ws: MockWebSocket;
  id: number;
  room: number;
  x: number;
  y: number;
}

export function joinPlayer(session: GameSession, name = 'Alice', avatar = 'a'): TestPlayer {
  const ws = new MockWebSocket();
  session.handleConnection(ws as unknown as WebSocket);
  ws.receive({ type: 'JOIN', name, avatar });
  const accepted = ws.lastOfType('ACCEPTED');
  if (!accepted) throw new Error(`JOIN was rejected for ${name}`);
  return { ws, id: accepted.id, room: accepted.room, x: accepted.x, y: accepted.y };
}
