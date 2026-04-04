# Wandering Monsters System — Design Plan

## 1. Architecture Overview

The monster system is a **companion module** to `GameSession`, not embedded within it. Monsters are server-side entities with **negative IDs** (distinguishing them from players, which have positive IDs). They reuse existing combat, visibility, and movement infrastructure through a well-defined interface with `GameSession`. The AI runs on event-driven timers, consistent with the existing architecture (no game loop).

### Key Design Decisions

1. **Negative IDs** for monsters — simple, no-collision discrimination from players. `id < 0` means monster everywhere.
2. **Separate module, narrow interface** — `MonsterManager` communicates with `GameSession` through `MonsterSessionInterface`, keeping session.ts changes minimal and MonsterManager independently testable.
3. **Same combat pipeline** — monsters fire the same weapons using the same missile math. No parallel combat system.
4. **Event-driven timers, not a game loop** — each monster has its own `setInterval` for movement ticks, matching the existing regen/AFK timer patterns.
5. **Explicit MONSTER_* protocol messages** — distinct message types (not overloaded PLAYER_INFO) keep the client clean.
6. **Behavior as Strategy pattern** — new monster behaviors can be added by implementing `BehaviorHandler` and registering it, without touching existing code.

---

## 2. Monster Definition File Format

A new JSON file lives alongside object definitions at `pipeline/out/data/monsters/`. Each map references a monster definition file, similar to how maps reference object files via `objfilename`.

**File: `pipeline/out/data/monsters/{name}.json`**

```json
{
  "monsters": [
    {
      "id": "dweeb",
      "name": "Dweeb",
      "avatar": "dweeb",
      "hp": 50,
      "maxHp": 50,
      "team": 0,
      "speed": 3,
      "behavior": {
        "type": "wander",
        "moveInterval": 2000,
        "pauseChance": 0.3
      },
      "combat": {
        "aggressive": false
      },
      "chat": {
        "phrases": ["Meep!", "Bloop bloop.", "Dweeb dweeb!"],
        "chatInterval": 15000,
        "chatChance": 0.2
      },
      "items": {
        "drops": [
          { "type": 42, "quantity": 1, "chance": 1.0 }
        ],
        "dropOnDeath": true,
        "pickup": null,
        "carry": null
      },
      "respawn": {
        "delay": 30000
      }
    },
    {
      "id": "guard",
      "name": "Guard",
      "avatar": "robot",
      "hp": 80,
      "maxHp": 80,
      "team": 1,
      "speed": 5,
      "behavior": {
        "type": "patrol",
        "moveInterval": 1500,
        "waypoints": "room",
        "chaseRange": 6,
        "chaseSpeed": 7,
        "returnAfterChase": true
      },
      "combat": {
        "aggressive": true,
        "aggroRange": 8,
        "weaponType": 45,
        "fireInterval": 1200,
        "targetPriority": "nearest"
      },
      "chat": null,
      "items": {
        "drops": [
          { "type": 45, "quantity": 1, "chance": 0.5 },
          { "type": 3, "quantity": 1, "chance": 0.8 }
        ],
        "dropOnDeath": true,
        "pickup": null,
        "carry": null
      },
      "respawn": {
        "delay": 60000
      }
    },
    {
      "id": "thief",
      "name": "Thief",
      "avatar": "dodger",
      "hp": 30,
      "maxHp": 30,
      "team": 0,
      "speed": 8,
      "behavior": {
        "type": "flee",
        "moveInterval": 800,
        "fleeRange": 6,
        "idleBehavior": "wander"
      },
      "combat": {
        "aggressive": false
      },
      "chat": {
        "phrases": ["Heh heh heh!", "Mine now!"],
        "chatInterval": 10000,
        "chatChance": 0.4
      },
      "items": {
        "drops": null,
        "dropOnDeath": true,
        "pickup": {
          "range": 3,
          "types": "any",
          "maxCarry": 3
        },
        "carry": {
          "deliverTo": "home",
          "deliverRoom": null
        }
      },
      "respawn": {
        "delay": 45000
      }
    }
  ]
}
```

### Schema Rationale

- **`behavior.type`** is a discriminated union: `"wander"`, `"patrol"`, `"chase"`, `"flee"`, `"stationary"`. New types can be added by implementing a new `BehaviorHandler` without modifying existing ones.
- **`combat.aggressive`** is a boolean (Option A). When `true`, the monster will chase and attack non-team players in `aggroRange`, temporarily overriding its primary movement behavior. When `false`, the monster never initiates attacks. A future Option B upgrade could replace this with a richer `aggression` enum (`"none"`, `"passive"`, `"defensive"`, `"aggressive"`).
- **`combat.weaponType`** references existing weapon `type` IDs from the object definition file. The monster "equips" this weapon and fires it using the same `calcMissilePath`/`dealDamage` logic players use.
- **`items.pickup`** and **`items.carry`** enable the thief/collector pattern: the monster picks up floor items, carries them, and deposits them elsewhere. `dropOnDeath` means carried items scatter when killed.
- **`chat`** is optional. Monsters with `chat` periodically say things in the room via MESSAGE protocol.

---

## 3. Map Integration

Additions to the existing map JSON format. All new fields are optional.

### Map level (alongside existing `map` object):

```json
{
  "map": {
    "name": "Desert Isle Paradise 3",
    "objfilename": "default.obj",
    "monsterfile": "default_monsters.json",
    "...existing fields..."
  }
}
```

### Room level (alongside existing room properties):

```json
{
  "name": "Dungeon",
  "floor": 156,
  "team": 0,
  "...existing fields...",
  "monsters": [
    {
      "monsterId": "dweeb",
      "count": 3,
      "spawnX": 10,
      "spawnY": 10,
      "spawnRate": 60000
    },
    {
      "monsterId": "guard",
      "count": 1,
      "spawnX": 5,
      "spawnY": 5,
      "spawnRate": 0
    }
  ]
}
```

- **`monsterId`** references a monster `id` in the monster definition file.
- **`count`** is the maximum simultaneous alive instances of this monster type in this room.
- **`spawnX/Y`** is the preferred spawn location (random walkable tile in room if omitted or occupied).
- **`spawnRate`** (ms): `0` = spawn on session start only; positive = also spawn replacements on this interval when count is below max. This is the "propagation" mechanism.

### World Interface Additions

```typescript
export interface MonsterDef {
  id: string;
  name: string;
  avatar: string;
  hp: number;
  maxHp: number;
  team: number;
  speed: number;
  behavior: BehaviorConfig;
  combat: CombatConfig;
  chat: ChatConfig | null;
  items: ItemConfig;
  respawn: { delay: number };
}

export interface RoomMonsterSpawn {
  monsterId: string;
  count: number;
  spawnX?: number;
  spawnY?: number;
  spawnRate: number;
}

// Added to RoomData:
export interface RoomData {
  // ...existing fields...
  monsters?: RoomMonsterSpawn[];
}

// Added to World:
export interface World {
  // ...existing fields...
  monsterDefs: MonsterDef[];
}
```

---

## 4. Server-Side Monster Entity System

### Monster Entity (`server/src/monster-types.ts`)

```typescript
interface Monster {
  id: number;            // negative IDs, starting from -1
  defId: string;         // references MonsterDef.id
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
  homeRoom: number;
  homeX: number;
  homeY: number;
  // Timer handles
  moveTimer: ReturnType<typeof setTimeout> | null;
  fireTimer: ReturnType<typeof setTimeout> | null;
  chatTimer: ReturnType<typeof setTimeout> | null;
  respawnTimer: ReturnType<typeof setTimeout> | null;
  lastFireTime: number;
}
```

### MonsterManager (`server/src/monster-manager.ts`)

```typescript
class MonsterManager {
  private monsters = new Map<number, Monster>();
  private nextMonsterId = -1;
  private spawnTimers = new Map<string, ReturnType<typeof setInterval>>();

  constructor(private session: MonsterSessionInterface) {}

  init(): void        // spawn initial monsters per room config
  destroy(): void     // clear all timers

  getMonster(id: number): Monster | undefined;
  getMonstersInRoom(room: number): Monster[];
  isMonster(id: number): boolean;

  damageMonster(id: number, damage: number, attackerId: number): void;
  findMonsterOnTile(room: number, x: number, y: number): Monster | null;
}
```

### MonsterSessionInterface (narrow coupling)

```typescript
interface MonsterSessionInterface {
  world: World;
  broadcastToRoom(room: number, msg: S2CMessage): void;
  broadcast(msg: S2CMessage): void;
  getPlayersInRoom(room: number): Array<{
    id: number; x: number; y: number; team: number; dead: boolean;
  }>;
  isWalkable(room: number, x: number, y: number): boolean;
  isTileOccupied(room: number, x: number, y: number, excludeId?: number): boolean;
  calcMissilePath(...): Array<{x: number; y: number}>;
  addFloorItem(room: number, x: number, y: number, item: InventoryItem): void;
  removeFloorItem(room: number, x: number, y: number): InventoryItem | null;
  findNearbyFreeTile(room: number, x: number, y: number): { x: number; y: number } | null;
  spotIsVisible(room: number, x1: number, y1: number, x2: number, y2: number): boolean;
}
```

---

## 5. AI/Behavior System (`server/src/behaviors.ts`)

Strategy pattern where each behavior type implements a common interface:

```typescript
interface BehaviorHandler {
  onTick(monster: Monster, context: BehaviorContext): BehaviorAction;
}

type BehaviorAction =
  | { type: 'move'; x: number; y: number }
  | { type: 'fire'; targetX: number; targetY: number }
  | { type: 'chat'; text: string }
  | { type: 'pickup'; x: number; y: number }
  | { type: 'drop'; x: number; y: number; item: InventoryItem }
  | { type: 'idle' };

interface BehaviorContext {
  nearbyPlayers: Array<{ id: number; x: number; y: number; team: number }>;
  nearbyItems: Array<{ x: number; y: number; item: InventoryItem }>;
  isWalkable(x: number, y: number): boolean;
  isOccupied(x: number, y: number): boolean;
  hasLOS(x1: number, y1: number, x2: number, y2: number): boolean;
}
```

### Built-in Behaviors

1. **`WanderBehavior`** — picks a random adjacent walkable tile and moves there. Matches legacy `dweeby_move`.
2. **`PatrolBehavior`** — moves between waypoints (room corners or specified points).
3. **`ChaseBehavior`** — pathfinds toward nearest enemy player using Chebyshev steps. Falls back to wander if no target.
4. **`FleeBehavior`** — moves away from nearest player. Matches legacy `dodgy_move` (evasive).
5. **`StationaryBehavior`** — stays put but can still fire and chat.

Each behavior handler is a pure function of current state, making them independently testable.

### Behavior Composition: Single Primary + Implicit Overrides (Option A)

Each monster has **one primary movement behavior** (the `behavior.type` field). However, the aggression system and item system can **temporarily override** movement:

1. **Aggression override**: A `wander` monster with `combat.aggressive: true` will temporarily switch to chase behavior when it detects a player in `aggroRange`. When the target escapes `pursuitRange` or dies, it reverts to wandering. The override priority is hardcoded in `MonsterManager.onMoveTick()`.

2. **Item delivery override**: A monster carrying an item with `carry.deliverTo` set will pathfind toward its delivery point, overriding normal movement until the item is delivered.

**Override priority** (highest wins):
1. Flee (if `behavior.type === 'flee'` and threat nearby)
2. Aggression chase (if combat.aggressive and enemy in range)
3. Item delivery (if carrying an item with a delivery target)
4. Primary behavior (the configured `behavior.type`)

This produces composite behaviors from simple definitions:
- **Dweeb**: `wander` + `passive` = just wanders around
- **Guard**: `patrol` + `aggressive` = patrols waypoints, chases intruders, returns to patrol
- **Thief**: `flee` + `passive` + `pickup` = grabs items, runs from players, delivers loot home
- **Merchant**: `wander` + `none` + `chat` = wanders slowly and talks to nearby players

**Future upgrade path (Option B)**: If we need more complex compositions, the override priority list can be made data-driven (a `behaviors[]` array with priority/condition pairs in the monster definition). The individual behavior handlers stay unchanged — only the selection logic in `onMoveTick` needs to become table-driven instead of hardcoded.

### Extensibility via Registry

```typescript
const behaviorRegistry = new Map<string, () => BehaviorHandler>();
behaviorRegistry.set('wander', () => new WanderBehavior());
behaviorRegistry.set('patrol', () => new PatrolBehavior());
// To add new behavior: behaviorRegistry.set('ambush', () => new AmbushBehavior());
```

The `MonsterManager` runs each monster's AI on a `setInterval` tied to `behavior.moveInterval`. On each tick:
1. Build `BehaviorContext` from current game state
2. Evaluate override priority (aggression, item delivery)
3. Call the selected behavior's `onTick(monster, context)` to get an action
4. Execute the action (move, fire, chat, pickup, drop)
5. Update visibility for players

---

## 6. Protocol Additions

### New S2CMessage Types

```typescript
// Monster spawns or becomes visible
| {
    type: 'MONSTER_INFO';
    id: number;          // negative
    name: string;
    avatar: string;
    room: number;
    x: number;
    y: number;
    hp: number;
    maxHp: number;
    team: number;
    dead: boolean;
  }

// Monster moves (same room, LOS permitting)
| { type: 'MONSTER_LOCATION'; id: number; room: number; x: number; y: number }

// Monster leaves LOS or is removed
| { type: 'MONSTER_HIDDEN'; id: number }
```

### Reused Messages (no changes needed)

- **`MISSILE_START` / `MISSILE_END`** — monster firing uses the same missile system
- **`PLAYER_HIT`** — `victimId` can be negative (monster taking damage)
- **`PLAYER_HEALTH`** — `id` can be negative for monster health updates
- **`MESSAGE`** — `from` can be negative for monster chat messages

---

## 7. Item Interaction System

Three configurable item behaviors per monster definition:

### Drop on Death (`dropOnDeath`)
When a monster dies, items from `items.drops` (loot table with `chance` probability) and any `carriedItems` are scattered around the death location. Reuses existing `nearbyFreeTile` + `ITEM_ADDED` broadcast pattern.

### Pickup (`items.pickup`)
On each AI tick, if the monster has `pickup` config and `carriedItems.length < maxCarry`, and there is a floor item within `pickup.range` tiles matching the `types` filter, the behavior handler returns a `pickup` action. The MonsterManager:
1. Calls `session.removeFloorItem(room, x, y)` → broadcasts `ITEM_REMOVED`
2. Adds the item to `monster.carriedItems`

### Carry and Deliver (`items.carry`)
When `carry.deliverTo === "home"` and the monster returns to its home room at `homeX/homeY`, it drops all carried items via `session.addFloorItem`. This creates emergent gameplay: thieves steal your weapons and bring them to their lair.

---

## 8. Integration with Existing Systems

### GameSession Changes (minimal)

1. **Instantiate MonsterManager** in constructor, after `initRoomItems()`
2. **Expose methods** matching `MonsterSessionInterface` (most are already private methods that just need to become callable)
3. **Modify `findPlayerHitOnPath`** to also check for monsters on tiles (or add a parallel call)
4. **Modify `updateVisibilityOnMove`** to also update monster visibility
5. **Modify `onJoin`** to send `MONSTER_INFO` for visible monsters in starting room
6. **Add to `regenTick`** a call to `monsterManager.regenTick()` for monster HP regen
7. **Add to `destroy`** a call to `monsterManager.destroy()`
8. **Add to `resetWorldState`** a call to `monsterManager.reset()`

### Monster Visibility

MonsterManager maintains its own visibility map: `Map<playerId, Set<monsterId>>`. When a monster moves, it iterates players in the room and sends `MONSTER_INFO`/`MONSTER_LOCATION`/`MONSTER_HIDDEN` as appropriate. When a player moves, the session calls `monsterManager.updatePlayerVisibility(playerId)`.

### Monster Combat

When a monster fires, MonsterManager:
1. Calls `session.calcMissilePath()` from the monster's position
2. Checks for player hits along the path
3. Broadcasts `MISSILE_START` to the room
4. Schedules a timer for damage application + `MISSILE_END`

This mirrors the exact pattern in `onFireWeapon` but in a separate method.

### World Loading

`loadWorld` in `world.ts` gains an optional step: if `data.map.monsterfile` exists, load and parse the monster definitions JSON. The `monsterDefs` array is added to the returned `World` object. Defaults to `[]` if absent.

---

## 9. Client Changes

### `client/src/game.ts`
- Add `private monsters = new Map<number, RemoteMonster>()` alongside `otherPlayers`
- Wire up `MONSTER_INFO`, `MONSTER_LOCATION`, `MONSTER_HIDDEN` handlers
- In `PLAYER_HIT`, if `victimId < 0`, look up in `monsters` for visual effect
- In `MESSAGE`, if `from < 0`, display with monster name styling

### `client/src/renderer.ts`
- Render monsters from the `monsters` map using same sprite-drawing logic as other players
- Use a different indicator style (e.g., yellow corner brackets instead of red/green)

### `client/src/types.ts` and `client/src/network.ts`
- Add the three new S2C message types and callback handlers

---

## 10. Testing Strategy

### Unit Tests (`server/src/__tests__/behaviors.test.ts`)
- Test each behavior handler in isolation with mock `BehaviorContext`
- WanderBehavior: returns valid adjacent walkable moves, handles being boxed in
- ChaseBehavior: moves toward target, stops when adjacent
- FleeBehavior: moves away from threat
- PatrolBehavior: follows waypoints, switches to chase on aggro

### Unit Tests (`server/src/__tests__/monster.test.ts`)
- MonsterDef loading and validation
- Monster ID assignment (negative, decrementing)
- Monster damage and death
- Item drop on death

### Integration Tests (`server/src/__tests__/integration/monsters.test.ts`)
Following existing patterns with `buildTestWorld`, `MockWebSocket`, and `vi.useFakeTimers()`:

- **Spawn test**: World with monster spawns; session created; verify `MONSTER_INFO` sent to joining player
- **Movement test**: Advance fake timers past moveInterval; verify `MONSTER_LOCATION` sent
- **Combat test**: Aggressive monster fires at player; verify `MISSILE_START` and `PLAYER_HIT`
- **Player kills monster**: Player fires weapon at monster tile; verify damage and drops
- **Visibility test**: Monster behind wall not revealed; player moves to LOS; `MONSTER_INFO` sent
- **Chat test**: Monster with chat config; advance timer; verify `MESSAGE` with negative `from`
- **Item pickup test**: Thief monster near floor item; advance timer; verify `ITEM_REMOVED`
- **Respawn test**: Kill monster; advance past respawn delay; verify new `MONSTER_INFO`
- **Spawn rate test**: Kill monster; verify replacement spawns after `spawnRate` interval
- **Cleanup test**: `session.destroy()` clears all monster timers

---

## 11. Phased Implementation Plan

### Phase 1: Foundation (Monster Entity + Data Loading)
**Create:**
- `server/src/monster-types.ts` — all TypeScript interfaces
- `pipeline/out/data/monsters/default.json` — test monster definitions

**Modify:**
- `server/src/world.ts` — add `monsterDefs` to World, load monster file
- `server/src/protocol.ts` — add `MONSTER_INFO`, `MONSTER_LOCATION`, `MONSTER_HIDDEN`

**Deliverable**: World loading includes monster definitions. Protocol types exist. No runtime behavior yet.

### Phase 2: MonsterManager Core
**Create:**
- `server/src/monster-manager.ts` — spawn, destroy, damage, death, item drops
- `server/src/__tests__/integration/monsters.test.ts` — basic spawn and death tests

**Modify:**
- `server/src/session.ts` — instantiate MonsterManager, expose interface methods, send `MONSTER_INFO` on join

**Deliverable**: Monsters spawn in rooms and appear to clients as stationary entities. Players can kill them.

### Phase 3: AI Behaviors (Movement)
**Create:**
- `server/src/behaviors.ts` — WanderBehavior, FleeBehavior, StationaryBehavior
- `server/src/__tests__/behaviors.test.ts` — unit tests

**Modify:**
- `server/src/monster-manager.ts` — AI tick loop, visibility tracking, `MONSTER_LOCATION` broadcasting

**Deliverable**: Monsters wander around rooms. Players see them move.

### Phase 4: Monster Combat
**Modify:**
- `server/src/monster-manager.ts` — monster firing with weapon objects
- `server/src/session.ts` — player missiles also hit monsters
- `server/src/behaviors.ts` — add ChaseBehavior, PatrolBehavior

**Deliverable**: Full bidirectional combat between players and monsters.

### Phase 5: Item Interaction (Pickup/Carry/Deliver)
**Modify:**
- `server/src/monster-manager.ts` — pickup action, carry tracking, deliver logic
- `server/src/behaviors.ts` — item-seeking behavior when pickup config exists

**Deliverable**: Thief monsters steal items and move them.

### Phase 6: Monster Chat + Respawn System
**Modify:**
- `server/src/monster-manager.ts` — chat timers, respawn timers, spawn-rate replenishment

**Deliverable**: Chatty monsters talk. Dead monsters respawn on schedule.

### Phase 7: Client Integration
**Modify:**
- `client/src/types.ts` — MonsterInfo type
- `client/src/network.ts` — new message handlers
- `client/src/game.ts` — monsters map, rendering, negative ID handling
- `client/src/renderer.ts` — monster rendering with distinct indicator

**Deliverable**: Full end-to-end working monster system.

### Phase 8: Map Authoring + Polish
- Create monster definitions for existing maps
- Add monster spawn configs to map JSONs
- Balance stats (HP, damage, speed, spawn rates)
- Add monster avatar sprites

---

## Critical Files Reference

| File | Action | Purpose |
|------|--------|---------|
| `server/src/session.ts` | Modify | Expose interface, integrate hit detection, instantiate MonsterManager |
| `server/src/world.ts` | Modify | Load monster definitions, extend World/RoomData interfaces |
| `server/src/protocol.ts` | Modify | Add MONSTER_INFO, MONSTER_LOCATION, MONSTER_HIDDEN |
| `server/src/monster-types.ts` | Create | All monster TypeScript interfaces |
| `server/src/monster-manager.ts` | Create | Central monster coordinator |
| `server/src/behaviors.ts` | Create | AI behavior handlers (Strategy pattern) |
| `pipeline/out/data/monsters/default.json` | Create | Monster definition data |
| `client/src/game.ts` | Modify | Monster rendering and protocol handling |
| `client/src/renderer.ts` | Modify | Monster visual indicators |
| `server/src/__tests__/integration/helpers.ts` | Modify | Add buildMonsterTestWorld |
