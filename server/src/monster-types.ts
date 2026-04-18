import { InventoryItem } from './protocol';

// ── Monster Definition (loaded from JSON) ─────────────────────────────────

export interface MonsterBehaviorConfig {
  type: 'wander' | 'patrol' | 'chase' | 'flee' | 'stationary';
  moveInterval: number; // ms between movement ticks
  /** Wander: max Chebyshev distance from spawn; absent = unlimited */
  radius?: number;
  /** Wander: chance (0-1) of doing nothing on a tick */
  pauseChance?: number;
  /** Patrol: ordered waypoints to visit in a loop */
  waypoints?: Array<{ x: number; y: number }>;
  /** Chase: Chebyshev distance to chase before giving up (also used as aggro chase range) */
  chaseRange?: number;
  /** Flee: distance from threat before stopping */
  fleeRange?: number;
  /** Flee: what to do when no threat is nearby */
  idleBehavior?: 'wander' | 'stationary';
}

export interface MonsterCombatConfig {
  aggressive: boolean;
  /** Object type ID of weapon (from object definitions), or absent for punch-only */
  weaponType?: number;
  /** Chebyshev distance to detect enemy players */
  aggroRange?: number;
  /** Max distance to chase an enemy before giving up */
  pursuitRange?: number;
  /** Ms between attacks */
  fireInterval?: number;
  /** How to pick targets: "nearest" | "weakest" | "random" */
  targetPriority?: 'nearest' | 'weakest' | 'random';
}

export interface MonsterChatConfig {
  phrases: string[];
  chatInterval: number; // ms between chat attempts
  chatChance: number; // probability (0-1) of chatting on each interval
}

export interface MonsterDropConfig {
  type: number; // object type ID
  quantity: number;
  chance: number; // probability (0-1)
}

export interface MonsterPickupConfig {
  range: number; // Chebyshev distance to pick up items
  /** Object type IDs to pick up, or "any" for all takeable items */
  types: number[] | 'any';
  maxCarry: number;
}

export interface MonsterCarryConfig {
  /** Where to deliver items: "home" = spawn point, or a specific location */
  deliverTo: 'home';
  deliverRoom?: number;
}

export interface MonsterItemsConfig {
  drops: MonsterDropConfig[] | null;
  dropOnDeath: boolean;
  pickup: MonsterPickupConfig | null;
  carry: MonsterCarryConfig | null;
}

export interface MonsterDef {
  id: string;
  name: string;
  avatar: string;
  hp: number;
  maxHp: number;
  team: number;
  behavior: MonsterBehaviorConfig;
  combat: MonsterCombatConfig;
  chat: MonsterChatConfig | null;
  items: MonsterItemsConfig;
  respawn: { delay: number }; // ms before respawning after death; 0 = no respawn
}

// ── Room Monster Spawn Config (in map JSON) ───────────────────────────────

export interface RoomMonsterSpawn {
  monsterId: string; // references MonsterDef.id
  count: number; // max simultaneous alive instances
  spawnX?: number; // preferred spawn X (random walkable tile if omitted)
  spawnY?: number;
  spawnRate: number; // ms; 0 = spawn only at game start
}

// ── Runtime Monster Instance ──────────────────────────────────────────────

export interface Monster {
  id: number; // negative IDs (starting from -1, decrementing)
  defId: string; // references MonsterDef.id
  name: string;
  avatar: string;
  room: number;
  x: number;
  y: number;
  hp: number;
  maxHp: number;
  team: number;
  dead: boolean;
  carriedItems: InventoryItem[];
  homeRoom: number; // room it spawned in (for respawn + wander radius)
  homeX: number;
  homeY: number;
  // AI state
  currentTarget: number | null; // player ID being chased/attacked
  patrolIndex: number; // current waypoint for patrol behavior
  lastFireTime: number;
  // Timer handles
  moveTimer: ReturnType<typeof setTimeout> | null;
  chatTimer: ReturnType<typeof setTimeout> | null;
  respawnTimer: ReturnType<typeof setTimeout> | null;
}
