import WebSocket from 'ws';
import { C2SMessage, S2CMessage, InventoryItem } from './protocol';
import { World, ObjDef, RecObj, RoomData } from './world';
import { filterText, randomScold } from './filter';

const INV_SIZE = 21;
const MAX_WEIGHT = 150;
const GRID = 20;
const RESPAWN_DELAY_MS = 5000;
export const PICKUP_RANGE = 4; // max Chebyshev distance to pick up an item

// AFK idle detection
const AFK_IDLE_MS = 5 * 60 * 1000; // idle time before first warning (5 min)
const AFK_WARN_INTERVAL_MS = 1 * 60 * 1000; // interval between warnings (1 min)
const AFK_GRACE_MINUTES = 5; // number of warnings before kick

const EXPLOSION_DIRS = [
  { dx: 0, dy: -1 },
  { dx: 1, dy: -1 },
  { dx: 1, dy: 0 },
  { dx: 1, dy: 1 },
  { dx: 0, dy: 1 },
  { dx: -1, dy: 1 },
  { dx: -1, dy: 0 },
  { dx: -1, dy: -1 },
] as const;

export const PUNCH_DAMAGE = 10;
export const PUNCH_COOLDOWN_MS = 400;

/**
 * For numbered items (guns, potions with charges), quantity represents the
 * charge count — the item itself is still ONE physical object. Weight is
 * per-item, not per-charge.  For non-numbered stackable items, multiply by
 * the stack size.
 */
// ── Line-of-sight helpers ──────────────────────────────────────────────────

/**
 * Walk from (x1,y1) toward (x2,y2) one Chebyshev step at a time.
 * Returns each step's tile, NOT including the start tile (x1,y1).
 * Stops when (x2,y2) is reached.
 */
export function chebyshevPath(
  x1: number,
  y1: number,
  x2: number,
  y2: number,
): Array<{ x: number; y: number }> {
  const path: Array<{ x: number; y: number }> = [];
  let cx = x1;
  let cy = y1;
  while (cx !== x2 || cy !== y2) {
    if (cx !== x2) cx += Math.sign(x2 - cx);
    if (cy !== y2) cy += Math.sign(y2 - cy);
    path.push({ x: cx, y: cy });
  }
  return path;
}

/**
 * Returns true if any object on tile (x,y) does NOT have transparent:true,
 * meaning it blocks line of sight.
 */
export function tileViewBlocked(
  room: RoomData,
  objects: Array<ObjDef | null>,
  x: number,
  y: number,
): boolean {
  const cell = room.spot?.[x]?.[y];
  if (cell) {
    const [flId, wlId] = cell;
    const flObj = flId > 0 ? objects[flId] : null;
    if (flObj != null && flObj.transparent !== true) return true;
    const wlObj = wlId > 0 ? objects[wlId] : null;
    if (wlObj != null && wlObj.transparent !== true) return true;
  }
  for (const ro of room.recorded_objects) {
    if (ro.x === x && ro.y === y) {
      const roObj = objects[ro.type];
      if (roObj != null && roObj.transparent !== true) return true;
    }
  }
  return false;
}

/**
 * Returns true if tile (x2,y2) is visible from tile (x1,y1).
 * Adjacent tiles (1 Chebyshev step) are always visible.
 * The looker's own tile (x1,y1) is not checked; the target tile IS checked.
 */
export function spotIsVisible(
  room: RoomData,
  objects: Array<ObjDef | null>,
  x1: number,
  y1: number,
  x2: number,
  y2: number,
): boolean {
  if (x1 === x2 && y1 === y2) return true;
  const path = chebyshevPath(x1, y1, x2, y2);
  if (path.length <= 1) return true; // adjacent — always visible
  for (const { x, y } of path) {
    if (tileViewBlocked(room, objects, x, y)) return false;
  }
  return true;
}

export function calcItemWeight(obj: ObjDef | null | undefined, item: InventoryItem): number {
  if (!obj) return 0;
  if (obj.numbered) return obj.weight ?? 0;
  return (obj.weight ?? 0) * item.quantity;
}

/** Cooldown in ms based on weapon refire field.
 *  refire=0 → 850ms, refire=5 → 0ms, refire=-5 → 1700ms */
export function calcFireCooldown(refire?: number): number {
  const x = Math.max(-5, Math.min(5, refire ?? 0));
  return Math.round(850 * (1 - x / 5));
}

/** Travel time per tile for a missile of the given speed (1–9).
 *  speed=9 → ~76ms/step, speed=1 → ~682ms/step, floor 50ms. */
export function calcMsPerStep(speed: number): number {
  return Math.max(50, Math.round(1500 / (speed * 2.2)));
}

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
  inventory: Array<InventoryItem | null>;
  currentWeight: number;
  team: number; // 1-based team number (0 = neutral)
  // combat stats
  hp: number;
  maxHp: number;
  dead: boolean;
  respawnTimer: ReturnType<typeof setTimeout> | null;
  lastFireTime: number;
  lastPunchedAt: number;
  afkIdleTimer: ReturnType<typeof setTimeout> | null;
  afkWarnTimer: ReturnType<typeof setTimeout> | null;
  afkWarningsLeft: number;
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

  // snapshot of original recorded_objects for reset (deep copy taken at construction)
  private originalRecordedObjects: RecObj[][] = [];

  // pending reset timer (cancelled if a player joins before it fires)
  private resetTimer: ReturnType<typeof setTimeout> | null = null;

  // active missiles: id → pending damage/end timer
  private nextMissileId = 1;
  private activeMissiles = new Map<number, ReturnType<typeof setTimeout>>();

  private onPlayerCountChange?: () => void;
  private regenInterval: ReturnType<typeof setInterval> | null = null;

  // viewerId → Set of visible player IDs (symmetric)
  private visibility = new Map<number, Set<number>>();

  constructor(world: World, opts?: { onPlayerCountChange?: () => void }) {
    this.onPlayerCountChange = opts?.onPlayerCountChange;
    this.world = world;
    this.originalRecordedObjects = world.rooms.map((r) =>
      r.recorded_objects.map((ro) => ({ ...ro })),
    );
    this.initRoomItems();
    this.regenInterval = setInterval(() => this.regenTick(), 1000);
  }

  destroy(): void {
    for (const player of this.players.values()) {
      this.clearAfkTimers(player);
    }
    if (this.regenInterval !== null) {
      clearInterval(this.regenInterval);
      this.regenInterval = null;
    }
  }

  private regenTick(): void {
    for (const player of this.players.values()) {
      if (player.dead || player.hp >= player.maxHp) continue;
      player.hp = Math.min(player.maxHp, player.hp + 1);
      this.broadcast({ type: 'PLAYER_HEALTH', id: player.id, hp: player.hp, maxHp: player.maxHp });
    }
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

  private resetWorldState(): void {
    for (let i = 0; i < this.world.rooms.length; i++) {
      this.world.rooms[i].recorded_objects = this.originalRecordedObjects[i].map((ro) => ({
        ...ro,
      }));
    }
    this.roomItems.clear();
    this.initRoomItems();
    this.chatHistory = [];
  }

  get playerCount(): number {
    return this.players.size;
  }
  get playerAvatars(): Array<{ avatar: string; name: string; team: number }> {
    return [...this.players.values()].map((p) => ({
      avatar: p.avatar,
      name: p.name,
      team: p.team,
    }));
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
        const afkPlayer = this.players.get(playerId);
        if (afkPlayer && msg.type !== 'PING') this.resetAfkTimer(afkPlayer);
        switch (msg.type) {
          case 'MY_LOCATION':
            this.onLocation(playerId, msg);
            break;
          case 'MESSAGE':
            this.onMessage(playerId, msg);
            break;
          case 'LEAVING_GAME':
            this.onLeave(playerId);
            break;
          case 'PICKUP':
            this.onPickup(playerId, msg);
            break;
          case 'DROP':
            this.onDrop(playerId, msg);
            break;
          case 'INV_SWAP':
            this.onInvSwap(playerId, msg);
            break;
          case 'FIRE_WEAPON':
            this.onFireWeapon(playerId, msg);
            break;
          case 'USE_ITEM':
            this.onUseItem(playerId, msg);
            break;
          case 'VOLUNTARY_RESPAWN':
            this.onVoluntaryRespawn(playerId);
            break;
          case 'PING':
            break; // no-op: keeps the connection alive
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

  private initVisibility(id: number): void {
    this.visibility.set(id, new Set());
  }

  private clearVisibility(id: number): void {
    const visSet = this.visibility.get(id);
    if (visSet) {
      for (const otherId of visSet) {
        this.visibility.get(otherId)?.delete(id);
      }
    }
    this.visibility.delete(id);
  }

  private onJoin(ws: WebSocket, msg: Extract<C2SMessage, { type: 'JOIN' }>): void {
    if (this.resetTimer !== null) {
      clearTimeout(this.resetTimer);
      this.resetTimer = null;
      console.log(`[reset] cancelled (${this.world.mapName} has a new player)`);
    }

    const nameTaken = [...this.players.values()].some((p) => p.name === msg.name);
    if (nameTaken) {
      this.send(ws, { type: 'REJECTED', msg: `Name "${msg.name}" is already taken.` });
      return;
    }

    const id = this.nextId++;
    const team =
      typeof msg.team === 'number' && msg.team >= 1 && msg.team <= this.world.teams ? msg.team : 1;
    const player: Player = {
      id,
      name: msg.name,
      avatar: msg.avatar,
      room: 0,
      x: 10,
      y: 10,
      ws,
      kills: 0,
      deaths: 0,
      joinedAt: Date.now(),
      leftHand: null,
      inventory: new Array<InventoryItem | null>(INV_SIZE).fill(null),
      currentWeight: 0,
      team,
      hp: 100,
      maxHp: 100,
      dead: false,
      respawnTimer: null,
      lastFireTime: 0,
      lastPunchedAt: 0,
      afkIdleTimer: null,
      afkWarnTimer: null,
      afkWarningsLeft: 0,
    };
    this.players.set(id, player);

    // Place player in a random walkable tile in their team's room
    const spawn = this.randomSpawnForTeam(team);
    if (spawn) {
      player.room = spawn.room;
      player.x = spawn.x;
      player.y = spawn.y;
    }
    this.wsToId.set(ws, id);
    this.onPlayerCountChange?.();
    this.initVisibility(id);

    // Tell the new player they're accepted and send existing players' info
    this.send(ws, {
      type: 'ACCEPTED',
      id,
      msg: `Welcome to Griljor, ${msg.name}!`,
      mapName: this.world.mapName,
      rooms: this.world.roomCount,
      room: player.room,
      x: player.x,
      y: player.y,
      team: player.team,
    });

    for (const other of this.players.values()) {
      if (other.id === id) continue;
      if (other.room !== player.room) {
        // Different room: no position reveal — players only learn about each other via LOS
        continue;
      } else {
        // Same room: check directional LOS independently for each perspective
        const room = this.world.rooms[player.room];
        if (room) {
          const newCanSeeOther = spotIsVisible(
            room,
            this.world.objects,
            player.x,
            player.y,
            other.x,
            other.y,
          );
          const otherCanSeeNew = spotIsVisible(
            room,
            this.world.objects,
            other.x,
            other.y,
            player.x,
            player.y,
          );
          if (newCanSeeOther) {
            this.visibility.get(id)!.add(other.id);
            this.send(ws, this.makePlayerInfo(other));
            this.send(ws, {
              type: 'PLAYER_HEALTH',
              id: other.id,
              hp: other.hp,
              maxHp: other.maxHp,
            });
          }
          if (otherCanSeeNew) {
            this.visibility.get(other.id)?.add(id);
            this.send(other.ws, this.makePlayerInfo(player));
          }
        }
      }
    }

    // Replay chat history for the new player
    for (const entry of this.chatHistory) {
      this.send(ws, {
        type: 'MESSAGE',
        from: entry.from,
        name: entry.name,
        to: 'all',
        text: entry.text,
      });
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

    // Sync any recorded_objects that have been toggled from their original state
    // (e.g. doors opened by another player before this player joined)
    for (let roomIdx = 0; roomIdx < this.world.rooms.length; roomIdx++) {
      const current = this.world.rooms[roomIdx].recorded_objects;
      const original = this.originalRecordedObjects[roomIdx];
      for (let i = 0; i < current.length && i < original.length; i++) {
        if (current[i].type !== original[i].type) {
          this.send(ws, {
            type: 'ROOM_OBJECT_CHANGED',
            room: roomIdx,
            x: current[i].x,
            y: current[i].y,
            newType: current[i].type,
          });
        }
      }
    }

    // Send empty inventory and starting stats to new player
    this.sendInventory(player);
    this.sendStats(player);

    // Sync health for all players in both directions, regardless of room or LOS
    for (const other of this.players.values()) {
      if (other.id === id) continue;
      this.send(ws, { type: 'PLAYER_HEALTH', id: other.id, hp: other.hp, maxHp: other.maxHp });
      this.send(other.ws, { type: 'PLAYER_HEALTH', id, hp: player.hp, maxHp: player.maxHp });
    }

    console.log(`[+] ${msg.name} (id=${id}) joined. Players: ${this.players.size}`);
    this.startAfkTimer(player);
  }

  private onLocation(playerId: number, msg: Extract<C2SMessage, { type: 'MY_LOCATION' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    if (player.dead) return;

    const prevRoom = player.room;
    if (msg.room !== prevRoom) {
      // Room changed: hide mover from all players in the old room (both directions)
      for (const other of this.players.values()) {
        if (other.id === playerId || other.room !== prevRoom) continue;
        const otherVisSet = this.visibility.get(other.id);
        if (otherVisSet?.has(playerId)) {
          this.send(other.ws, { type: 'PLAYER_HIDDEN', id: playerId });
          otherVisSet.delete(playerId);
        }
      }
      this.visibility.get(playerId)?.clear();
    }

    player.room = msg.room;
    player.x = msg.x;
    player.y = msg.y;
    this.updateVisibilityOnMove(playerId);
  }

  private updateVisibilityOnMove(moverId: number): void {
    const mover = this.players.get(moverId);
    if (!mover) return;
    const moverVisSet = this.visibility.get(moverId);
    if (!moverVisSet) return;

    for (const other of this.players.values()) {
      if (other.id === moverId) continue;

      if (other.room !== mover.room) {
        // Different room: no position reveal
        continue;
      }

      // Same room: check directional LOS independently for each perspective
      const room = this.world.rooms[mover.room];
      if (!room) continue;

      const otherVisSet = this.visibility.get(other.id);
      const wasMoverSeeOther = moverVisSet.has(other.id);
      const wasOtherSeeMover = otherVisSet?.has(moverId) ?? false;

      // Can mover see other? (mover's tile excluded, other's tile included)
      const nowMoverSeeOther = spotIsVisible(
        room,
        this.world.objects,
        mover.x,
        mover.y,
        other.x,
        other.y,
      );
      // Can other see mover? (other's tile excluded, mover's tile included)
      const nowOtherSeeMover = spotIsVisible(
        room,
        this.world.objects,
        other.x,
        other.y,
        mover.x,
        mover.y,
      );

      // Update visibility sets
      if (nowMoverSeeOther) moverVisSet.add(other.id);
      else moverVisSet.delete(other.id);
      if (otherVisSet) {
        if (nowOtherSeeMover) otherVisSet.add(moverId);
        else otherVisSet.delete(moverId);
      }

      // What other can see about mover
      if (nowOtherSeeMover && !wasOtherSeeMover) {
        this.send(other.ws, this.makePlayerInfo(mover));
        this.send(other.ws, {
          type: 'PLAYER_HEALTH',
          id: moverId,
          hp: mover.hp,
          maxHp: mover.maxHp,
        });
      } else if (!nowOtherSeeMover && wasOtherSeeMover) {
        this.send(other.ws, { type: 'PLAYER_HIDDEN', id: moverId });
      } else if (nowOtherSeeMover) {
        // Still visible: send position update
        this.send(other.ws, {
          type: 'MY_LOCATION',
          id: moverId,
          room: mover.room,
          x: mover.x,
          y: mover.y,
        });
      }

      // What mover can see about other (other didn't move, but mover's vantage changed)
      if (nowMoverSeeOther && !wasMoverSeeOther) {
        this.send(mover.ws, this.makePlayerInfo(other));
        this.send(mover.ws, {
          type: 'PLAYER_HEALTH',
          id: other.id,
          hp: other.hp,
          maxHp: other.maxHp,
        });
      } else if (!nowMoverSeeOther && wasMoverSeeOther) {
        this.send(mover.ws, { type: 'PLAYER_HIDDEN', id: other.id });
      }
      // If still mutually visible or still mutually hidden: nothing extra for mover's view of other
    }
  }

  private onMessage(playerId: number, msg: Extract<C2SMessage, { type: 'MESSAGE' }>): void {
    const sender = this.players.get(playerId);
    if (!sender) return;
    const { filtered, triggered } = filterText(msg.text);
    const s2c: S2CMessage = {
      type: 'MESSAGE',
      from: playerId,
      name: sender.name,
      to: msg.to,
      text: filtered,
    };
    if (msg.to === 'all') {
      this.chatHistory.push({ from: playerId, name: sender.name, text: filtered });
      if (this.chatHistory.length > 100) this.chatHistory.shift();
      this.broadcast(s2c);
      if (triggered) {
        this.broadcastGM(randomScold());
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
    if (player.dead) return;

    const roomMap = this.roomItems.get(player.room);
    const key = `${msg.x},${msg.y}`;
    const item = roomMap?.get(key);
    if (!item) return;

    // Block pickup if another player is standing on that tile
    for (const other of this.players.values()) {
      if (
        other.id !== playerId &&
        other.room === player.room &&
        other.x === msg.x &&
        other.y === msg.y
      )
        return;
    }

    const room = this.world.rooms[player.room];

    // Proximity: Chebyshev distance must be within PICKUP_RANGE
    if (Math.max(Math.abs(msg.x - player.x), Math.abs(msg.y - player.y)) > PICKUP_RANGE) return;

    // Visibility: LOS must not be blocked
    if (!spotIsVisible(room, this.world.objects, player.x, player.y, msg.x, msg.y)) return;

    // Walkability: every tile along the Chebyshev path (including destination)
    // must be walkable — catches transparent-but-unwalkable tiles like windows.
    for (const { x, y } of chebyshevPath(player.x, player.y, msg.x, msg.y)) {
      const cell = room.spot?.[x]?.[y];
      if (cell) {
        const [flId, wlId] = cell;
        if (flId || wlId) {
          const wallObj = wlId > 0 ? this.world.objects[wlId] : null;
          const floorObj = flId > 0 ? this.world.objects[flId] : null;
          if (wallObj && !wallObj.movement) return;
          if (floorObj && !floorObj.movement) return;
        }
      }
      for (const ro of room.recorded_objects) {
        if (ro.x === x && ro.y === y && ro.type > 0) {
          const roObj = this.world.objects[ro.type];
          if (roObj?.takeable) continue; // floor items don't block movement
          if (roObj && !roObj.movement) return;
        }
      }
    }

    const obj = this.world.objects[item.type];
    const itemWeight = calcItemWeight(obj, item);

    if (player.currentWeight + itemWeight > MAX_WEIGHT) {
      this.send(player.ws, {
        type: 'MESSAGE',
        from: 0,
        name: 'GM',
        to: player.id,
        text: 'That is too heavy to carry.',
      });
      return;
    }

    const handOccupied = player.leftHand !== null;

    if (!handOccupied) {
      player.leftHand = item;
    } else {
      const freeSlot = player.inventory.indexOf(null);
      if (freeSlot === -1) {
        this.send(player.ws, {
          type: 'MESSAGE',
          from: 0,
          name: 'GM',
          to: player.id,
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
    if (player.dead) return;

    let item: InventoryItem | null = null;
    if (msg.source === 'active') {
      item = player.leftHand;
      player.leftHand = null;
    } else if (typeof msg.source === 'number' && msg.source >= 0 && msg.source < INV_SIZE) {
      item = player.inventory[msg.source];
      player.inventory[msg.source] = null;
    }

    if (!item) return;

    const obj = this.world.objects[item.type];
    const itemWeight = calcItemWeight(obj, item);
    player.currentWeight = Math.max(0, player.currentWeight - itemWeight);

    const tile = this.nearbyFreeTile(player.room, player.x, player.y);
    if (tile) {
      const roomMap = this.roomItems.get(player.room) ?? new Map<string, InventoryItem>();
      roomMap.set(`${tile.x},${tile.y}`, item);
      this.roomItems.set(player.room, roomMap);
      this.broadcast({ type: 'ITEM_ADDED', room: player.room, x: tile.x, y: tile.y, item });
    } else {
      console.log(
        `[inv] item type=${item.type} lost — no free tile near (${player.x},${player.y}) in room ${player.room}`,
      );
    }

    this.sendInventory(player);
  }

  private onInvSwap(playerId: number, msg: Extract<C2SMessage, { type: 'INV_SWAP' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    if (player.dead) return;
    if (msg.slot < 0 || msg.slot >= INV_SIZE) return;

    const slotItem = player.inventory[msg.slot];
    player.inventory[msg.slot] = player.leftHand;
    player.leftHand = slotItem;

    this.sendInventory(player);
  }

  // ── Combat ────────────────────────────────────────────────────────────────

  /** Auto-reload weapon from compatible ammo in inventory. */
  private tryReloadFromInventory(player: Player): void {
    const weaponItem = player.leftHand;
    if (!weaponItem) return;
    const weaponObj = this.world.objects[weaponItem.type];
    if (!weaponObj?.numbered || !weaponObj.type) return;

    const capacity = weaponObj.capacity ?? Infinity;
    if (weaponItem.quantity >= capacity) return;

    for (let i = 0; i < player.inventory.length; i++) {
      const ammoItem = player.inventory[i];
      if (!ammoItem) continue;
      const ammoObj = this.world.objects[ammoItem.type];
      if (!ammoObj?.charges) continue;
      if ((ammoObj.charges & weaponObj.type) === 0) continue;

      const needed = capacity - weaponItem.quantity;
      const transfer = Math.min(ammoItem.quantity, needed);
      if (transfer <= 0) continue;

      weaponItem.quantity += transfer;

      if (!ammoObj.numbered) {
        player.currentWeight = Math.max(0, player.currentWeight - transfer * (ammoObj.weight ?? 0));
      }
      ammoItem.quantity -= transfer;

      if (ammoItem.quantity <= 0) {
        if (ammoObj.numbered) {
          player.currentWeight = Math.max(0, player.currentWeight - (ammoObj.weight ?? 0));
        }
        player.inventory[i] = null;
      }

      if (weaponItem.quantity >= capacity) break;
    }
  }

  /** Scan path for the first player occupying a tile. Pass excludeId to skip a player (e.g. the shooter). */
  private findPlayerHitOnPath(
    path: Array<{ x: number; y: number }>,
    roomIdx: number,
    excludeId?: number,
  ): { player: Player; hitAtStep: number } | null {
    for (let i = 0; i < path.length; i++) {
      const { x, y } = path[i];
      for (const p of this.players.values()) {
        if (p.room !== roomIdx) continue;
        if (excludeId !== undefined && p.id === excludeId) continue;
        if (p.x === x && p.y === y) return { player: p, hitAtStep: i + 1 };
      }
    }
    return null;
  }

  private calcMissilePath(
    room: RoomData,
    x0: number,
    y0: number,
    x1: number,
    y1: number,
    range: number,
    piercing: boolean,
  ): Array<{ x: number; y: number }> {
    const path: Array<{ x: number; y: number }> = [];
    const adx = Math.abs(x1 - x0),
      ady = Math.abs(y1 - y0);
    const sx = x0 < x1 ? 1 : -1,
      sy = y0 < y1 ? 1 : -1;
    let err = adx - ady;
    let cx = x0,
      cy = y0;
    while (path.length < range) {
      const e2 = 2 * err;
      if (e2 > -ady) {
        err -= ady;
        cx += sx;
      }
      if (e2 < adx) {
        err += adx;
        cy += sy;
      }
      if (cx < 0 || cx >= GRID || cy < 0 || cy >= GRID) break;
      const cell = room.spot?.[cx]?.[cy];
      if (cell) {
        const [flId, wlId] = cell;
        const wallObj = wlId > 0 ? this.world.objects[wlId] : null;
        const floorObj = flId > 0 ? this.world.objects[flId] : null;
        if (!piercing && wallObj && !wallObj.permeable) break;
        if (!piercing && floorObj && !floorObj.permeable) break;
      }
      path.push({ x: cx, y: cy });
      if (cx === x1 && cy === y1) break;
    }
    return path;
  }

  private getRoomExit(roomIdx: number, dx: number, dy: number): number {
    const room = this.world.rooms[roomIdx];
    if (!room) return -1;
    if (dy === -1 && dx === 0) return room.exitNorth;
    if (dx === 1 && dy === 0) return room.exitEast;
    if (dy === 1 && dx === 0) return room.exitSouth;
    if (dx === -1 && dy === 0) return room.exitWest;
    return -1; // diagonals and zero-vector: no cross-room
  }

  private triggerExplosion(
    attacker: Player,
    roomIdx: number,
    landX: number,
    landY: number,
    boomObjType: number,
    radius: number,
    piercing: boolean,
  ): void {
    const boomObj = this.world.objects[boomObjType];
    if (!boomObj) return;

    const msPerStep = calcMsPerStep(boomObj.speed ?? 5);
    const damage = boomObj.damage ?? 10;
    const roomData = this.world.rooms[roomIdx];
    if (!roomData) return;

    // Direct hit: damage any player standing exactly on the landing tile
    const centerHit = this.findPlayerHitOnPath([{ x: landX, y: landY }], roomIdx);
    if (centerHit) this.dealDamage(centerHit.player, damage, attacker);

    for (const { dx, dy } of EXPLOSION_DIRS) {
      const targetX = Math.max(0, Math.min(GRID - 1, landX + dx * radius));
      const targetY = Math.max(0, Math.min(GRID - 1, landY + dy * radius));
      const path = this.calcMissilePath(roomData, landX, landY, targetX, targetY, radius, piercing);
      if (path.length === 0) continue;

      // Find first player hit along this ray (attacker included — self-damage allowed)
      const hit = this.findPlayerHitOnPath(path, roomIdx);
      const hitPlayer = hit?.player ?? null;
      const finalPath = path.slice(0, hit?.hitAtStep ?? path.length);
      const id = this.nextMissileId++;
      this.broadcastToRoom(roomIdx, {
        type: 'MISSILE_START',
        id,
        room: roomIdx,
        path: finalPath,
        objType: boomObjType,
        msPerStep,
        dx,
        dy,
      });

      const timer = setTimeout(() => {
        this.activeMissiles.delete(id);
        this.broadcastToRoom(roomIdx, { type: 'MISSILE_END', id });
        if (hitPlayer) this.dealDamage(hitPlayer, damage, attacker);
      }, finalPath.length * msPerStep);
      this.activeMissiles.set(id, timer);
    }
  }

  private autoReloadHand(player: Player, itemType: number): void {
    const reloadSlot = player.inventory.findIndex(
      (item) => item !== null && item.type === itemType,
    );
    if (reloadSlot !== -1) {
      player.leftHand = player.inventory[reloadSlot];
      player.inventory[reloadSlot] = null;
    }
  }

  private onFireWeapon(playerId: number, msg: Extract<C2SMessage, { type: 'FIRE_WEAPON' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    if (player.dead) return;

    const handItem = player.leftHand;
    if (!handItem) {
      this.onPunch(player, msg);
      return;
    }

    const obj = this.world.objects[handItem.type];
    if (!obj?.weapon) return;

    // For numbered items (guns, staves), require charges; if empty try to reload first
    if (obj.numbered && handItem.quantity <= 0) {
      this.tryReloadFromInventory(player);
      if (handItem.quantity <= 0) {
        this.sendInventory(player); // update UI even if shot fails
        return;
      }
    }

    // Enforce fire rate cooldown.
    // Pipeline emits refire as an unsigned byte; values > 127 are negative in the
    // original C binary (e.g. 255 → -1, 253 → -3). Sign-extend before use.
    const refireRaw = obj.refire ?? 0;
    const refire = refireRaw > 127 ? refireRaw - 256 : refireRaw;
    const cooldown = calcFireCooldown(refire);
    if (Date.now() - player.lastFireTime < cooldown) return;

    // Damage may live on the bullet/projectile object rather than the weapon itself
    const bulletObj = obj.movingobj ? this.world.objects[obj.movingobj] : null;
    const damage = obj.damage ?? bulletObj?.damage ?? 10;
    const range = obj.range ?? 5;
    const movingObjType = obj.movingobj ?? handItem.type;

    // Decrement ammo/charges for numbered weapons; consume lost (single-use) weapons
    if (obj.numbered) {
      handItem.quantity--;
      if (handItem.quantity <= 0) {
        this.tryReloadFromInventory(player);
      }
      this.sendInventory(player);
    } else if (obj.lost) {
      player.currentWeight = Math.max(0, player.currentWeight - calcItemWeight(obj, handItem));
      player.leftHand = null;
      // Auto-reload: pull matching item from inventory into the now-empty hand
      this.autoReloadHand(player, handItem.type);
      this.sendInventory(player);
    }

    // Record fire time now (committed to firing)
    player.lastFireTime = Date.now();

    // Compute unit direction toward target
    const rawDx = msg.targetX - player.x;
    const rawDy = msg.targetY - player.y;
    if (rawDx === 0 && rawDy === 0) return;
    const dx = Math.sign(rawDx);
    const dy = Math.sign(rawDy);

    const room = this.world.rooms[player.room];
    if (!room) return;

    const path = this.calcMissilePath(
      room,
      player.x,
      player.y,
      msg.targetX,
      msg.targetY,
      range,
      false,
    );

    // Find first player hit along path (excluding the shooter)
    const hit = this.findPlayerHitOnPath(path, player.room, playerId);
    const hitPlayer = hit?.player ?? null;
    const finalPath = path.slice(0, hit?.hitAtStep ?? path.length);
    if (finalPath.length === 0) return;

    const id = this.nextMissileId++;
    const speed = bulletObj?.speed ?? obj.speed ?? 5;
    const msPerStep = calcMsPerStep(speed);

    this.broadcastToRoom(player.room, {
      type: 'MISSILE_START',
      id,
      room: player.room,
      path: finalPath,
      objType: movingObjType,
      msPerStep,
      dx,
      dy,
    });

    // Apply damage when missile arrives; also signal clients to clear the sprite
    const travelMs = finalPath.length * msPerStep;
    const timer = setTimeout(() => {
      this.activeMissiles.delete(id);
      this.broadcastToRoom(player.room, { type: 'MISSILE_END', id });
      if (hitPlayer) this.dealDamage(hitPlayer, damage, player);
      const landTile = finalPath[finalPath.length - 1];
      // Trigger explosion for exploding weapons
      if (obj.explodes) {
        const boomObjType = obj.boombit ?? obj.movingobj ?? handItem.type;
        const radius = Math.max(1, obj.explodes - 1);
        const boomObj = this.world.objects[boomObjType];
        const piercingFlag = (boomObj?.piercing ?? 0) > 0;

        if (obj.lost && !hitPlayer && finalPath.length < range) {
          const onEdge =
            (dy === -1 && landTile.y === 0) ||
            (dx === 1 && landTile.x === GRID - 1) ||
            (dy === 1 && landTile.y === GRID - 1) ||
            (dx === -1 && landTile.x === 0);
          const nextRoomIdx = onEdge ? this.getRoomExit(player.room, dx, dy) : -1;
          const nextRoom = nextRoomIdx >= 0 ? this.world.rooms[nextRoomIdx] : null;

          if (nextRoom) {
            const entryX = dy !== 0 ? landTile.x : dx === 1 ? 0 : GRID - 1;
            const entryY = dx !== 0 ? landTile.y : dy === 1 ? 0 : GRID - 1;
            const remainingRange = range - finalPath.length;

            // calcMissilePath never includes the start tile, so prepend the entry tile
            // so the grenade visually appears on the first square in the next room.
            const contPath =
              remainingRange > 1
                ? this.calcMissilePath(
                    nextRoom,
                    entryX,
                    entryY,
                    entryX + dx * (remainingRange - 1),
                    entryY + dy * (remainingRange - 1),
                    remainingRange - 1,
                    piercingFlag,
                  )
                : [];

            const contId = this.nextMissileId++;
            const contTilePath = [{ x: entryX, y: entryY }, ...contPath];

            this.broadcastToRoom(nextRoomIdx, {
              type: 'MISSILE_START',
              id: contId,
              room: nextRoomIdx,
              path: contTilePath,
              objType: movingObjType,
              msPerStep,
              dx,
              dy,
            });

            const contTravelMs = contTilePath.length * msPerStep;
            const landInNext = contTilePath[contTilePath.length - 1];

            const contTimer = setTimeout(() => {
              this.activeMissiles.delete(contId);
              this.broadcastToRoom(nextRoomIdx, { type: 'MISSILE_END', id: contId });
              this.triggerExplosion(
                player,
                nextRoomIdx,
                landInNext.x,
                landInNext.y,
                boomObjType,
                radius,
                piercingFlag,
              );
            }, contTravelMs);
            this.activeMissiles.set(contId, contTimer);
            return; // grenade continues in next room; skip in-room explosion
          }
        }

        this.triggerExplosion(
          player,
          player.room,
          landTile.x,
          landTile.y,
          boomObjType,
          radius,
          piercingFlag,
        );
      }
      // Drop throwable items (lost+stop, non-exploding) at landing position
      if (obj.lost && obj.stop && !obj.explodes) {
        const tile = this.nearbyFreeTile(player.room, landTile.x, landTile.y);
        if (tile) {
          const roomMap = this.roomItems.get(player.room) ?? new Map<string, InventoryItem>();
          roomMap.set(`${tile.x},${tile.y}`, handItem);
          this.roomItems.set(player.room, roomMap);
          this.broadcast({
            type: 'ITEM_ADDED',
            room: player.room,
            x: tile.x,
            y: tile.y,
            item: handItem,
          });
        }
      }
    }, travelMs);
    this.activeMissiles.set(id, timer);
  }

  private onPunch(player: Player, msg: Extract<C2SMessage, { type: 'FIRE_WEAPON' }>): void {
    if (Date.now() - player.lastPunchedAt < PUNCH_COOLDOWN_MS) return;

    const dx = Math.sign(msg.targetX - player.x);
    const dy = Math.sign(msg.targetY - player.y);
    if (dx === 0 && dy === 0) return;

    const targetX = player.x + dx;
    const targetY = player.y + dy;

    player.lastPunchedAt = Date.now();

    if (targetX < 0 || targetX >= GRID || targetY < 0 || targetY >= GRID) {
      // Punch crosses into an adjacent room
      const nextRoomIdx = this.getRoomExit(player.room, dx, dy);
      if (nextRoomIdx < 0) return;
      const entryX = targetX < 0 ? GRID - 1 : targetX >= GRID ? 0 : targetX;
      const entryY = targetY < 0 ? GRID - 1 : targetY >= GRID ? 0 : targetY;
      this.broadcastToRoom(nextRoomIdx, {
        type: 'PUNCH',
        room: nextRoomIdx,
        x: entryX,
        y: entryY,
        dx,
        dy,
      });
      const hit = this.findPlayerHitOnPath([{ x: entryX, y: entryY }], nextRoomIdx, player.id);
      if (hit) this.dealDamage(hit.player, PUNCH_DAMAGE, player);
      return;
    }

    this.broadcastToRoom(player.room, {
      type: 'PUNCH',
      room: player.room,
      x: targetX,
      y: targetY,
      dx,
      dy,
    });

    const hit = this.findPlayerHitOnPath([{ x: targetX, y: targetY }], player.room, player.id);
    if (hit) this.dealDamage(hit.player, PUNCH_DAMAGE, player);
  }

  private onUseItem(playerId: number, msg: Extract<C2SMessage, { type: 'USE_ITEM' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    if (player.dead) return;

    const handItem = player.leftHand;
    if (!handItem) return;

    const obj = this.world.objects[handItem.type];
    if (!obj) return;

    // --- Consumable branch (same-tile use) ---
    if ((obj.health ?? 0) < 0) {
      // Block if HP already full
      if (player.hp >= player.maxHp) return;

      const healAmount = Math.min(player.maxHp - player.hp, -(obj.health ?? 0));
      player.hp = Math.min(player.maxHp, player.hp + healAmount);

      // Consume the item: decrement numbered items, remove lost items entirely
      let handEmptied = false;
      if (obj.numbered) {
        handItem.quantity--;
        if (handItem.quantity <= 0) {
          player.currentWeight = Math.max(0, player.currentWeight - calcItemWeight(obj, handItem));
          player.leftHand = null;
          handEmptied = true;
        }
      } else if (obj.lost) {
        player.currentWeight = Math.max(0, player.currentWeight - calcItemWeight(obj, handItem));
        player.leftHand = null;
        handEmptied = true;
      }

      // Auto-reload: if hand is now empty, move first matching item from inventory
      if (handEmptied) {
        this.autoReloadHand(player, handItem.type);
      }

      this.sendInventory(player);
      this.sendStats(player);
      this.broadcast({ type: 'PLAYER_HEALTH', id: player.id, hp: player.hp, maxHp: player.maxHp });
      this.broadcastToRoom(player.room, {
        type: 'PLAYER_HEAL',
        playerId: player.id,
        room: player.room,
        x: player.x,
        y: player.y,
        amount: healAmount,
      });
      console.log(`[use] ${player.name} consumed ${obj.name ?? '?'} (+${healAmount} HP)`);
      return;
    }

    // --- Opener branch (adjacent-tile use) ---
    if (!obj.opens) return;

    // Must be adjacent (Chebyshev distance = 1) — not on the same tile
    const adx = Math.abs(msg.targetX - player.x);
    const ady = Math.abs(msg.targetY - player.y);
    if (adx === 0 && ady === 0) return;
    if (adx > 1 || ady > 1) return;

    const room = this.world.rooms[player.room];
    if (!room) return;

    // Find swinging objects at the target tile and toggle them
    let toggled = false;
    for (const ro of room.recorded_objects) {
      if (ro.x !== msg.targetX || ro.y !== msg.targetY) continue;
      const doorDef = this.world.objects[ro.type];
      if (!doorDef?.swings || !doorDef.alternate) continue;

      // Type matching: skip if either side is 0 (universal), otherwise must share a bit
      if (obj.opens && doorDef.type && !(obj.opens & doorDef.type)) continue;

      ro.type = doorDef.alternate;
      this.broadcastToRoom(player.room, {
        type: 'ROOM_OBJECT_CHANGED',
        room: player.room,
        x: msg.targetX,
        y: msg.targetY,
        newType: ro.type,
      });
      toggled = true;
    }

    if (toggled && obj.numbered) {
      handItem.quantity--;
      if (handItem.quantity <= 0) {
        player.leftHand = null;
      }
      this.sendInventory(player);
    }

    if (toggled) {
      console.log(
        `[use] ${player.name} used ${obj.name ?? '?'} on (${msg.targetX},${msg.targetY})`,
      );
    }
  }

  private dealDamage(victim: Player, damage: number, attacker: Player | null): void {
    if (victim.dead) return;
    victim.hp = Math.max(0, victim.hp - damage);

    this.broadcast({ type: 'PLAYER_HEALTH', id: victim.id, hp: victim.hp, maxHp: victim.maxHp });
    this.broadcastToRoom(victim.room, {
      type: 'PLAYER_HIT',
      victimId: victim.id,
      room: victim.room,
      x: victim.x,
      y: victim.y,
      damage,
    });

    if (victim.hp <= 0) {
      this.killPlayer(victim, attacker);
    }
  }

  private killPlayer(victim: Player, killer: Player | null): void {
    console.log(
      `[combat] ${victim.name} killed by ${killer?.name ?? 'void'} at room=${victim.room} (${victim.x},${victim.y})`,
    );
    victim.deaths++;
    this.broadcast({
      type: 'PLAYER_STATS',
      id: victim.id,
      kills: victim.kills,
      deaths: victim.deaths,
    });

    if (killer) {
      killer.kills++;
      this.send(killer.ws, { type: 'REPORT', text: `You killed ${victim.name}!` });
      this.broadcast({
        type: 'PLAYER_STATS',
        id: killer.id,
        kills: killer.kills,
        deaths: killer.deaths,
      });
      this.sendStats(killer);
    }

    // Announce death in global chat
    const killerDesc = killer ? killer.name : 'the void';
    this.broadcastGM(`${victim.name} was slain by ${killerDesc}.`);

    // Drop all inventory items and notify victim their inventory is now empty
    this.dropPlayerItems(victim);
    this.sendInventory(victim);

    // Schedule respawn after tombstone delay
    this.scheduleRespawn(victim, killer);
  }

  private dropPlayerItems(player: Player): void {
    const items: Array<InventoryItem | null> = [player.leftHand, ...player.inventory];
    player.leftHand = null;
    player.inventory.fill(null);
    player.currentWeight = 0;

    // Build occupied set once; re-use for each drop in this batch
    const playerOccupied = new Set<string>();
    for (const p of this.players.values()) {
      if (p.room === player.room) playerOccupied.add(`${p.x},${p.y}`);
    }

    for (const item of items) {
      if (!item) continue;
      const tile = this.nearbyFreeTile(player.room, player.x, player.y, playerOccupied);
      if (tile) {
        const roomMap = this.roomItems.get(player.room) ?? new Map<string, InventoryItem>();
        roomMap.set(`${tile.x},${tile.y}`, item);
        this.roomItems.set(player.room, roomMap);
        this.broadcast({ type: 'ITEM_ADDED', room: player.room, x: tile.x, y: tile.y, item });
      }
    }
  }

  // Return a random walkable, unoccupied tile in any room belonging to the
  // given team (matching the original game's select_person_place logic).
  // Falls back to any room if no team room found.
  private randomSpawnForTeam(team: number): { room: number; x: number; y: number } | null {
    // Collect candidate room indices for this team, then fall back to all rooms
    const pickRooms = (t: number) =>
      this.world.rooms
        .map((r, i) => ({ r, i }))
        .filter(({ r }) => t === -1 || r.team === t)
        .map(({ i }) => i);

    let candidates = pickRooms(team);
    if (candidates.length === 0) candidates = pickRooms(-1);
    if (candidates.length === 0) return null;

    // Shuffle candidates so we try rooms in random order
    for (let i = candidates.length - 1; i > 0; i--) {
      const j = Math.floor(Math.random() * (i + 1));
      [candidates[i], candidates[j]] = [candidates[j], candidates[i]];
    }

    for (const roomIdx of candidates) {
      const spot = this.randomWalkableTile(roomIdx);
      if (spot) return { room: roomIdx, ...spot };
    }
    console.warn(
      `[respawn] team=${team} no walkable tile found in any of [${candidates.join(',')}]`,
    );
    return null;
  }

  // Pick a random walkable, unoccupied tile in a room.
  private randomWalkableTile(roomIdx: number): { x: number; y: number } | null {
    const room = this.world.rooms[roomIdx];
    if (!room?.spot) return null;

    const playerOccupied = new Set<string>();
    for (const p of this.players.values()) {
      if (p.room === roomIdx) playerOccupied.add(`${p.x},${p.y}`);
    }

    // Build a set of tiles blocked by recorded_objects (e.g. closed doors, walls).
    const roBlocked = new Set<string>();
    for (const ro of room.recorded_objects) {
      const obj = ro.type > 0 ? this.world.objects[ro.type] : null;
      if (obj && !obj.movement) roBlocked.add(`${ro.x},${ro.y}`);
    }

    // Collect all walkable, unoccupied tiles
    const walkable: Array<{ x: number; y: number }> = [];
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        if (playerOccupied.has(`${x},${y}`)) continue;
        if (roBlocked.has(`${x},${y}`)) continue;
        const cell = room.spot[x]?.[y];
        if (!cell) continue;
        const [flId, wlId] = cell;
        // Void tile [0,0]: walkable but not a valid respawn point when the room
        // has a defined floor (it may be outside the walls).
        if (!flId && !wlId) {
          if (!room.floor) walkable.push({ x, y });
          // When room.floor !== 0, skip void tiles for respawn (but movement is allowed).
          continue;
        }
        const wallObj = wlId > 0 ? this.world.objects[wlId] : null;
        const floorObj = flId > 0 ? this.world.objects[flId] : null;
        // Non-void: walkable only if objects allow movement (absent = blocked)
        if (wallObj && !wallObj.movement) continue;
        if (floorObj && !floorObj.movement) continue;
        walkable.push({ x, y });
      }
    }
    if (walkable.length === 0) return null;
    return walkable[Math.floor(Math.random() * walkable.length)];
  }

  private scheduleRespawn(victim: Player, killer: Player | null): void {
    victim.dead = true;
    const killerName = killer?.name ?? 'the void'; // capture now; killer may disconnect before timer fires

    // Broadcast tombstone state at death location, respecting LOS
    console.log(
      `[respawn] ${victim.name} is dead; broadcasting tombstone at room=${victim.room} (${victim.x},${victim.y})`,
    );
    const tombstoneInfo = this.makePlayerInfo(victim);
    const victimVisSet = this.visibility.get(victim.id);
    for (const other of this.players.values()) {
      if (other.id === victim.id) continue;
      if (other.room !== victim.room || victimVisSet?.has(other.id)) {
        this.send(other.ws, tombstoneInfo);
      }
    }

    console.log(
      `[respawn] sending YOU_DIED to ${victim.name} (ws readyState=${victim.ws.readyState}): deadForMs=${RESPAWN_DELAY_MS}`,
    );
    this.send(victim.ws, {
      type: 'YOU_DIED',
      killedBy: killer?.id ?? 0,
      killerName,
      deadForMs: RESPAWN_DELAY_MS,
    });

    victim.respawnTimer = setTimeout(() => {
      this.doRespawn(victim, killerName);
    }, RESPAWN_DELAY_MS);
  }

  private doRespawn(victim: Player, killerName: string): void {
    victim.respawnTimer = null;
    victim.hp = victim.maxHp; // restore HP at respawn, not at death
    victim.dead = false;

    this.placePlayer(victim, 'respawn');

    this.broadcast({ type: 'PLAYER_HEALTH', id: victim.id, hp: victim.hp, maxHp: victim.maxHp });
    this.sendStats(victim);
    this.sendInventory(victim);

    console.log(
      `[respawn] ${victim.name} complete at room=${victim.room} (${victim.x},${victim.y}) (killed by ${killerName})`,
    );
  }

  // ── Leave ─────────────────────────────────────────────────────────────────

  private onLeave(playerId: number): void {
    const player = this.players.get(playerId);
    if (!player) return;

    if (player.respawnTimer !== null) {
      clearTimeout(player.respawnTimer);
      player.respawnTimer = null;
    }
    this.clearAfkTimers(player);

    this.dropPlayerItems(player);

    this.players.delete(playerId);
    this.wsToId.delete(player.ws);
    this.clearVisibility(playerId);
    this.broadcast({ type: 'LEAVING_GAME', id: playerId });
    if (this.players.size === 0) {
      if (this.world.resetOnEmpty) {
        const delay = this.world.resetAfterSeconds * 1000;
        console.log(
          `[reset] scheduled in ${this.world.resetAfterSeconds}s (${this.world.mapName} is empty)`,
        );
        this.resetTimer = setTimeout(() => {
          this.resetTimer = null;
          this.resetWorldState();
          console.log(`[reset] map state reset (${this.world.mapName})`);
        }, delay);
      } else {
        this.chatHistory = [];
        console.log('[chat] history cleared (server empty)');
      }
    }
    this.onPlayerCountChange?.();
    console.log(`[-] ${player.name} (id=${playerId}) left. Players: ${this.players.size}`);
  }

  private onVoluntaryRespawn(playerId: number): void {
    const player = this.players.get(playerId);
    if (!player || player.dead) return;

    this.dropPlayerItems(player);
    this.placePlayer(player, 'voluntary-respawn');
    this.broadcast({ type: 'PLAYER_HEALTH', id: player.id, hp: player.hp, maxHp: player.maxHp });
    this.broadcastGM(`${player.name} chose to respawn.`);
    this.sendInventory(player);
  }

  // ── Helpers ───────────────────────────────────────────────────────────────

  private startAfkTimer(player: Player): void {
    this.clearAfkTimers(player);
    player.afkIdleTimer = setTimeout(() => {
      player.afkIdleTimer = null;
      player.afkWarningsLeft = AFK_GRACE_MINUTES;
      this.sendAfkWarning(player);
    }, AFK_IDLE_MS);
  }

  private clearAfkTimers(player: Player): void {
    if (player.afkIdleTimer !== null) {
      clearTimeout(player.afkIdleTimer);
      player.afkIdleTimer = null;
    }
    if (player.afkWarnTimer !== null) {
      clearTimeout(player.afkWarnTimer);
      player.afkWarnTimer = null;
    }
  }

  private sendAfkWarning(player: Player): void {
    const mins = player.afkWarningsLeft;
    this.send(player.ws, {
      type: 'MESSAGE',
      from: 0,
      name: 'GM',
      to: player.id,
      text: `You'll be kicked from the game in another ${mins} minute${mins === 1 ? '' : 's'} if you are still inactive.`,
    });
    player.afkWarningsLeft--;
    if (player.afkWarningsLeft <= 0) {
      // Grace period exhausted — kick the player
      player.afkWarnTimer = setTimeout(() => {
        player.afkWarnTimer = null;
        this.onLeave(player.id);
        try {
          player.ws.close();
        } catch {
          /* already closed */
        }
      }, AFK_WARN_INTERVAL_MS);
    } else {
      player.afkWarnTimer = setTimeout(() => {
        player.afkWarnTimer = null;
        this.sendAfkWarning(player);
      }, AFK_WARN_INTERVAL_MS);
    }
  }

  private resetAfkTimer(player: Player): void {
    const wasWarning = player.afkWarningsLeft > 0 || player.afkWarnTimer !== null;
    this.startAfkTimer(player);
    if (wasWarning) {
      player.afkWarningsLeft = 0;
      this.send(player.ws, {
        type: 'MESSAGE',
        from: 0,
        name: 'GM',
        to: player.id,
        text: 'Welcome back, I see you are still active!',
      });
    }
  }

  private broadcastGM(text: string): void {
    this.broadcast({ type: 'MESSAGE', from: 0, name: 'GM', to: 'all', text });
    this.chatHistory.push({ from: 0, name: 'GM', text });
    if (this.chatHistory.length > 100) this.chatHistory.shift();
  }

  // Move a player to a new spawn point and notify everyone.
  private placePlayer(player: Player, context: string): void {
    const spawn = this.randomSpawnForTeam(player.team);
    if (spawn) {
      console.log(`[${context}] ${player.name} spawn: room=${spawn.room} (${spawn.x},${spawn.y})`);
      player.room = spawn.room;
      player.x = spawn.x;
      player.y = spawn.y;
    } else {
      console.warn(`[${context}] ${player.name} team=${player.team} no spawn — staying in place`);
    }
    this.recomputeVisibilityAfterTeleport(player);
    this.send(player.ws, { type: 'YOU_RESPAWNED', room: player.room, x: player.x, y: player.y });
  }

  /**
   * Called after a player teleports (respawn, voluntary respawn).
   * Hides the player from everyone who could see them before, clears their
   * visibility set, then recomputes who can see them at the new position and
   * sends appropriate PLAYER_INFO reveals.
   */
  private recomputeVisibilityAfterTeleport(player: Player): void {
    // Hide from everyone who currently sees this player
    const oldVisSet = this.visibility.get(player.id);
    if (oldVisSet) {
      for (const otherId of oldVisSet) {
        const other = this.players.get(otherId);
        if (other) {
          this.send(other.ws, { type: 'PLAYER_HIDDEN', id: player.id });
          this.visibility.get(otherId)?.delete(player.id);
        }
      }
      oldVisSet.clear();
    }

    const newVisSet = this.visibility.get(player.id) ?? new Set<number>();
    this.visibility.set(player.id, newVisSet);

    // Recompute visibility for all others
    for (const other of this.players.values()) {
      if (other.id === player.id) continue;

      if (other.room !== player.room) {
        // Different room: no position reveal
        continue;
      }

      // Same room: check directional LOS independently for each perspective
      const room = this.world.rooms[player.room];
      if (!room) continue;

      const playerCanSeeOther = spotIsVisible(
        room,
        this.world.objects,
        player.x,
        player.y,
        other.x,
        other.y,
      );
      const otherCanSeePlayer = spotIsVisible(
        room,
        this.world.objects,
        other.x,
        other.y,
        player.x,
        player.y,
      );

      if (playerCanSeeOther) {
        newVisSet.add(other.id);
        this.send(player.ws, this.makePlayerInfo(other));
        this.send(player.ws, {
          type: 'PLAYER_HEALTH',
          id: other.id,
          hp: other.hp,
          maxHp: other.maxHp,
        });
      }
      if (otherCanSeePlayer) {
        this.visibility.get(other.id)?.add(player.id);
        this.send(other.ws, this.makePlayerInfo(player));
        this.send(other.ws, {
          type: 'PLAYER_HEALTH',
          id: player.id,
          hp: player.hp,
          maxHp: player.maxHp,
        });
      }
    }
  }

  private sendInventory(player: Player): void {
    this.send(player.ws, {
      type: 'YOUR_INVENTORY',
      leftHand: player.leftHand,
      inventory: player.inventory,
      currentWeight: player.currentWeight,
      maxWeight: MAX_WEIGHT,
    });
  }

  private sendStats(player: Player): void {
    this.send(player.ws, {
      type: 'YOUR_STATS',
      hp: player.hp,
      maxHp: player.maxHp,
    });
  }

  private nearbyFreeTile(
    roomIdx: number,
    px: number,
    py: number,
    playerOccupied?: Set<string>,
  ): { x: number; y: number } | null {
    const room = this.world.rooms[roomIdx];
    if (!room) return null;
    const roomMap = this.roomItems.get(roomIdx) ?? new Map<string, InventoryItem>();

    // Build player-occupied set if caller didn't supply one
    if (!playerOccupied) {
      playerOccupied = new Set<string>();
      for (const p of this.players.values()) {
        if (p.room === roomIdx) playerOccupied.add(`${p.x},${p.y}`);
      }
    }

    const isValidTile = (tx: number, ty: number): boolean => {
      if (tx < 0 || tx >= GRID || ty < 0 || ty >= GRID) return false;
      if (roomMap.has(`${tx},${ty}`)) return false;
      if (playerOccupied!.has(`${tx},${ty}`)) return false;
      const cell = room.spot?.[tx]?.[ty];
      if (cell) {
        const [flId, wlId] = cell;
        // Void tile: not walkable when room has a floor (ring-style map)
        if (!flId && !wlId) {
          if (room.floor) return false;
        } else {
          const wallObj = wlId > 0 ? this.world.objects[wlId] : null;
          const floorObj = flId > 0 ? this.world.objects[flId] : null;
          if (wallObj && !wallObj.movement) return false;
          if (floorObj && !floorObj.movement) return false;
        }
      }
      return true;
    };

    const hasDoor = (tx: number, ty: number): boolean =>
      room.recorded_objects.some(
        (ro) => ro.x === tx && ro.y === ty && (this.world.objects[ro.type]?.swings ?? false),
      );

    // BFS flood-fill to find all tiles structurally reachable from the player.
    // Uses only spot+recorded_objects walkability (not items or player positions),
    // so doors and walls correctly partition the reachable area.
    const isPassable = (tx: number, ty: number): boolean => {
      if (tx < 0 || tx >= GRID || ty < 0 || ty >= GRID) return false;
      const cell = room.spot?.[tx]?.[ty];
      if (cell) {
        const [flId, wlId] = cell;
        if (!flId && !wlId) {
          if (room.floor) return false;
        } else {
          if (wlId > 0 && !this.world.objects[wlId]?.movement) return false;
          if (flId > 0 && !this.world.objects[flId]?.movement) return false;
        }
      }
      for (const ro of room.recorded_objects) {
        if (ro.x === tx && ro.y === ty && ro.type > 0) {
          const obj = this.world.objects[ro.type];
          if (obj?.takeable) continue;
          if (!obj?.movement) return false;
        }
      }
      return true;
    };

    const reachable = new Set<string>();
    const bfsQueue: Array<{ x: number; y: number }> = [{ x: px, y: py }];
    reachable.add(`${px},${py}`);
    const DIRS: [number, number][] = [
      [0, -1],
      [1, 0],
      [0, 1],
      [-1, 0],
      [1, -1],
      [1, 1],
      [-1, 1],
      [-1, -1],
    ];
    while (bfsQueue.length > 0) {
      const { x, y } = bfsQueue.shift()!;
      for (const [ddx, ddy] of DIRS) {
        const nx = x + ddx;
        const ny = y + ddy;
        const nk = `${nx},${ny}`;
        if (reachable.has(nk)) continue;
        if (!isPassable(nx, ny)) continue;
        reachable.add(nk);
        bfsQueue.push({ x: nx, y: ny });
      }
    }

    // Spiral search: prefer tiles without doors, must be reachable from player
    for (let radius = 0; radius <= 5; radius++) {
      for (let dx = -radius; dx <= radius; dx++) {
        for (let dy = -radius; dy <= radius; dy++) {
          if (Math.abs(dx) !== radius && Math.abs(dy) !== radius) continue;
          const tx = px + dx;
          const ty = py + dy;
          if (isValidTile(tx, ty) && reachable.has(`${tx},${ty}`) && !hasDoor(tx, ty))
            return { x: tx, y: ty };
        }
      }
    }
    // Fallback: accept door tiles if no door-free reachable tile is available
    for (let radius = 0; radius <= 5; radius++) {
      for (let dx = -radius; dx <= radius; dx++) {
        for (let dy = -radius; dy <= radius; dy++) {
          if (Math.abs(dx) !== radius && Math.abs(dy) !== radius) continue;
          const tx = px + dx;
          const ty = py + dy;
          if (isValidTile(tx, ty) && reachable.has(`${tx},${ty}`)) return { x: tx, y: ty };
        }
      }
    }
    return null;
  }

  private makePlayerInfo(p: Player): Extract<S2CMessage, { type: 'PLAYER_INFO' }> {
    return {
      type: 'PLAYER_INFO',
      id: p.id,
      name: p.name,
      avatar: p.avatar,
      room: p.room,
      x: p.x,
      y: p.y,
      kills: p.kills,
      deaths: p.deaths,
      joinedAt: p.joinedAt,
      dead: p.dead,
      team: p.team,
    };
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

  private broadcastToRoom(roomIdx: number, msg: S2CMessage): void {
    for (const player of this.players.values()) {
      if (player.room === roomIdx) this.send(player.ws, msg);
    }
  }
}
