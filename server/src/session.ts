import WebSocket from 'ws';
import { C2SMessage, S2CMessage, InventoryItem } from './protocol';
import { World } from './world';
import { filterText, randomScold } from './filter';

const INV_SIZE = 35;
const MAX_WEIGHT = 150;

interface Player {
  id: number;
  name: string;
  avatar: string;
  room: number;
  x: number;
  y: number;
  ws: WebSocket;
  kills: number;
  deaths: number;
  joinedAt: number;
  leftHand: InventoryItem | null;
  rightHand: InventoryItem | null;
  inventory: Array<InventoryItem | null>;
  currentWeight: number;
}

interface ChatEntry {
  from: number;
  name: string;
  text: string;
}

export class GameSession {
  private players = new Map<number, Player>();
  private wsToId = new Map<WebSocket, number>();
  private nextId = 1;
  private world: World;
  private chatHistory: ChatEntry[] = [];

  // roomItems: Map<roomIndex, Map<"x,y", InventoryItem>>
  private roomItems = new Map<number, Map<string, InventoryItem>>();

  constructor(world: World) {
    this.world = world;
    this.initRoomItems();
  }

  private initRoomItems(): void {
    for (let roomIdx = 0; roomIdx < this.world.rooms.length; roomIdx++) {
      const room = this.world.rooms[roomIdx];
      const itemMap = new Map<string, InventoryItem>();
      for (const ro of room.recorded_objects) {
        if (ro.type <= 0) continue;
        const obj = this.world.objects[ro.type];
        if (!obj?.takeable) continue;
        const quantity = ro.detail > 0 ? ro.detail : 1;
        itemMap.set(`${ro.x},${ro.y}`, { type: ro.type, quantity });
      }
      this.roomItems.set(roomIdx, itemMap);
    }
  }

  get playerCount(): number { return this.players.size; }

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
        } else if (msg.type === 'PICKUP') {
          this.onPickup(playerId, msg);
        } else if (msg.type === 'DROP') {
          this.onDrop(playerId, msg);
        } else if (msg.type === 'INV_SWAP') {
          this.onInvSwap(playerId, msg);
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
    const player: Player = {
      id, name: msg.name, avatar: msg.avatar, room: 0, x: 10, y: 10, ws,
      kills: 0, deaths: 0, joinedAt: Date.now(),
      leftHand: null, rightHand: null,
      inventory: new Array<InventoryItem | null>(INV_SIZE).fill(null),
      currentWeight: 0,
    };
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
        kills: other.kills,
        deaths: other.deaths,
        joinedAt: other.joinedAt,
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
      kills: player.kills,
      deaths: player.deaths,
      joinedAt: player.joinedAt,
    }, id);

    // Replay chat history for the new player
    for (const entry of this.chatHistory) {
      this.send(ws, { type: 'MESSAGE', from: entry.from, name: entry.name, to: 'all', text: entry.text });
    }

    // Send current floor items to new player
    const syncItems: Array<{ room: number; x: number; y: number; item: InventoryItem }> = [];
    for (const [roomIdx, itemMap] of this.roomItems) {
      for (const [key, item] of itemMap) {
        const [sx, sy] = key.split(',').map(Number);
        syncItems.push({ room: roomIdx, x: sx, y: sy, item });
      }
    }
    this.send(ws, { type: 'ITEMS_SYNC', items: syncItems });

    // Send empty inventory to new player
    this.sendInventory(player);

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
    const sender = this.players.get(playerId);
    if (!sender) return;
    const { filtered, triggered } = filterText(msg.text);
    const s2c: S2CMessage = { type: 'MESSAGE', from: playerId, name: sender.name, to: msg.to, text: filtered };
    if (msg.to === 'all') {
      this.chatHistory.push({ from: playerId, name: sender.name, text: filtered });
      this.broadcast(s2c);
      if (triggered) {
        const gmMsg: S2CMessage = { type: 'MESSAGE', from: 0, name: 'GM', to: 'all', text: randomScold() };
        this.broadcast(gmMsg);
        this.chatHistory.push({ from: 0, name: 'GM', text: gmMsg.text });
      }
    } else {
      const target = this.players.get(msg.to as number);
      if (target) this.send(target.ws, s2c);
      this.send(sender.ws, s2c);
    }
  }

  private onPickup(playerId: number, msg: Extract<C2SMessage, { type: 'PICKUP' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;

    const roomMap = this.roomItems.get(player.room);
    const key = `${msg.x},${msg.y}`;
    const item = roomMap?.get(key);
    if (!item) return;

    const obj = this.world.objects[item.type];
    const itemWeight = (obj?.weight ?? 0) * item.quantity;

    if (player.currentWeight + itemWeight > MAX_WEIGHT) {
      this.send(player.ws, {
        type: 'MESSAGE', from: 0, name: 'GM', to: player.id,
        text: 'That is too heavy to carry.',
      });
      return;
    }

    // Determine target hand slot
    const hand = msg.hand;
    const handOccupied = hand === 'left' ? player.leftHand !== null : player.rightHand !== null;

    if (!handOccupied) {
      // Place in hand
      if (hand === 'left') player.leftHand = item;
      else player.rightHand = item;
    } else {
      // Hand occupied — find first free inventory slot
      const freeSlot = player.inventory.indexOf(null);
      if (freeSlot === -1) {
        // Inventory full too
        this.send(player.ws, {
          type: 'MESSAGE', from: 0, name: 'GM', to: player.id,
          text: 'Your hands are full.',
        });
        return;
      }
      player.inventory[freeSlot] = item;
    }

    roomMap!.delete(key);
    player.currentWeight += itemWeight;

    this.broadcast({ type: 'ITEM_REMOVED', room: player.room, x: msg.x, y: msg.y });
    this.sendInventory(player);
  }

  private onDrop(playerId: number, msg: Extract<C2SMessage, { type: 'DROP' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;

    let item: InventoryItem | null = null;
    if (msg.source === 'left') {
      item = player.leftHand;
      player.leftHand = null;
    } else if (msg.source === 'right') {
      item = player.rightHand;
      player.rightHand = null;
    } else if (typeof msg.source === 'number' && msg.source >= 0 && msg.source < INV_SIZE) {
      item = player.inventory[msg.source];
      player.inventory[msg.source] = null;
    }

    if (!item) return;

    const obj = this.world.objects[item.type];
    const itemWeight = (obj?.weight ?? 0) * item.quantity;
    player.currentWeight = Math.max(0, player.currentWeight - itemWeight);

    const tile = this.nearbyFreeTile(player.room, player.x, player.y);
    if (tile) {
      const roomMap = this.roomItems.get(player.room) ?? new Map<string, InventoryItem>();
      roomMap.set(`${tile.x},${tile.y}`, item);
      this.roomItems.set(player.room, roomMap);
      this.broadcast({ type: 'ITEM_ADDED', room: player.room, x: tile.x, y: tile.y, item });
    } else {
      console.log(`[inv] item type=${item.type} lost — no free tile near (${player.x},${player.y}) in room ${player.room}`);
    }

    this.sendInventory(player);
  }

  private onInvSwap(playerId: number, msg: Extract<C2SMessage, { type: 'INV_SWAP' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    if (msg.slot < 0 || msg.slot >= INV_SIZE) return;

    const slotItem = player.inventory[msg.slot];
    const handItem = msg.hand === 'left' ? player.leftHand : player.rightHand;

    player.inventory[msg.slot] = handItem;
    if (msg.hand === 'left') player.leftHand = slotItem;
    else player.rightHand = slotItem;

    this.sendInventory(player);
  }

  private onLeave(playerId: number): void {
    const player = this.players.get(playerId);
    if (!player) return;

    // Drop all held items to floor
    const heldItems: Array<InventoryItem | null> = [
      player.leftHand, player.rightHand, ...player.inventory
    ];
    for (const item of heldItems) {
      if (!item) continue;
      const tile = this.nearbyFreeTile(player.room, player.x, player.y);
      if (tile) {
        const roomMap = this.roomItems.get(player.room) ?? new Map<string, InventoryItem>();
        roomMap.set(`${tile.x},${tile.y}`, item);
        this.roomItems.set(player.room, roomMap);
        this.broadcast({ type: 'ITEM_ADDED', room: player.room, x: tile.x, y: tile.y, item });
      }
    }

    this.players.delete(playerId);
    this.wsToId.delete(player.ws);
    this.broadcast({ type: 'LEAVING_GAME', id: playerId });
    if (this.players.size === 0) {
      this.chatHistory = [];
      console.log('[chat] history cleared (server empty)');
    }
    console.log(`[-] ${player.name} (id=${playerId}) left. Players: ${this.players.size}`);
  }

  private sendInventory(player: Player): void {
    this.send(player.ws, {
      type: 'YOUR_INVENTORY',
      leftHand: player.leftHand,
      rightHand: player.rightHand,
      inventory: player.inventory,
      currentWeight: player.currentWeight,
      maxWeight: MAX_WEIGHT,
    });
  }

  private nearbyFreeTile(roomIdx: number, px: number, py: number): { x: number; y: number } | null {
    const room = this.world.rooms[roomIdx];
    if (!room) return null;
    const roomMap = this.roomItems.get(roomIdx) ?? new Map<string, InventoryItem>();
    const GRID = 20;

    // Spiral search outward from player position
    for (let radius = 0; radius <= 5; radius++) {
      for (let dx = -radius; dx <= radius; dx++) {
        for (let dy = -radius; dy <= radius; dy++) {
          if (Math.abs(dx) !== radius && Math.abs(dy) !== radius) continue;
          const tx = px + dx;
          const ty = py + dy;
          if (tx < 0 || tx >= GRID || ty < 0 || ty >= GRID) continue;
          if (roomMap.has(`${tx},${ty}`)) continue;
          // Check if tile is passable (not a solid wall)
          const [flId, wlId] = room.spot[tx][ty];
          const wallObj = wlId > 0 ? this.world.objects[wlId] : null;
          const floorObj = flId > 0 ? this.world.objects[flId] : null;
          if (wallObj && !wallObj.permeable) continue;
          if (floorObj && !floorObj.permeable) continue;
          return { x: tx, y: ty };
        }
      }
    }
    return null;
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
