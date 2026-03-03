import WebSocket from 'ws';
import { C2SMessage, S2CMessage } from './protocol';
import { World } from './world';

interface Player {
  id: number;
  name: string;
  avatar: string;
  room: number;
  x: number;
  y: number;
  ws: WebSocket;
}

export class GameSession {
  private players = new Map<number, Player>();
  private wsToId = new Map<WebSocket, number>();
  private nextId = 1;
  private world: World;

  constructor(world: World) {
    this.world = world;
  }

  handleConnection(ws: WebSocket): void {
    ws.on('message', (data) => {
      let msg: C2SMessage;
      try {
        msg = JSON.parse(data.toString()) as C2SMessage;
      } catch {
        return;
      }

      const playerId = this.wsToId.get(ws);

      if (msg.type === 'JOIN') {
        this.onJoin(ws, msg);
      } else if (playerId !== undefined) {
        if (msg.type === 'MY_LOCATION') {
          this.onLocation(playerId, msg);
        } else if (msg.type === 'MESSAGE') {
          this.onMessage(playerId, msg);
        } else if (msg.type === 'LEAVING_GAME') {
          this.onLeave(playerId);
        }
      }
    });

    ws.on('close', () => {
      const playerId = this.wsToId.get(ws);
      if (playerId !== undefined) this.onLeave(playerId);
    });

    ws.on('error', () => {
      const playerId = this.wsToId.get(ws);
      if (playerId !== undefined) this.onLeave(playerId);
    });
  }

  private onJoin(ws: WebSocket, msg: Extract<C2SMessage, { type: 'JOIN' }>): void {
    const nameTaken = [...this.players.values()].some((p) => p.name === msg.name);
    if (nameTaken) {
      this.send(ws, { type: 'REJECTED', msg: `Name "${msg.name}" is already taken.` });
      return;
    }

    const id = this.nextId++;
    const player: Player = { id, name: msg.name, avatar: msg.avatar, room: 0, x: 10, y: 10, ws };
    this.players.set(id, player);
    this.wsToId.set(ws, id);

    // Tell the new player they're accepted and send existing players' info
    this.send(ws, {
      type: 'ACCEPTED',
      id,
      msg: `Welcome to Griljor, ${msg.name}!`,
      mapName: this.world.mapName,
      rooms: this.world.roomCount,
    });

    for (const other of this.players.values()) {
      if (other.id === id) continue;
      this.send(ws, {
        type: 'PLAYER_INFO',
        id: other.id,
        name: other.name,
        avatar: other.avatar,
        room: other.room,
        x: other.x,
        y: other.y,
      });
    }

    // Tell everyone else about the new player
    this.broadcast({
      type: 'PLAYER_INFO',
      id,
      name: player.name,
      avatar: player.avatar,
      room: player.room,
      x: player.x,
      y: player.y,
    }, id);

    console.log(`[+] ${msg.name} (id=${id}) joined. Players: ${this.players.size}`);
  }

  private onLocation(playerId: number, msg: Extract<C2SMessage, { type: 'MY_LOCATION' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    player.room = msg.room;
    player.x = msg.x;
    player.y = msg.y;
    this.broadcast({ type: 'MY_LOCATION', id: playerId, room: msg.room, x: msg.x, y: msg.y });
  }

  private onMessage(playerId: number, msg: Extract<C2SMessage, { type: 'MESSAGE' }>): void {
    const s2c: S2CMessage = { type: 'MESSAGE', from: playerId, to: msg.to, text: msg.text };
    if (msg.to === 'all') {
      this.broadcast(s2c);
    } else {
      const target = this.players.get(msg.to as number);
      if (target) this.send(target.ws, s2c);
      // Also echo back to sender
      const sender = this.players.get(playerId);
      if (sender) this.send(sender.ws, s2c);
    }
  }

  private onLeave(playerId: number): void {
    const player = this.players.get(playerId);
    if (!player) return;
    this.players.delete(playerId);
    this.wsToId.delete(player.ws);
    this.broadcast({ type: 'LEAVING_GAME', id: playerId });
    console.log(`[-] ${player.name} (id=${playerId}) left. Players: ${this.players.size}`);
  }

  private send(ws: WebSocket, msg: S2CMessage): void {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify(msg));
    }
  }

  private broadcast(msg: S2CMessage, exceptId?: number): void {
    for (const player of this.players.values()) {
      if (exceptId !== undefined && player.id === exceptId) continue;
      this.send(player.ws, msg);
    }
  }
}
