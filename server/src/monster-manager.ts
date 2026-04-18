import { Monster, MonsterDef, RoomMonsterSpawn } from './monster-types';
import { InventoryItem, S2CMessage } from './protocol';
import { World } from './world';
import { getBehavior, BehaviorContext, ChaseBehavior } from './behaviors';
import { calcMsPerStep } from './session';

// ── Narrow interface between MonsterManager and GameSession ────────────────

export interface MonsterSessionInterface {
  world: World;
  broadcastToRoom(room: number, msg: S2CMessage): void;
  broadcast(msg: S2CMessage): void;
  getPlayersInRoom(
    room: number,
  ): Array<{ id: number; x: number; y: number; team: number; dead: boolean }>;
  isWalkable(room: number, x: number, y: number): boolean;
  isTileOccupiedByPlayer(room: number, x: number, y: number, excludeId?: number): boolean;
  addFloorItem(room: number, x: number, y: number, item: InventoryItem): void;
  findNearbyFreeTile(room: number, x: number, y: number): { x: number; y: number } | null;
  getPlayerName(id: number): string | undefined;
  spotIsVisible(room: number, x1: number, y1: number, x2: number, y2: number): boolean;
  sendToPlayer(playerId: number, msg: S2CMessage): void;
  calcMissilePath(
    room: number,
    x0: number,
    y0: number,
    x1: number,
    y1: number,
    range: number,
  ): Array<{ x: number; y: number }>;
  dealDamageToPlayer(playerId: number, damage: number, attackerName: string): void;
  removeFloorItem(room: number, x: number, y: number): InventoryItem | null;
  getFloorItemsInRoom(room: number): Array<{ x: number; y: number; item: InventoryItem }>;
  getAllPlayers(): Array<{
    id: number;
    room: number;
    x: number;
    y: number;
    team: number;
    dead: boolean;
  }>;
}

const GRID = 20;

// Shared chase behavior instance for aggression override
const aggroChaseBehavior = new ChaseBehavior();

export class MonsterManager {
  private monsters = new Map<number, Monster>();
  private nextMonsterId = -1;
  private nextMissileId = -1; // negative IDs to avoid collision with session missile IDs
  // Track spawn timers for replenishment (keyed by "roomIdx:monsterId")
  private spawnTimers = new Map<string, ReturnType<typeof setInterval>>();
  // Monster visibility: playerId → Set of monster IDs visible to that player
  private monsterVisibility = new Map<number, Set<number>>();

  constructor(private session: MonsterSessionInterface) {}

  // ── Public API ──────────────────────────────────────────────────────────

  /** Spawn initial monsters per room config. Call after world is loaded. */
  init(): void {
    const { world } = this.session;
    console.log(
      `[monsters] init: ${world.monsterDefs.length} defs, checking ${world.rooms.length} rooms`,
    );
    for (let roomIdx = 0; roomIdx < world.rooms.length; roomIdx++) {
      const room = world.rooms[roomIdx];
      const spawns = room.monsterSpawns;
      if (!spawns) continue;
      for (const spawn of spawns) {
        const def = world.monsterDefs.find((d) => d.id === spawn.monsterId);
        if (!def) {
          console.warn(`[monsters] unknown monsterId "${spawn.monsterId}" in room ${roomIdx}`);
          continue;
        }
        // Spawn initial count
        for (let i = 0; i < spawn.count; i++) {
          this.spawnMonster(def, roomIdx, spawn);
        }
        // Set up replenishment timer if spawnRate > 0
        if (spawn.spawnRate > 0) {
          const key = `${roomIdx}:${spawn.monsterId}`;
          const timer = setInterval(() => {
            this.replenishSpawn(roomIdx, spawn, def);
          }, spawn.spawnRate);
          this.spawnTimers.set(key, timer);
        }
      }
    }
  }

  /** Clear all timers and state. */
  destroy(): void {
    for (const monster of this.monsters.values()) {
      this.clearMonsterTimers(monster);
    }
    this.monsters.clear();
    for (const timer of this.spawnTimers.values()) {
      clearInterval(timer);
    }
    this.spawnTimers.clear();
    this.monsterVisibility.clear();
  }

  /** Reset: destroy and re-init. */
  reset(): void {
    this.destroy();
    this.nextMonsterId = -1;
    this.nextMissileId = -1;
    this.init();
  }

  getMonster(id: number): Monster | undefined {
    return this.monsters.get(id);
  }

  getMonstersInRoom(room: number): Monster[] {
    const result: Monster[] = [];
    for (const m of this.monsters.values()) {
      if (m.room === room && !m.dead) result.push(m);
    }
    return result;
  }

  isMonster(id: number): boolean {
    return this.monsters.has(id);
  }

  /** Find a living monster on a specific tile. */
  findMonsterOnTile(room: number, x: number, y: number): Monster | null {
    for (const m of this.monsters.values()) {
      if (m.room === room && m.x === x && m.y === y && !m.dead) return m;
    }
    return null;
  }

  /** Apply damage to a monster. Returns true if the monster died. */
  damageMonster(id: number, damage: number, attackerId: number): boolean {
    const monster = this.monsters.get(id);
    if (!monster || monster.dead) return false;

    monster.hp = Math.max(0, monster.hp - damage);

    // Broadcast health update
    this.session.broadcastToRoom(monster.room, {
      type: 'PLAYER_HIT',
      victimId: monster.id,
      room: monster.room,
      x: monster.x,
      y: monster.y,
      damage,
    });

    if (monster.hp <= 0) {
      this.killMonster(monster, attackerId);
      return true;
    }
    return false;
  }

  /** Build MONSTER_INFO messages for all living monsters in a room. */
  getMonsterInfosForRoom(room: number): Array<Extract<S2CMessage, { type: 'MONSTER_INFO' }>> {
    return this.getMonstersInRoom(room).map((m) => this.makeMonsterInfo(m));
  }

  /** Initialize visibility tracking for a player (call on join). */
  initPlayerVisibility(playerId: number): void {
    this.monsterVisibility.set(playerId, new Set());
  }

  /** Remove visibility tracking for a player (call on leave). */
  clearPlayerVisibility(playerId: number): void {
    this.monsterVisibility.delete(playerId);
  }

  /**
   * Recompute which monsters a player can see, sending MONSTER_INFO for
   * newly visible monsters and MONSTER_HIDDEN for those that left LOS.
   * Call when a player moves or changes rooms.
   */
  updatePlayerVisibility(playerId: number, playerRoom: number, px: number, py: number): void {
    const visSet = this.monsterVisibility.get(playerId);
    if (!visSet) return;

    // Build set of monster IDs that should be visible now
    const nowVisible = new Set<number>();
    for (const m of this.monsters.values()) {
      if (m.dead || m.room !== playerRoom) continue;
      if (this.session.spotIsVisible(playerRoom, px, py, m.x, m.y)) {
        nowVisible.add(m.id);
      }
    }

    // Send MONSTER_INFO for newly visible
    for (const mId of nowVisible) {
      if (!visSet.has(mId)) {
        const m = this.monsters.get(mId);
        if (m) this.session.sendToPlayer(playerId, this.makeMonsterInfo(m));
      }
    }

    // Send MONSTER_HIDDEN for no longer visible
    for (const mId of visSet) {
      if (!nowVisible.has(mId)) {
        this.session.sendToPlayer(playerId, { type: 'MONSTER_HIDDEN', id: mId });
      }
    }

    // Replace the set
    this.monsterVisibility.set(playerId, nowVisible);
  }

  /**
   * When a player changes rooms, clear all monster visibility for them
   * (they'll get fresh visibility in the new room from updatePlayerVisibility).
   */
  onPlayerRoomChange(playerId: number): void {
    const visSet = this.monsterVisibility.get(playerId);
    if (!visSet) return;
    for (const mId of visSet) {
      this.session.sendToPlayer(playerId, { type: 'MONSTER_HIDDEN', id: mId });
    }
    visSet.clear();
  }

  // ── Internal ────────────────────────────────────────────────────────────

  private spawnMonster(def: MonsterDef, roomIdx: number, spawn: RoomMonsterSpawn): Monster | null {
    const pos = this.findSpawnPosition(roomIdx, spawn);
    if (!pos) {
      console.warn(`[monsters] no walkable tile for "${def.id}" in room ${roomIdx}`);
      return null;
    }

    const id = this.nextMonsterId--;
    const monster: Monster = {
      id,
      defId: def.id,
      name: def.name,
      avatar: def.avatar,
      room: roomIdx,
      x: pos.x,
      y: pos.y,
      hp: def.hp,
      maxHp: def.maxHp,
      team: def.team,
      dead: false,
      carriedItems: [],
      homeRoom: roomIdx,
      homeX: pos.x,
      homeY: pos.y,
      currentTarget: null,
      patrolIndex: 0,
      lastFireTime: 0,
      moveTimer: null,
      chatTimer: null,
      respawnTimer: null,
    };

    this.monsters.set(id, monster);
    console.log(
      `[monsters] spawned "${def.name}" (id=${id}, avatar=${def.avatar}) in room ${roomIdx} at (${pos.x},${pos.y})`,
    );

    // Start AI movement tick
    this.startMoveTick(monster, def);

    // Start chat timer if configured
    this.startChatTimer(monster, def);

    // Broadcast to players already in the room
    this.session.broadcastToRoom(roomIdx, this.makeMonsterInfo(monster));

    return monster;
  }

  private findSpawnPosition(
    roomIdx: number,
    spawn: RoomMonsterSpawn,
  ): { x: number; y: number } | null {
    // Try preferred spawn point first
    if (
      spawn.spawnX !== undefined &&
      spawn.spawnY !== undefined &&
      this.session.isWalkable(roomIdx, spawn.spawnX, spawn.spawnY) &&
      !this.isTileOccupied(roomIdx, spawn.spawnX, spawn.spawnY)
    ) {
      return { x: spawn.spawnX, y: spawn.spawnY };
    }

    // Fall back to random walkable tile
    return this.randomWalkableUnoccupiedTile(roomIdx);
  }

  /** Check if a tile is occupied by a monster or player. */
  private isTileOccupied(room: number, x: number, y: number): boolean {
    if (this.session.isTileOccupiedByPlayer(room, x, y)) return true;
    return this.findMonsterOnTile(room, x, y) !== null;
  }

  private randomWalkableUnoccupiedTile(roomIdx: number): { x: number; y: number } | null {
    const candidates: Array<{ x: number; y: number }> = [];
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        if (this.session.isWalkable(roomIdx, x, y) && !this.isTileOccupied(roomIdx, x, y)) {
          candidates.push({ x, y });
        }
      }
    }
    if (candidates.length === 0) return null;
    return candidates[Math.floor(Math.random() * candidates.length)];
  }

  private killMonster(monster: Monster, killerId: number): void {
    monster.dead = true;
    this.clearMonsterTimers(monster);

    const killerName = this.getKillerName(killerId);
    console.log(
      `[monsters] killed "${monster.name}" (id=${monster.id}, avatar=${monster.avatar}) in room ${monster.room} by ${killerName}`,
    );
    this.session.broadcast({
      type: 'MESSAGE',
      from: 0,
      name: 'GM',
      to: 'all',
      text: `${monster.name} was slain by ${killerName}.`,
    });

    // Broadcast dead state
    this.session.broadcastToRoom(monster.room, this.makeMonsterInfo(monster));

    // Drop loot
    this.dropLoot(monster);

    // Schedule respawn
    const def = this.session.world.monsterDefs.find((d) => d.id === monster.defId);
    if (def && def.respawn.delay > 0) {
      monster.respawnTimer = setTimeout(() => {
        monster.respawnTimer = null;
        this.respawnMonster(monster, def);
      }, def.respawn.delay);
    } else {
      // No respawn: remove from map after a tick so clients can see the death
      setTimeout(() => {
        this.monsters.delete(monster.id);
        this.session.broadcastToRoom(monster.room, {
          type: 'MONSTER_HIDDEN',
          id: monster.id,
        });
      }, 5000);
    }
  }

  private getKillerName(killerId: number): string {
    return this.session.getPlayerName(killerId) ?? 'the void';
  }

  private dropLoot(monster: Monster): void {
    const def = this.session.world.monsterDefs.find((d) => d.id === monster.defId);
    if (!def) return;

    const drops: InventoryItem[] = [];

    // Loot table drops
    if (def.items.drops) {
      for (const drop of def.items.drops) {
        if (Math.random() < drop.chance) {
          drops.push({ type: drop.type, quantity: drop.quantity });
        }
      }
    }

    // Carried items (from pickup behavior — future phase)
    if (def.items.dropOnDeath) {
      drops.push(...monster.carriedItems);
      monster.carriedItems = [];
    }

    // Place items on the floor near the death location
    for (const item of drops) {
      const tile = this.session.findNearbyFreeTile(monster.room, monster.x, monster.y);
      if (tile) {
        this.session.addFloorItem(monster.room, tile.x, tile.y, item);
      }
    }
  }

  private respawnMonster(monster: Monster, def: MonsterDef): void {
    const pos = this.randomWalkableUnoccupiedTile(monster.homeRoom);
    if (!pos) {
      console.warn(
        `[monsters] no walkable tile for respawn of "${def.id}" in room ${monster.homeRoom}`,
      );
      return;
    }

    monster.dead = false;
    monster.hp = def.hp;
    monster.room = monster.homeRoom;
    monster.x = pos.x;
    monster.y = pos.y;
    monster.carriedItems = [];
    monster.currentTarget = null;
    console.log(
      `[monsters] respawned "${def.name}" (id=${monster.id}, avatar=${def.avatar}) in room ${monster.homeRoom} at (${pos.x},${pos.y})`,
    );

    // Restart AI movement tick and chat timer
    this.startMoveTick(monster, def);
    this.startChatTimer(monster, def);

    this.session.broadcastToRoom(monster.room, this.makeMonsterInfo(monster));
  }

  private replenishSpawn(roomIdx: number, spawn: RoomMonsterSpawn, def: MonsterDef): void {
    // Count living monsters of this type in this room
    let aliveCount = 0;
    for (const m of this.monsters.values()) {
      if (m.defId === spawn.monsterId && m.room === roomIdx && !m.dead) {
        aliveCount++;
      }
    }
    if (aliveCount < spawn.count) {
      this.spawnMonster(def, roomIdx, spawn);
    }
  }

  private startMoveTick(monster: Monster, def: MonsterDef): void {
    if (monster.moveTimer !== null) {
      clearInterval(monster.moveTimer);
    }
    const interval = def.behavior.moveInterval;
    if (interval <= 0) return; // no movement
    monster.moveTimer = setInterval(() => {
      this.onMoveTick(monster, def);
    }, interval);
  }

  private onMoveTick(monster: Monster, def: MonsterDef): void {
    if (monster.dead) return;

    const context = this.buildBehaviorContext(monster);

    // Aggression override: if aggressive and enemy in aggroRange with LOS, chase + fire
    if (def.combat.aggressive) {
      const target = this.findAggroTarget(monster, def, context);
      if (target) {
        monster.currentTarget = target.id;
        // Chase toward target
        const action = aggroChaseBehavior.onTick(monster, def.behavior, context);
        if (action.type === 'move') {
          this.moveMonster(monster, action.x, action.y);
        }
        // Try to fire at target
        this.tryFireAtTarget(monster, def, target);
        return;
      } else {
        monster.currentTarget = null;
      }
    }

    // Primary behavior
    const behavior = getBehavior(def.behavior.type);
    const action = behavior.onTick(monster, def.behavior, context);

    if (action.type === 'move') {
      this.moveMonster(monster, action.x, action.y);
    }

    // Item interactions (after movement)
    this.tryPickupItem(monster, def);
    this.tryDeliverItems(monster, def);
  }

  private findAggroTarget(
    monster: Monster,
    def: MonsterDef,
    context: BehaviorContext,
  ): { id: number; x: number; y: number } | null {
    const aggroRange = def.combat.aggroRange ?? 8;
    let nearest: { id: number; x: number; y: number } | null = null;
    let nearestDist = Infinity;

    for (const p of context.nearbyPlayers) {
      if (p.team === monster.team && monster.team !== 0) continue;
      const dist = Math.max(Math.abs(p.x - monster.x), Math.abs(p.y - monster.y));
      if (dist <= aggroRange && dist < nearestDist) {
        // Check LOS
        if (this.session.spotIsVisible(monster.room, monster.x, monster.y, p.x, p.y)) {
          nearestDist = dist;
          nearest = p;
        }
      }
    }

    return nearest;
  }

  private tryFireAtTarget(
    monster: Monster,
    def: MonsterDef,
    target: { id: number; x: number; y: number },
  ): void {
    const weaponType = def.combat.weaponType;
    if (weaponType === undefined) return; // no weapon configured

    const fireInterval = def.combat.fireInterval ?? 1200;
    const now = Date.now();
    if (now - monster.lastFireTime < fireInterval) return;

    const weaponObj = this.session.world.objects[weaponType];
    if (!weaponObj) return;

    const damage = weaponObj.damage ?? 10;
    const range = weaponObj.range ?? 5;
    const movingObjType = weaponObj.movingobj ?? weaponType;
    const bulletObj = weaponObj.movingobj ? this.session.world.objects[weaponObj.movingobj] : null;
    const speed = bulletObj?.speed ?? weaponObj.speed ?? 5;
    const msPerStep = calcMsPerStep(speed);

    // Check range to target
    const dist = Math.max(Math.abs(target.x - monster.x), Math.abs(target.y - monster.y));
    if (dist > range) return;

    monster.lastFireTime = now;

    // Compute missile path
    const path = this.session.calcMissilePath(
      monster.room,
      monster.x,
      monster.y,
      target.x,
      target.y,
      range,
    );
    if (path.length === 0) return;

    // Check if a player is hit along the path
    let hitPlayerId: number | undefined;
    let hitAtStep = path.length;
    for (let i = 0; i < path.length; i++) {
      for (const p of this.session.getPlayersInRoom(monster.room)) {
        if (p.dead) continue;
        if (p.x === path[i].x && p.y === path[i].y) {
          hitPlayerId = p.id;
          hitAtStep = i + 1;
          break;
        }
      }
      if (hitPlayerId !== undefined) break;
    }

    const finalPath = path.slice(0, hitAtStep);
    const dx = Math.sign(target.x - monster.x);
    const dy = Math.sign(target.y - monster.y);

    const missileId = this.nextMissileId--;

    this.session.broadcastToRoom(monster.room, {
      type: 'MISSILE_START',
      id: missileId,
      room: monster.room,
      path: finalPath,
      objType: movingObjType,
      msPerStep,
      dx,
      dy,
    });

    const travelMs = finalPath.length * msPerStep;
    const capturedHitPlayerId = hitPlayerId;
    const capturedDamage = damage;
    const capturedName = monster.name;
    const capturedRoom = monster.room;

    setTimeout(() => {
      this.session.broadcastToRoom(capturedRoom, { type: 'MISSILE_END', id: missileId });
      if (capturedHitPlayerId !== undefined) {
        this.session.dealDamageToPlayer(capturedHitPlayerId, capturedDamage, capturedName);
      }
    }, travelMs);
  }

  private tryPickupItem(monster: Monster, def: MonsterDef): void {
    const pickup = def.items.pickup;
    if (!pickup) return;
    if (monster.carriedItems.length >= pickup.maxCarry) return;

    // Find the nearest floor item within pickup range
    const floorItems = this.session.getFloorItemsInRoom(monster.room);
    let bestItem: { x: number; y: number; item: InventoryItem } | null = null;
    let bestDist = Infinity;

    for (const fi of floorItems) {
      const dist = Math.max(Math.abs(fi.x - monster.x), Math.abs(fi.y - monster.y));
      if (dist > pickup.range) continue;
      if (dist >= bestDist) continue;

      // Check type filter
      if (pickup.types !== 'any') {
        if (!pickup.types.includes(fi.item.type)) continue;
      } else {
        // "any" = all takeable items
        const obj = this.session.world.objects[fi.item.type];
        if (!obj?.takeable) continue;
      }

      bestDist = dist;
      bestItem = fi;
    }

    if (!bestItem) return;

    // Pick it up
    const removed = this.session.removeFloorItem(monster.room, bestItem.x, bestItem.y);
    if (removed) {
      monster.carriedItems.push(removed);
    }
  }

  private tryDeliverItems(monster: Monster, def: MonsterDef): void {
    const carry = def.items.carry;
    if (!carry) return;
    if (monster.carriedItems.length === 0) return;

    if (carry.deliverTo === 'home') {
      // Deliver when at home position
      if (monster.room !== monster.homeRoom) return;
      if (monster.x !== monster.homeX || monster.y !== monster.homeY) return;

      // Drop all carried items near home
      const items = [...monster.carriedItems];
      monster.carriedItems = [];
      for (const item of items) {
        const tile = this.session.findNearbyFreeTile(monster.room, monster.x, monster.y);
        if (tile) {
          this.session.addFloorItem(monster.room, tile.x, tile.y, item);
        }
      }
    }
  }

  private buildBehaviorContext(monster: Monster): BehaviorContext {
    const players = this.session.getPlayersInRoom(monster.room).filter((p) => !p.dead);

    return {
      nearbyPlayers: players,
      isWalkable: (x, y) => this.session.isWalkable(monster.room, x, y),
      isOccupied: (x, y) => this.isTileOccupied(monster.room, x, y),
    };
  }

  private moveMonster(monster: Monster, newX: number, newY: number): void {
    monster.x = newX;
    monster.y = newY;

    // Update visibility for all players in this room
    for (const player of this.session.getAllPlayers()) {
      if (player.room !== monster.room) continue;
      const visSet = this.monsterVisibility.get(player.id);
      if (!visSet) continue;

      const canSee = this.session.spotIsVisible(
        monster.room,
        player.x,
        player.y,
        monster.x,
        monster.y,
      );
      const wasSeen = visSet.has(monster.id);

      if (canSee && !wasSeen) {
        // Newly visible: send full info
        visSet.add(monster.id);
        this.session.sendToPlayer(player.id, this.makeMonsterInfo(monster));
      } else if (canSee && wasSeen) {
        // Still visible: send location update
        this.session.sendToPlayer(player.id, {
          type: 'MONSTER_LOCATION',
          id: monster.id,
          room: monster.room,
          x: monster.x,
          y: monster.y,
        });
      } else if (!canSee && wasSeen) {
        // No longer visible
        visSet.delete(monster.id);
        this.session.sendToPlayer(player.id, { type: 'MONSTER_HIDDEN', id: monster.id });
      }
    }
  }

  private startChatTimer(monster: Monster, def: MonsterDef): void {
    if (!def.chat) return;
    if (monster.chatTimer !== null) {
      clearInterval(monster.chatTimer);
    }
    const { chatInterval, chatChance, phrases } = def.chat;
    if (chatInterval <= 0 || phrases.length === 0) return;
    monster.chatTimer = setInterval(() => {
      if (monster.dead) return;
      if (Math.random() >= chatChance) return;
      const text = phrases[Math.floor(Math.random() * phrases.length)];
      this.session.broadcastToRoom(monster.room, {
        type: 'MESSAGE',
        from: monster.id,
        name: monster.name,
        to: 'all',
        text,
      });
    }, chatInterval);
  }

  private clearMonsterTimers(monster: Monster): void {
    if (monster.moveTimer !== null) {
      clearInterval(monster.moveTimer);
      monster.moveTimer = null;
    }
    if (monster.chatTimer !== null) {
      clearInterval(monster.chatTimer);
      monster.chatTimer = null;
    }
    if (monster.respawnTimer !== null) {
      clearTimeout(monster.respawnTimer);
      monster.respawnTimer = null;
    }
  }

  private makeMonsterInfo(m: Monster): Extract<S2CMessage, { type: 'MONSTER_INFO' }> {
    return {
      type: 'MONSTER_INFO',
      id: m.id,
      name: m.name,
      avatar: m.avatar,
      room: m.room,
      x: m.x,
      y: m.y,
      hp: m.hp,
      maxHp: m.maxHp,
      team: m.team,
      dead: m.dead,
      monsterId: m.defId,
    };
  }
}
