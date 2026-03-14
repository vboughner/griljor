import WebSocket from 'ws';
import { C2SMessage, S2CMessage, InventoryItem } from './protocol';
import { World, ObjDef, RecObj } from './world';
import { filterText, randomScold } from './filter';

const INV_SIZE = 35;
const MAX_WEIGHT = 150;
const GRID = 20;
const RESPAWN_DELAY_MS = 5000;

/**
 * For numbered items (guns, potions with charges), quantity represents the
 * charge count — the item itself is still ONE physical object. Weight is
 * per-item, not per-charge.  For non-numbered stackable items, multiply by
 * the stack size.
 */
export function calcItemWeight(obj: ObjDef | null | undefined, item: InventoryItem): number {
  if (!obj) return 0;
  if (obj.numbered) return obj.weight ?? 0;
  return (obj.weight ?? 0) * item.quantity;
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
  rightHand: InventoryItem | null;
  inventory: Array<InventoryItem | null>;
  currentWeight: number;
  team: number; // 1-based team number (0 = neutral)
  // combat stats
  hp: number;
  maxHp: number;
  dead: boolean;
  respawnTimer: ReturnType<typeof setTimeout> | null;
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

  constructor(world: World, opts?: { onPlayerCountChange?: () => void }) {
    this.onPlayerCountChange = opts?.onPlayerCountChange;
    this.world = world;
    this.originalRecordedObjects = world.rooms.map((r) =>
      r.recorded_objects.map((ro) => ({ ...ro })),
    );
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
  get playerAvatars(): Array<{ avatar: string; name: string }> {
    return [...this.players.values()].map((p) => ({ avatar: p.avatar, name: p.name }));
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
    const team = 1; // default to team 1 until team selection is implemented
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
      rightHand: null,
      inventory: new Array<InventoryItem | null>(INV_SIZE).fill(null),
      currentWeight: 0,
      team,
      hp: 100,
      maxHp: 100,
      dead: false,
      respawnTimer: null,
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
    });

    for (const other of this.players.values()) {
      if (other.id === id) continue;
      this.send(ws, this.makePlayerInfo(other));
      // Also send their current HP so the new player sees health bars
      this.send(ws, { type: 'PLAYER_HEALTH', id: other.id, hp: other.hp, maxHp: other.maxHp });
    }

    // Tell everyone else about the new player
    this.broadcast(this.makePlayerInfo(player), id);

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

    // Send empty inventory and starting stats to new player
    this.sendInventory(player);
    this.sendStats(player);

    console.log(`[+] ${msg.name} (id=${id}) joined. Players: ${this.players.size}`);
  }

  private onLocation(playerId: number, msg: Extract<C2SMessage, { type: 'MY_LOCATION' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    if (player.dead) return;
    player.room = msg.room;
    player.x = msg.x;
    player.y = msg.y;
    this.broadcast({ type: 'MY_LOCATION', id: playerId, room: msg.room, x: msg.x, y: msg.y });
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
      this.broadcast(s2c);
      if (triggered) {
        const gmMsg: S2CMessage = {
          type: 'MESSAGE',
          from: 0,
          name: 'GM',
          to: 'all',
          text: randomScold(),
        };
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
    if (player.dead) return;

    const roomMap = this.roomItems.get(player.room);
    const key = `${msg.x},${msg.y}`;
    const item = roomMap?.get(key);
    if (!item) return;

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

    // Determine target hand slot
    const hand = msg.hand;
    const handOccupied = hand === 'left' ? player.leftHand !== null : player.rightHand !== null;

    if (!handOccupied) {
      if (hand === 'left') player.leftHand = item;
      else player.rightHand = item;
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
    const handItem = msg.hand === 'left' ? player.leftHand : player.rightHand;

    player.inventory[msg.slot] = handItem;
    if (msg.hand === 'left') player.leftHand = slotItem;
    else player.rightHand = slotItem;

    this.sendInventory(player);
  }

  // ── Combat ────────────────────────────────────────────────────────────────

  private onFireWeapon(playerId: number, msg: Extract<C2SMessage, { type: 'FIRE_WEAPON' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    if (player.dead) return;

    const handItem = msg.hand === 'left' ? player.leftHand : player.rightHand;
    if (!handItem) return;

    const obj = this.world.objects[handItem.type];
    if (!obj?.weapon) return;

    // For numbered items (guns, staves), require charges
    if (obj.numbered && handItem.quantity <= 0) return;

    // Damage may live on the bullet/projectile object rather than the weapon itself
    const bulletObj = obj.movingobj ? this.world.objects[obj.movingobj] : null;
    const damage = obj.damage ?? bulletObj?.damage ?? 10;
    const range = obj.range ?? 5;
    const movingObjType = obj.movingobj ?? handItem.type;

    // Decrement ammo/charges for numbered weapons
    if (obj.numbered) {
      handItem.quantity--;
      if (handItem.quantity <= 0) {
        if (msg.hand === 'left') player.leftHand = null;
        else player.rightHand = null;
      }
      this.sendInventory(player);
    }

    // Compute unit direction toward target
    const rawDx = msg.targetX - player.x;
    const rawDy = msg.targetY - player.y;
    if (rawDx === 0 && rawDy === 0) return;
    const dx = Math.sign(rawDx);
    const dy = Math.sign(rawDy);

    const room = this.world.rooms[player.room];
    if (!room) return;

    // Compute Bresenham path from player to target, capped at range
    const path: Array<{ x: number; y: number }> = [];
    {
      const x0 = player.x,
        y0 = player.y;
      const x1 = msg.targetX,
        y1 = msg.targetY;
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
          if (wallObj && !wallObj.permeable) break;
          if (floorObj && !floorObj.permeable) break;
        }
        path.push({ x: cx, y: cy });
        if (cx === x1 && cy === y1) break; // reached target tile
      }
    }

    // Find first player hit along path
    let hitPlayer: Player | null = null;
    let hitAtStep = path.length;
    outer: for (let i = 0; i < path.length; i++) {
      const { x, y } = path[i];
      for (const other of this.players.values()) {
        if (other.id === playerId || other.room !== player.room) continue;
        if (other.x === x && other.y === y) {
          hitPlayer = other;
          hitAtStep = i + 1;
          break outer;
        }
      }
    }

    const finalPath = path.slice(0, hitAtStep);
    if (finalPath.length === 0) return;

    const id = this.nextMissileId++;
    const speed = bulletObj?.speed ?? obj.speed ?? 5;
    // Match original formula: missile_wait = CLICKS_PER_MOVE*5 / speed / MISSILE_SPEED_FACTOR
    // With CLICKS_PER_MOVE=500, MISSILE_SPEED_FACTOR=2.2 → msPerStep = 2500/(speed*2.2)
    const msPerStep = Math.max(50, Math.round(2500 / (speed * 2.2)));

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
    }, travelMs);
    this.activeMissiles.set(id, timer);
  }

  private onUseItem(playerId: number, msg: Extract<C2SMessage, { type: 'USE_ITEM' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;
    if (player.dead) return;

    const handItem = msg.hand === 'left' ? player.leftHand : player.rightHand;
    if (!handItem) return;

    const obj = this.world.objects[handItem.type];
    if (!obj) return;

    // --- Consumable branch (same-tile use) ---
    if ((obj.health ?? 0) < 0) {
      // Block if HP already full
      if (player.hp >= player.maxHp) return;

      player.hp = Math.min(player.maxHp, player.hp - (obj.health ?? 0));

      if (obj.numbered) {
        handItem.quantity--;
        if (handItem.quantity <= 0) {
          if (msg.hand === 'left') player.leftHand = null;
          else player.rightHand = null;
        }
        this.sendInventory(player);
      }

      this.sendStats(player);
      this.broadcast({ type: 'PLAYER_HEALTH', id: player.id, hp: player.hp, maxHp: player.maxHp });
      console.log(`[use] ${player.name} consumed ${obj.name ?? '?'}`);
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
        if (msg.hand === 'left') player.leftHand = null;
        else player.rightHand = null;
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
    victim.hp = Math.max(0, victim.hp - damage);

    this.broadcast({ type: 'PLAYER_HEALTH', id: victim.id, hp: victim.hp, maxHp: victim.maxHp });

    const atkName = attacker?.name ?? 'something';
    this.send(victim.ws, { type: 'REPORT', text: `${atkName} hits you for ${damage}!` });
    if (attacker) {
      this.send(attacker.ws, { type: 'REPORT', text: `You hit ${victim.name} for ${damage}.` });
    }

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
    const deathMsg: S2CMessage = {
      type: 'MESSAGE',
      from: 0,
      name: 'GM',
      to: 'all',
      text: `${victim.name} was slain by ${killerDesc}.`,
    };
    this.broadcast(deathMsg);
    this.chatHistory.push({ from: 0, name: 'GM', text: deathMsg.text });

    // Drop all inventory items
    this.dropPlayerItems(victim);

    // Schedule respawn after tombstone delay
    this.scheduleRespawn(victim, killer);
  }

  private dropPlayerItems(player: Player): void {
    const items: Array<InventoryItem | null> = [
      player.leftHand,
      player.rightHand,
      ...player.inventory,
    ];
    player.leftHand = null;
    player.rightHand = null;
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

    // Collect all walkable, unoccupied tiles
    const walkable: Array<{ x: number; y: number }> = [];
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        if (playerOccupied.has(`${x},${y}`)) continue;
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

    // Broadcast tombstone state at death location
    console.log(
      `[respawn] ${victim.name} is dead; broadcasting tombstone at room=${victim.room} (${victim.x},${victim.y})`,
    );
    this.broadcast(this.makePlayerInfo(victim));

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

    const spawn = this.randomSpawnForTeam(victim.team);
    if (spawn) {
      console.log(
        `[respawn] ${victim.name} team=${victim.team} spawn found: room=${spawn.room} (${spawn.x},${spawn.y})`,
      );
      victim.room = spawn.room;
      victim.x = spawn.x;
      victim.y = spawn.y;
    } else {
      console.warn(
        `[respawn] ${victim.name} team=${victim.team} NO SPAWN FOUND — staying at room=${victim.room} (${victim.x},${victim.y})`,
      );
    }

    victim.dead = false;

    // Broadcast alive state at new location
    console.log(
      `[respawn] broadcasting PLAYER_INFO for ${victim.name}: room=${victim.room} (${victim.x},${victim.y})`,
    );
    this.broadcast(this.makePlayerInfo(victim));

    // Tell the victim where they respawned
    console.log(
      `[respawn] sending YOU_RESPAWNED to ${victim.name}: room=${victim.room} (${victim.x},${victim.y})`,
    );
    this.send(victim.ws, { type: 'YOU_RESPAWNED', room: victim.room, x: victim.x, y: victim.y });

    this.broadcast({ type: 'PLAYER_HEALTH', id: victim.id, hp: victim.hp, maxHp: victim.maxHp });
    this.sendStats(victim);
    this.sendInventory(victim);

    console.log(
      `[respawn] ${victim.name} respawn complete at room=${victim.room} (${victim.x},${victim.y}) (killed by ${killerName})`,
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

    this.dropPlayerItems(player);

    this.players.delete(playerId);
    this.wsToId.delete(player.ws);
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

  // ── Helpers ───────────────────────────────────────────────────────────────

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

    // Spiral search outward from player position
    for (let radius = 0; radius <= 5; radius++) {
      for (let dx = -radius; dx <= radius; dx++) {
        for (let dy = -radius; dy <= radius; dy++) {
          if (Math.abs(dx) !== radius && Math.abs(dy) !== radius) continue;
          const tx = px + dx;
          const ty = py + dy;
          if (tx < 0 || tx >= GRID || ty < 0 || ty >= GRID) continue;
          if (roomMap.has(`${tx},${ty}`)) continue;
          if (playerOccupied.has(`${tx},${ty}`)) continue;
          const cell = room.spot?.[tx]?.[ty];
          if (cell) {
            const [flId, wlId] = cell;
            // Void tile: not walkable when room has a floor (ring-style map)
            if (!flId && !wlId) {
              if (room.floor) continue;
            } else {
              const wallObj = wlId > 0 ? this.world.objects[wlId] : null;
              const floorObj = flId > 0 ? this.world.objects[flId] : null;
              if (wallObj && !wallObj.movement) continue;
              if (floorObj && !floorObj.movement) continue;
            }
          }
          return { x: tx, y: ty };
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
