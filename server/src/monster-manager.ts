import { Monster, MonsterDef, RoomMonsterSpawn } from './monster-types';
import { InventoryItem, S2CMessage } from './protocol';
import { World } from './world';
import { getBehavior, BehaviorContext } from './behaviors';

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

export class MonsterManager {
  private monsters = new Map<number, Monster>();
  private nextMonsterId = -1;
  // Track spawn timers for replenishment (keyed by "roomIdx:monsterId")
  private spawnTimers = new Map<string, ReturnType<typeof setInterval>>();
  // Monster visibility: playerId → Set of monster IDs visible to that player
  private monsterVisibility = new Map<number, Set<number>>();

  constructor(private session: MonsterSessionInterface) {}

  // ── Public API ──────────────────────────────────────────────────────────

  /** Spawn initial monsters per room config. Call after world is loaded. */
  init(): void {
    const { world } = this.session;
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

    // Start AI movement tick
    this.startMoveTick(monster, def);

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

    // Announce death
    const killerName = this.getKillerName(killerId);
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
      }, 100);
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

    // Restart AI movement tick
    this.startMoveTick(monster, def);

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

    const behavior = getBehavior(def.behavior.type);
    const context = this.buildBehaviorContext(monster);
    const action = behavior.onTick(monster, def.behavior, context);

    if (action.type === 'move') {
      this.moveMonster(monster, action.x, action.y);
    }
    // 'idle' → do nothing
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

  private clearMonsterTimers(monster: Monster): void {
    if (monster.moveTimer !== null) {
      clearInterval(monster.moveTimer);
      monster.moveTimer = null;
    }
    if (monster.chatTimer !== null) {
      clearTimeout(monster.chatTimer);
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
