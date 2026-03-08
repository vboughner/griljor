import WebSocket from 'ws';
import { C2SMessage, S2CMessage, InventoryItem } from './protocol';
import { World, ObjDef, RecObj } from './world';
import { filterText, randomScold } from './filter';

const INV_SIZE = 35;
const MAX_WEIGHT = 150;
const GRID = 20;

/**
 * For numbered items (guns, potions with charges), quantity represents the
 * charge count — the item itself is still ONE physical object. Weight is
 * per-item, not per-charge.  For non-numbered stackable items, multiply by
 * the stack size.
 */
function calcItemWeight(obj: ObjDef | null | undefined, item: InventoryItem): number {
  if (!obj) return 0;
  if (obj.numbered) return obj.weight ?? 0;
  return (obj.weight ?? 0) * item.quantity;
}

// Level/XP progression
const BASE_HP    = 100;
const BASE_POWER = 50;
const HP_PER_LEVEL    = 20;
const POWER_PER_LEVEL = 10;
const XP_PER_LEVEL    = 100; // level N requires N*100 XP to advance

function maxHpForLevel(level: number): number    { return BASE_HP    + (level - 1) * HP_PER_LEVEL; }
function maxPowerForLevel(level: number): number { return BASE_POWER + (level - 1) * POWER_PER_LEVEL; }

function levelForXp(xp: number): number {
  let level = 1;
  let xpLeft = xp;
  let needed = level * XP_PER_LEVEL;
  while (xpLeft >= needed) {
    xpLeft -= needed;
    level++;
    needed = level * XP_PER_LEVEL;
  }
  return level;
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
  // combat stats
  hp: number;
  maxHp: number;
  power: number;
  maxPower: number;
  xp: number;
  level: number;
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
      r.recorded_objects.map((ro) => ({ ...ro }))
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
      this.world.rooms[i].recorded_objects = this.originalRecordedObjects[i].map((ro) => ({ ...ro }));
    }
    this.roomItems.clear();
    this.initRoomItems();
    this.chatHistory = [];
  }

  get playerCount(): number { return this.players.size; }
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
          case 'MY_LOCATION':  this.onLocation(playerId, msg);   break;
          case 'MESSAGE':      this.onMessage(playerId, msg);    break;
          case 'LEAVING_GAME': this.onLeave(playerId);           break;
          case 'PICKUP':       this.onPickup(playerId, msg);     break;
          case 'DROP':         this.onDrop(playerId, msg);       break;
          case 'INV_SWAP':     this.onInvSwap(playerId, msg);   break;
          case 'FIRE_WEAPON':  this.onFireWeapon(playerId, msg); break;
          case 'USE_ITEM':     this.onUseItem(playerId, msg);    break;
          case 'PING':         break; // no-op: keeps the connection alive
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
    const player: Player = {
      id, name: msg.name, avatar: msg.avatar, room: 0, x: 10, y: 10, ws,
      kills: 0, deaths: 0, joinedAt: Date.now(),
      leftHand: null, rightHand: null,
      inventory: new Array<InventoryItem | null>(INV_SIZE).fill(null),
      currentWeight: 0,
      hp: BASE_HP, maxHp: BASE_HP,
      power: BASE_POWER, maxPower: BASE_POWER,
      xp: 0, level: 1,
    };
    this.players.set(id, player);
    this.wsToId.set(ws, id);
    this.onPlayerCountChange?.();

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
      // Also send their current HP so the new player sees health bars
      this.send(ws, { type: 'PLAYER_HEALTH', id: other.id, hp: other.hp, maxHp: other.maxHp });
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

    // Send empty inventory and starting stats to new player
    this.sendInventory(player);
    this.sendStats(player);

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
    const itemWeight = calcItemWeight(obj, item);

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
      if (hand === 'left') player.leftHand = item;
      else player.rightHand = item;
    } else {
      const freeSlot = player.inventory.indexOf(null);
      if (freeSlot === -1) {
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
    const itemWeight = calcItemWeight(obj, item);
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

  // ── Combat ────────────────────────────────────────────────────────────────

  private onFireWeapon(playerId: number, msg: Extract<C2SMessage, { type: 'FIRE_WEAPON' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;

    const handItem = msg.hand === 'left' ? player.leftHand : player.rightHand;
    if (!handItem) return;

    const obj = this.world.objects[handItem.type];
    if (!obj?.weapon) return;

    // For numbered items (guns, staves), require charges
    if (obj.numbered && handItem.quantity <= 0) return;

    // Damage may live on the bullet/projectile object rather than the weapon itself
    const bulletObj = obj.movingobj ? this.world.objects[obj.movingobj] : null;
    const damage = obj.damage ?? bulletObj?.damage ?? 10;
    const range  = obj.range  ?? 5;
    const movingObjType = obj.movingobj ?? handItem.type;

    // Decrement ammo/charges for numbered weapons
    if (obj.numbered) {
      handItem.quantity--;
      if (handItem.quantity <= 0) {
        if (msg.hand === 'left') player.leftHand  = null;
        else                     player.rightHand = null;
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
    const path: Array<{x: number, y: number}> = [];
    {
      const x0 = player.x, y0 = player.y;
      const x1 = msg.targetX, y1 = msg.targetY;
      const adx = Math.abs(x1 - x0), ady = Math.abs(y1 - y0);
      const sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
      let err = adx - ady;
      let cx = x0, cy = y0;
      while (path.length < range) {
        const e2 = 2 * err;
        if (e2 > -ady) { err -= ady; cx += sx; }
        if (e2 <  adx) { err += adx; cy += sy; }
        if (cx < 0 || cx >= GRID || cy < 0 || cy >= GRID) break;
        const cell = room.spot?.[cx]?.[cy];
        if (cell) {
          const [flId, wlId] = cell;
          const wallObj  = wlId > 0 ? this.world.objects[wlId] : null;
          const floorObj = flId > 0 ? this.world.objects[flId] : null;
          if (wallObj  && !wallObj.permeable)  break;
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
      id, room: player.room,
      path: finalPath,
      objType: movingObjType,
      msPerStep,
      dx, dy,
    });

    // Apply damage when missile arrives; also signal clients to clear the sprite
    const travelMs = finalPath.length * msPerStep;
    const timer = setTimeout(() => {
      this.activeMissiles.delete(id);
      this.broadcastToRoom(player.room, { type: 'MISSILE_END', id });
      if (hitPlayer) this.dealDamage(hitPlayer, damage, player);
    }, travelMs);
    this.activeMissiles.set(id, timer);

    console.log(`[combat] ${player.name} fired ${obj.name ?? '?'} (${finalPath.length} steps @ ${msPerStep}ms)${hitPlayer ? ` → hits ${hitPlayer.name}` : ''}`);
  }

  private onUseItem(playerId: number, msg: Extract<C2SMessage, { type: 'USE_ITEM' }>): void {
    const player = this.players.get(playerId);
    if (!player) return;

    const handItem = msg.hand === 'left' ? player.leftHand : player.rightHand;
    if (!handItem) return;

    const obj = this.world.objects[handItem.type];
    if (!obj) return;

    // --- Consumable branch (same-tile use) ---
    if ((obj.health ?? 0) < 0 || (obj.mana ?? 0) < 0) {
      const healsHp    = (obj.health ?? 0) < 0;
      const restoresMp = (obj.mana   ?? 0) < 0;
      // Block if every applicable stat is already full
      const hpFull  = player.hp    >= player.maxHp;
      const mpFull  = player.power >= player.maxPower;
      if ((!healsHp || hpFull) && (!restoresMp || mpFull)) return;

      if (healsHp)    player.hp    = Math.min(player.maxHp,    player.hp    - (obj.health ?? 0));
      if (restoresMp) player.power = Math.min(player.maxPower, player.power - (obj.mana   ?? 0));

      if (obj.numbered) {
        handItem.quantity--;
        if (handItem.quantity <= 0) {
          if (msg.hand === 'left') player.leftHand  = null;
          else                     player.rightHand = null;
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
        if (msg.hand === 'left') player.leftHand  = null;
        else                     player.rightHand = null;
      }
      this.sendInventory(player);
    }

    if (toggled) {
      console.log(`[use] ${player.name} used ${obj.name ?? '?'} on (${msg.targetX},${msg.targetY})`);
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
    victim.deaths++;
    this.broadcast({ type: 'PLAYER_STATS', id: victim.id, kills: victim.kills, deaths: victim.deaths });

    if (killer) {
      killer.kills++;
      const xpReward = 10 + victim.level * 10;
      killer.xp += xpReward;

      // Check level-up
      const newLevel = levelForXp(killer.xp);
      if (newLevel > killer.level) {
        killer.level = newLevel;
        killer.maxHp    = maxHpForLevel(killer.level);
        killer.maxPower = maxPowerForLevel(killer.level);
        killer.hp = Math.min(killer.hp, killer.maxHp);
        this.send(killer.ws, { type: 'REPORT', text: `Level up! You are now level ${killer.level}.` });
      }

      this.send(killer.ws, { type: 'REPORT', text: `You killed ${victim.name}! (+${xpReward} XP)` });
      this.broadcast({ type: 'PLAYER_STATS', id: killer.id, kills: killer.kills, deaths: killer.deaths });
      this.sendStats(killer);
    }

    // Announce death in global chat
    const killerDesc = killer ? killer.name : 'the void';
    const deathMsg: S2CMessage = {
      type: 'MESSAGE', from: 0, name: 'GM', to: 'all',
      text: `${victim.name} was slain by ${killerDesc}.`,
    };
    this.broadcast(deathMsg);
    this.chatHistory.push({ from: 0, name: 'GM', text: deathMsg.text });

    // Drop all inventory items
    this.dropPlayerItems(victim);

    // Respawn
    this.respawnPlayer(victim, killer);
  }

  private dropPlayerItems(player: Player): void {
    const items: Array<InventoryItem | null> = [
      player.leftHand, player.rightHand, ...player.inventory
    ];
    player.leftHand = null;
    player.rightHand = null;
    player.inventory.fill(null);
    player.currentWeight = 0;

    for (const item of items) {
      if (!item) continue;
      const tile = this.nearbyFreeTile(player.room, player.x, player.y);
      if (tile) {
        const roomMap = this.roomItems.get(player.room) ?? new Map<string, InventoryItem>();
        roomMap.set(`${tile.x},${tile.y}`, item);
        this.roomItems.set(player.room, roomMap);
        this.broadcast({ type: 'ITEM_ADDED', room: player.room, x: tile.x, y: tile.y, item });
      }
    }
  }

  private respawnPlayer(victim: Player, _killer: Player | null): void {
    victim.hp    = victim.maxHp;
    victim.power = victim.maxPower;

    // Find a free spawn tile near (10,10) in room 0
    const spawn = this.nearbyFreeTile(0, 10, 10) ?? { x: 10, y: 10 };
    victim.room = 0;
    victim.x = spawn.x;
    victim.y = spawn.y;

    // Tell victim they died and where they respawned
    this.send(victim.ws, {
      type: 'YOU_DIED',
      killedBy: _killer?.id ?? 0,
      killerName: _killer?.name ?? 'the void',
      respawnRoom: victim.room,
      respawnX: victim.x,
      respawnY: victim.y,
    });

    // Broadcast new position to all
    this.broadcast({
      type: 'PLAYER_INFO',
      id: victim.id, name: victim.name, avatar: victim.avatar,
      room: victim.room, x: victim.x, y: victim.y,
      kills: victim.kills, deaths: victim.deaths, joinedAt: victim.joinedAt,
    });

    this.broadcast({ type: 'PLAYER_HEALTH', id: victim.id, hp: victim.hp, maxHp: victim.maxHp });
    this.sendStats(victim);
    this.sendInventory(victim);

    console.log(`[combat] ${victim.name} respawned at room=${victim.room} (${victim.x},${victim.y})`);
  }

  // ── Leave ─────────────────────────────────────────────────────────────────

  private onLeave(playerId: number): void {
    const player = this.players.get(playerId);
    if (!player) return;

    this.dropPlayerItems(player);

    this.players.delete(playerId);
    this.wsToId.delete(player.ws);
    this.broadcast({ type: 'LEAVING_GAME', id: playerId });
    if (this.players.size === 0) {
      if (this.world.resetOnEmpty) {
        const delay = this.world.resetAfterSeconds * 1000;
        console.log(`[reset] scheduled in ${this.world.resetAfterSeconds}s (${this.world.mapName} is empty)`);
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
      hp: player.hp, maxHp: player.maxHp,
      power: player.power, maxPower: player.maxPower,
      xp: player.xp, level: player.level,
    });
  }

  private nearbyFreeTile(roomIdx: number, px: number, py: number): { x: number; y: number } | null {
    const room = this.world.rooms[roomIdx];
    if (!room) return null;
    const roomMap = this.roomItems.get(roomIdx) ?? new Map<string, InventoryItem>();

    // Build player-occupied set once to avoid repeated Map iteration per tile
    const playerOccupied = new Set<string>();
    for (const p of this.players.values()) {
      if (p.room === roomIdx) playerOccupied.add(`${p.x},${p.y}`);
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
            const wallObj  = wlId > 0 ? this.world.objects[wlId] : null;
            const floorObj = flId > 0 ? this.world.objects[flId] : null;
            if (wallObj  && !wallObj.movement)  continue;
            if (floorObj && !floorObj.movement) continue;
          }
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

  private broadcastToRoom(roomIdx: number, msg: S2CMessage): void {
    for (const player of this.players.values()) {
      if (player.room === roomIdx) this.send(player.ws, msg);
    }
  }
}
