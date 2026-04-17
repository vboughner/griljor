import { Monster, MonsterDef, RoomMonsterSpawn } from './monster-types';
import { InventoryItem, S2CMessage } from './protocol';
import { World } from './world';

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
}

const GRID = 20;

export class MonsterManager {
  private monsters = new Map<number, Monster>();
  private nextMonsterId = -1;
  // Track spawn timers for replenishment (keyed by "roomIdx:monsterId")
  private spawnTimers = new Map<string, ReturnType<typeof setInterval>>();

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

  private clearMonsterTimers(monster: Monster): void {
    if (monster.moveTimer !== null) {
      clearTimeout(monster.moveTimer);
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
