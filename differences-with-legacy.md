# Differences with Legacy: Griljor Modern Rewrite vs Original C Game

This document catalogues every meaningful difference between the legacy C game (`legacy/src/`) and the modern TypeScript web rewrite (`server/` + `client/`). It is organized by gameplay system and covers only items that might be worth implementing — things already decided against (XP, mana/power, AI monsters, persistent profiles) are omitted.

---

## 1. Movement

### 1.1 Diagonal movement
- **Legacy**: Full 8-directional movement using a 3×3 key grid (`qwe`/`asd`/`zxc`). The center key `s` triggers random movement to a random tile. `diag_movement_list()` generates paths using all 8 directions.
- **Modern**: 4-directional cardinal movement only (arrow keys + click-to-move). No diagonals.
- **Note**: The modern rewrite already has its own hotkey layout defined, and `s` has been repurposed as the pickup key. No further changes to directional hotkeys are planned.

### 1.2 Speed penalty when damaged
- **Legacy** (`movement.c`): When a player is hurt, their movement speed is scaled proportionally:
  ```c
  movewait = movewait * max_health / max(current_health, max_health / 4)
  ```
  A player at 25% health moves at 1/4 speed. They never slow below 25% of full speed.
- **Modern**: No speed penalty. Players always move at the tile's full speed regardless of HP.

### 1.3 Ignore-walls mode (deity movement)
- **Legacy**: `gameperson[num]->ignore_walls` flag allows walking through impassable tiles. Used by deity players and potentially by certain object effects.
- **Modern**: Not implemented. All players obey normal wall collisions.

### 1.4 Exit tile timing delay
- **Legacy**: Exits (stairs/portals) can have a `zinger` value in their `RecordedObj` record specifying a delay (in game clicks) before the exit whisks you away. `-1` means "use the movement speed." This allows trap doors you step on and then escape from, or slow-acting teleporters.
- **Modern**: Exit transitions are immediate — no delay mechanic.

### 1.5 Random movement key
- **Legacy**: Pressing `s` (center of the 3×3 grid) moves to a random tile in the room.
- **Modern**: Not implemented.

---

## 2. Combat / Weapons

### 2.1 Fire rate limiting
- **Legacy**: Each player has a `fire_again` timer (`person[num]->fire_again`) — the game click at which they may next fire. The base cooldown is `FIRING_WAIT_CLICKS = 850ms`. Each weapon has a `refire` adjustment field (−5 to +5) that speeds or slows the cooldown: faster weapons have negative values, slower weapons have positive ones.
- **Modern**: No fire rate limiting. Players can fire on every click.

### 2.2 Weapon spread / arc / fan (multi-missile)
- **Legacy**: Weapons can have `arc`, `spread`, and `fan` fields:
  - `arc`: angular width of the spread (0 = straight line or 360°)
  - `spread`: number of missiles fired across the arc (0 = as many as needed)
  - `fan`: random degree of fanning added to each missile
  This enables shotguns, wide-arc blasters, and 360° explosions.
  The `MULTI_FIRE` packet type broadcasts all missiles at once.
- **Modern**: Only single-projectile weapons. No spread or arc.

### 2.3 Mowdown missiles (piercing through people)
- **Legacy**: Missiles with `mowdown = TRUE` pass through hit players and continue on, wounding multiple targets in a line.
- **Modern**: Missiles stop when they hit a player.

### 2.4 Explosions and chain reactions
- **Legacy**: Weapons/missiles can have:
  - `explodes`: non-zero means the projectile spawns an explosion object when it expires
  - `boombit`: the object type spawned as the explosion
  - `flammable`: non-zero means this *wall/floor object* is a secondary explosive triggered by certain missiles, with `flammable` as the blast radius
  - `vulnerable`: walls/objects that can be destroyed by missiles whose `destroys` bitmask matches their type
  - `destroyed`: the object type this becomes when destroyed (e.g., rubble)
- **Modern**: No explosion system. Damage is applied directly; walls cannot be destroyed.

### 2.5 Missile wall-piercing / wall-destroying
- **Legacy**: Missiles have:
  - `ignoring`: bitmask of wall types the missile completely ignores (passes through)
  - `piercing`: bitmask of wall types the missile may pass through if they are also `vulnerable`
  - `destroys`: bitmask of wall types this missile destroys on contact
- **Modern**: All missiles stop at any non-permeable tile. No wall destruction.

### 2.6 Friendly fire / `hurts_owner`
- **Legacy**: Each missile has a `hurts_owner` flag. Most weapons set this to FALSE so you can't blow yourself up. Some (grenades?) may set it TRUE.
- **Modern**: No friendly fire protection implemented. No `hurts_owner` check.

### 2.7 Hit markers
- **Legacy**: When a missile hits, a visible hit marker appears on-screen for `HIT_LINGER_CLICKS = 200ms` before being erased.
- **Modern**: Not implemented (listed as a todo item).

### 2.8 Weapon `stop` flag (direction vs. destination)
- **Legacy**: `stop = TRUE` means the missile travels toward the target tile and stops there. `stop = FALSE` uses the target only as a direction and the missile continues past it to its range limit.
- **Modern**: Always behaves as if `stop = TRUE` — missiles always travel toward target and stop at range or on impact. No always-direction mode.

---

## 3. Inventory and Item System

### 3.1 Ammo / reload system
- **Legacy**: Ammo objects have a `charges` field that is a bitmask of weapon types they reload. The functions `remove_with_reload()` and `types_match()` handle finding a compatible ammo item in inventory and loading it into the weapon. For example, arrows reload a bow when the quiver is in inventory.
- **Modern**: Numbered items track charges and decrement on fire/use. But there is no reload-from-ammo-item mechanic — when a weapon runs out it is gone. The bow disappearing when empty is noted as a bug in todo.md.

### 3.2 Wearable items
- **Legacy**: Objects have a `wearable` bitfield that acts as a "slot" mask (e.g., one value for helmets, another for armor). You can only wear one item per slot. Worn items provide passive effects via their object fields.
- **Modern**: Not implemented. No wear slots; items are either in hand or inventory with no distinction for "worn" gear.

### 3.3 Container objects (items inside items)
- **Legacy**: Every `ObjectInstance` has `contained_id`, `container_id`, `lsibling_id`, `rsibling_id` fields forming a sibling-linked container tree. Chests, bags, or any object can contain other objects. `important` flag marks objects the driver tracks (flags, key items).
- **Modern**: No container system. Items are flat — each is either in a slot or on the floor.

### 3.4 Born-with items
- **Legacy**: Object definitions have a `bornwith` field: the number of copies of this item you receive when you join the game, subject to `level`, `class`, `team`, and `deity` requirements. This is how the game could give certain players starting equipment.
- **Modern**: No born-with items. Players start empty-handed; the only starting items come from walking over them on the map.

### 3.5 Cursed / undroppable / unswitchable items
- **Legacy**: Objects can be:
  - `unremoveable`: cannot take off once worn
  - `undroppable`: cannot drop (except on death or via a special dropper item)
  - `unswitchable`: cannot remove from hand once placed there
  - `nodeathdrop`: not dropped when you die (often used with `bornwith` + `undroppable`)
- **Modern**: No such restrictions. All items can always be dropped, swapped, or removed.

### 3.6 Secret items
- **Legacy**: `secret = TRUE` means the item appears in your own inventory but is not shown to other players when they inspect you.
- **Modern**: Not implemented. (Other players currently can't inspect inventories anyway — but the concept of hidden items is absent.)

### 3.7 Two-hand requirements
- **Legacy**: Objects with `needanother = TRUE` require a specific item (given by `otherobj`) in the other hand before they can be used. For example, a shield that requires a specific armor in the off-hand.
- **Modern**: No such requirement. Items can always be used with anything (or nothing) in the other hand.

### 3.8 Object breakage
- **Legacy**: Objects have a `breaks` field (chance per use in 100ths of a percent) and a `broken` field (type it becomes when it breaks). Heavy use can degrade items.
- **Modern**: No breakage system. Items last forever until charges run out.

### 3.9 Object ID matching for doors/locks
- **Legacy**: Keys and locks can have `id = TRUE`, which means the key's `x` slot in its `RecordedObj` record must match the door's. Two different keys with the same type but different `id` values will not work on the same door. Enables distinct locks.
- **Modern**: Matching is purely by the `opens` bitfield vs. the door's `type`. Any key of the right type opens any door of the matching type — no per-instance ID matching.

### 3.10 Weight limit scaling with level
- **Legacy** (`level.h`): `max_burden = MAX_BURDEN + (level - 1) * BURDEN_PER_LEVEL` = 150 + 10 per level. At level 5, you can carry 190.
- **Modern**: Fixed at 150 for all players (no levels). Noted bug: burden bar doesn't reset properly.

### 3.11 Item use restrictions by player count
- **Legacy**: `minplayers` / `maxplayers` fields prevent item use unless the right number of players are in the game. This allows "powerful items only when it's a big game."
- **Modern**: No player count restrictions on items.

### 3.12 Item use restricted by team representation
- **Legacy**: `represented` bitfield — teams that must be in the game for this item to work. `teams` bitfield — which teams are allowed to use this item at all.
- **Modern**: No team restrictions on item use.

### 3.13 Item use restricted to certain rooms
- **Legacy**: `rooms` bitfield — which teams' rooms this item can be used in. A team-specific weapon that only works in neutral rooms, for example.
- **Modern**: Items work in any room.

---

## 4. Room & Map Features

### 4.1 Room lighting: dark, daylit, day/night cycle
- **Legacy** (`map.h`): Rooms have a `dark` field with four values:
  - `DARK (0)`: always dark — you see only your flashlight radius
  - `DAYLIT (1)`: lit during daytime, dark at night (day/night cycle)
  - `LIT (2)`: always fully lit
  - `GLOBAL (3)`: you see the whole room, but only players in line-of-missile-fire are visible
  Rooms with `cycles = TRUE` toggle between daylit and dark based on in-game time.
- **Modern**: All rooms are always fully lit. No lighting simulation.

### 4.2 Flashlight objects
- **Legacy**: Objects have a `flashlight` field (non-zero = radius in tiles it illuminates for the bearer). Carrying a torch or lantern lets you see in dark rooms. `OWN_LIGHT(pnum)` macro returns 6 tiles as the player's innate light radius in dark rooms.
- **Modern**: No dark rooms, so no flashlight mechanic.

### 4.3 Glowing objects
- **Legacy**: `glows = TRUE` — this object emits its own light and is visible in dark rooms even without a flashlight.
- **Modern**: Not applicable (no dark rooms).

### 4.4 Invisible floor objects
- **Legacy**: `invisible = TRUE` — object is not rendered when on the floor or flying through the air, but is visible once picked up and in your inventory. Useful for hidden traps or secret items.
- **Modern**: Not implemented.

### 4.5 Line-of-sight visibility (`limited_sight`, `transparent`, `concealing`)
- **Legacy**: Each room can have `limited_sight = TRUE`, enabling raycasting visibility — you only see tiles your character has line-of-sight to. Objects with `transparent = FALSE` block sight; objects with `concealing = TRUE` hide the player standing on them from other players' sight.
- **Modern**: No line-of-sight system. The whole room is always visible. `transparent` and `concealing` fields exist in the JSON but are not used.

### 4.6 GLOBAL room type
- **Legacy**: `dark = GLOBAL` rooms show the entire room map to everyone, but you can only see *players* who are in your direct line of missile fire (not hidden by walls). This is a special fog-of-war mode for open battlefields.
- **Modern**: Not implemented.

### 4.7 Multi-room missile travel
- **Legacy**: Missiles can travel across room boundaries. `new_room_for_refire()` in `map.h` handles a missile crossing a room edge and continuing into the adjacent room.
- **Modern**: Missiles are confined to the room they were fired in.

### 4.8 Object-override system
- **Legacy**: Objects can override each other's properties via `overridden` and `overrider` bitmask fields plus three effect flags:
  - `ignoremove`: overriding object causes the overridden object's movement flag to be ignored (you walk over it as if it weren't there)
  - `ignoretrans`: overrides the target's transparency for sight
  - `ignoreperm`: overrides the target's permeability for missile fire
  This allows "boots that let you walk on lava" or "smoke screen that overrides wall visibility."
- **Modern**: Not implemented. Object fields are read directly with no override mechanism.

### 4.5 Room `objects_appear` / `people_appear` flags
- **Legacy**: Each room has `objects_appear` and `people_appear` boolean flags. If `objects_appear = FALSE`, the random item placement system never drops items there. If `people_appear = FALSE`, the respawn system skips this room.
- **Modern**: Not implemented. Every room is eligible for item drops and spawning (aside from the void-tile and castle key-room workarounds).

---

## 5. Win Conditions & Game Lifecycle

### 5.1 Capture-the-flag win condition
- **Legacy**: Objects can have `flag = TRUE` and `flagteams` as a bitmask of which teams need to hold this flag to win. The `important` flag tells the driver to track these objects. When win conditions are met, `GAME_OVER` packets are sent and the driver enters a countdown.
- **Modern**: No win condition. Games run indefinitely until the server resets.

### 5.2 Game timer / end of game
- **Legacy**: `end_of_world` counter — driver counts down and broadcasts `GAME_OVER`. After `GAME_OVER_WAIT = 12000ms` the game ends. A linger period (`GAME_OVER_LINGER = 5000ms`) keeps the driver alive briefly after the game clock expires. Rewards distributed.
- **Modern**: No time limit. No formal end-of-game state.

### 5.3 Game rewards / win bonuses
- **Legacy** (`level.h`): On game end:
  - `BASE_GAME_REWARD = 1000 XP` just for participating, scaled by time in game vs `REQUIRED_TIME = 2400s` and player count vs `REQUIRED_PEOPLE = 10`
  - `BASE_TEAM_REWARD = 1000 XP` for the winning team
  - `BASE_WINNER_REWARD = 1000 XP` for the player who triggered the win
- **Modern**: No such rewards (no XP system, and no win condition). Kill stats show in the player list but serve no purpose beyond bragging rights.

### 5.4 AFK / idle timeout
- **Legacy**: `NO_IO_CUTOUT_CLICKS = 1,800,000ms = 30 minutes`. If no IO is received from a client for 30 minutes, they are disconnected.
- **Modern**: Not implemented. Browsers can stay connected indefinitely (noted as a todo item).

### 5.5 `SAVE_STATS` packet
- **Legacy**: Players can request their stats be saved to the password file during a game. On normal exit the driver saves stats automatically.
- **Modern**: No persistence; nothing to save.

---

## 6. Player System

### 6.1 Multiple persons per player connection
- **Legacy**: `MAX_PLAYER_SIZE = 5`. One player process can run up to 5 persons (characters) simultaneously, each with their own position, health, and inventory. A player could control a team of characters.
- **Modern**: One character per WebSocket connection.

### 6.2 Rank / title system
- **Legacy** (`level.h`): 18 rank names from "rank beginner" to "God of War" plus "Deity." Rank is computed from a weighted formula: 35% level, 30% kill/death ratio, 15% team balance, 20% games won. Minimum 20 kills to be ranked. Displayed in player list and lobby.
- **Modern**: No rank. Player list shows kills/deaths only.

### 6.3 Neutral team
- **Legacy**: Maps have a `neutrals_allowed` flag. Team 0 is "neutral" — players on this team fight for no faction. Neutral team has its own bit in item `teams` and `represented` fields.
- **Modern**: Not implemented. Team 0 is currently effectively "no team."

### 6.4 Deity mode
- **Legacy**: `deity` flag on `GamePerson`. Deity players can use items with `deity = TRUE` in their definition and have special privileges (ignore walls, etc.).
- **Modern**: Not implemented.

### 6.5 `listed` flag (hidden players)
- **Legacy**: `listed` flag on a person — if FALSE, the player does not appear in the game list or the in-game player panel. Used for monsters and hidden accounts.
- **Modern**: All players appear in the player list.

### 6.6 Health regeneration
- **Legacy** (`level.h`): Players regenerate 1 HP every `REVIVAL_TIME = 1000ms` (i.e., 1 HP per second). Higher levels regenerate faster: `1000 / (1 + level * REVIVE_BONUS_TIME)`. At level 5 you regen 5× faster. Between fights, HP recovers automatically.
- **Modern**: No regeneration. HP stays at whatever it was until you die and respawn at 100.

### 6.7 Player-to-player messaging (`EXECUTE_COMMAND`)
- **Legacy**: `EXECUTE_COMMAND` packet — one player can send a control override to another player's process. Used for deity commands and perhaps forced movement.
- **Modern**: No such mechanism.

### 6.8 `REPEAT_INFO` / resync request
- **Legacy**: If a client misses a player's state, they can send `REPEAT_INFO` to request that player re-broadcast their full state. Essential for UDP loss recovery.
- **Modern**: WebSocket is reliable (TCP); no need for manual resync. The server pushes full state on join and rebroadcasts on relevant events.

---

## 7. Chat and Messaging

### 7.1 Object use/look messages
- **Legacy**: Every object can have rich text messages:
  - `lookmsg`: shown when you look at the object from afar
  - `examinemsg`: shown when examining an object in your possession
  - `usemsg`: shown to the user when they use the object
  - `roommsg`: broadcast to others in the same room
  - `allmsg`: broadcast to all other players (if no `roommsg` for those in room)
  - `hitmsg`: shown to attacker when missile hits someone
  - `victimmsg`: shown to the player being hit
- **Modern**: No object-specific messages. A single hardcoded "You hit X for Y damage" / "X hit you for Y damage" report is sent. No look/examine messages.

### 7.2 GM message logging
- **Legacy**: Messages sent `TO_GM (-2)` are logged to a file (`GM_FILE`). The GM can observe game events. `GM_ACTIVE = 1` controls this. The `gm.text` personality file drives the Burt NPC responses.
- **Modern**: Server logs significant events to stdout, but there is no GM file or GM-addressed message routing.

### 7.3 News file at startup
- **Legacy**: A `news` file is displayed to the player at startup before entering a game.
- **Modern**: Not implemented.

### 7.4 Help system
- **Legacy**: Five separate help files: `player.help`, `game.help`, `editor.help`, `host.help`, `obtor.help`. Players can open help windows in-game.
- **Modern**: Not implemented (noted as todo item).

---

## 8. Periodic Item Placement (`.pla` files)

- **Legacy**: Each map can have a `.pla` placement file (plain text script) that drives random item drops during the game. Format:
  - `s <seconds>` — interval between cycles
  - `t <objnum> <quantity> <team>` — drop into a random room owned by that team (0 = any)
  - `r <objnum> <quantity> <roomnum>` — drop into a specific room
  Many maps have these (battle, castle, flames, hack1, hometown, ivarr, outdoor, paradise3, ring, shelter, shooter, three, trek, tunnel, etc.)
- **Modern**: Not implemented. The architecture is described in `modern-rewrite-plan.md` as a stretch goal. The server would parse the `.pla` file (or JSON equivalent) at game start, set a repeating timer, and use the existing `ITEM_ADDED` broadcast path.

---

## 9. Team System (Partially Implemented)

### 9.1 Team selection UI
- **Legacy**: Players chose a team when joining. The game list entry showed team names and sizes.
- **Modern**: No team-selection UI. All players are hardcoded to team 1. (Todo item.)

### 9.2 Custom team names
- **Legacy**: Each map defines custom names for up to 4 teams (`team_name[NUM_OF_TEAMS+1]`).
- **Modern**: Teams are referred to by number only.

### 9.3 Friendly fire options
- **Legacy**: Design-level support via `hurts_owner` and team-restriction fields on weapons.
- **Modern**: No friendly fire detection. Todo item: "consider friendly fire options for teammates."

### 9.4 Team identification in player list / map
- **Legacy**: Players appeared colored by team in the player list and had team-colored displays.
- **Modern**: All player icons look the same on the map. Team not shown in player list. (Todo item.)

### 9.5 Maximum player cap
- **Legacy**: `MAX_GAME_SIZE = 20` persons hard-coded.
- **Modern**: No cap enforced server-side. (Bug noted in todo.md: "there probably isn't anything keeping a 17th player from joining a max 16 player game.")

---

## 10. Rendering Differences

### 10.1 Object rendering priority / stacking
- **Legacy**: Multiple objects can stack on the same tile. `most_prominant_on_square()` and `probable_floor_here()` determine which object is drawn. A specific draw order handles which layers are visible.
- **Modern**: Tiles have exactly 2 slots: `[floor, wall]`. Floor is drawn first; wall is drawn on top. No multi-object stacking within a single layer slot.

### 10.2 Color mode rendering
- **Legacy**: X11 `XCopyPlane` operations, hardware color planes, pixel-level masked compositing using Pixmaps. Color comes from the `color` field in object definitions (X11 color name string).
- **Modern**: Canvas 2D with color mode toggle (dark/light). Grayscale PNGs tinted by drawing mode. The `color` field from object definitions is not used — all objects render as the global mode color.

### 10.3 Player "appearance" state
- **Legacy** (`person.h`): `appearance` field tracks player state: normal, dying, dead, invisible. The rendering code checks this to choose the right sprite or skip drawing the player.
- **Modern**: `dead: boolean` flag only. No intermediate appearance states; death is binary.

### 10.4 Room `object_floor` / `people_floor`
- **Legacy**: Each room defines a preferred floor tile for rendering objects and a preferred floor tile for rendering people. This allows rooms to have a "people" layer above the ground.
- **Modern**: Objects and players render on the same layer at fixed z-order.

---

## 11. Object Properties Not Checked

These fields exist in the parsed JSON objects (from the pipeline) but are never read or applied in the modern rewrite. They are noted here as potential features to add:

| Field | Legacy behavior |
|-------|----------------|
| `strength` | Minimum health percentage required to use this object |
| `experience` | XP cost/gain per use (negative = gain) |
| `class` | Bitmask of player classes that can use this object |
| `scalebits` | Map scaling modes in which object works |
| `connectgroup` / `connectbits` | Wall connection groups for editor random placement; affects rendering of connected walls |
| `concealing` | Standing here hides you from others' sight |
| `transparent` | Whether this tile blocks line-of-sight |
| `glows` | Visible in dark rooms |
| `invisible` | Not visible on floor, only in inventory |
| `magic` | Where magic property IDs are stored |
| `def[]` / `set[]` | Default initialization values for object instances |
| `varspace` | Forth code local variable space allocation |
| `object1/2/3` | Object references used in Forth code |

---

## 12. Input / Control Differences

| Action | Legacy | Modern |
|--------|--------|--------|
| Move direction | 9-key grid (diagonals + random) | Arrow keys + click-to-move (4-direction) |
| Fire direction | Left/right click anywhere; also queued via `future_hand/x/y` | Left/right click target tile |
| Look at object | Dedicated look command → `lookmsg` | No look command |
| Examine inventory item | Dedicated examine → `examinemsg` | Hover tooltip only |
| Navigate N/S/E/W | Room edge walk-off | Room edge walk-off + navigation buttons |
| Open help | Dedicated help window | Not implemented |
| GM message | `TO_GM` addressed chat | Not implemented |
| Random movement | `s` key | Not implemented |

---

## 13. Bugs in Play (from todo.md that reflect Legacy Differences)

The following items in `todo.md` represent mechanics from the legacy game that are partially or incorrectly implemented in the modern rewrite:

- **Ammo consumption**: Numbered weapon items should consume charges on fire — not fully working
- **Grenade depletion**: Grenades don't get depleted from inventory properly as fired
- **Bow behavior**: Bow disappears when empty; quiver reload doesn't work
- **Punching**: Getting bare-hand melee combat to work (no weapon equipped)
- **Eating food**: Food consumables don't restore health properly / health bar doesn't update
- **Proximity pickup**: Player should only be able to pick up items that are nearby, not across the room
- **Spawn-in-locked-room escape**: No way to escape if you spawn in a room you can't get out of without a key
- **Burden bar**: Doesn't reset to zero, may not count hand-slot items correctly
- **Hit markers**: Visual feedback when a missile hits someone — not shown yet

---

## 14. Legacy Assets and Files Not Yet Used

This section covers files in `legacy/` outside of `legacy/src/` that exist and could be useful but are not currently used in the modern rewrite.

### 14.1 Status indicator bitmaps (`legacy/bit/`)
Several bitmaps correspond to gameplay states that aren't yet implemented:

| File | Purpose |
|------|---------|
| `hit` | Hit marker shown momentarily when a missile connects (in todo as unimplemented) |
| `winner` / `loser` | End-of-game result graphics (no win condition yet) |
| `friend` | Indicator for friendly/same-team players |
| `invis` | Indicator for invisible players |
| `box` | Player outline/border box (in todo: "need border boxes around other players") |
| `wait` | Waiting state indicator |
| `intro1.b`, `intro2.b`, `intro3.b` | Intro/splash screen artwork (not used in modern title screen) |
| `logo` | Original game logo bitmap (modern title screen uses canvas-drawn letters instead) |
| `modemask` | Light/dark mode toggle mask |

### 14.2 River tile bitmaps (`legacy/bitmaps/`)
`river0` through `river20` — 18 directional/animated river connection frames. The original used these to render rivers based on which adjacent tiles connected. The modern rewrite instead uses a dilation algorithm to *infer* the river body from sparse ripple marks. These directional tiles exist if a proper connected-river-rendering system is ever implemented.

Path tiles (`hpath`, `vpath`, `llpath`, `lrpath`, `ulpath`, `urpath`) are also available for directional path rendering if needed.

### 14.3 Maps not yet converted (`legacy/lib/map/`)
The pipeline converted only a subset of the original maps. Still available as binary `.map` files:

| Map | Description |
|-----|-------------|
| `flag` | Swamp/forest map |
| `hometown` | Grenades-focused |
| `sword` | 3 rooms, swords only |
| `twoperson` | 4 rooms, machine guns |
| `paradise2` | Huge single-team free-for-all |
| `hack` / `hack1` | Dark-room maps (require lighting system to be useful) |
| `flames`, `flash`, `ring`, `shelter`, `shooter`, `three`, `trek`, `tunnel`, `two`, `ivarr`, `outdoor` | Various unported maps |

Maps referenced in `maplist` but with no `.map` file found (may be lost or user-created): `superiority`, `spindizzy`, `schwappo`, `efficiency`, `grenadewars` (3 variants), `berzerkely`, `smashtv`, `mapland`, `megarator`, `twolevel`.

### 14.4 Burt NPC system (`legacy/burt/`)
Five C source files (`main.c`, `io.c`, `load.c`, `person.c`, `response.c`) implementing the full Eliza-style NPC. The system is self-contained and can run standalone (`testburt`) or be embedded. `load.c` reads `gm.text`, `response.c` does keyword matching and substitution. The logic is straightforward and would port easily to TypeScript.

- **`legacy/lib/gm.text`** (780 lines): Burt's full personality script — keyword matching rules, 150+ response phrases, creator bios, how-to-play explanations. Ready to use.
- **`legacy/burt/speech.text`** (8.3K): Alternative personality data for testing.
- **`legacy/lib/messages`** (408 lines): Actual Burt conversation logs from 1990–1991 showing how players talked to him and what he replied — good reference for tone and personality.

The `todo.md` note says "add Burt back somehow, maybe to respond to chats if you address him directly." All the source and data to do this is here.

### 14.5 Categorized object data files (`legacy/lib/obj/*.bin`)
Pre-categorized binary slices of the object definitions: `exits`, `flags`, `floors`, `foliage`, `magic`, `objects`, `terrain`, `walls`, `weapons`. The modern pipeline reads the full `.obj` binary directly and doesn't use these, but they are a useful reference for what object categories exist and which object IDs fall into each category.

### 14.6 Documentation worth reading (`legacy/doc/`)
| File | Contents | Why useful |
|------|----------|------------|
| `fields` (22K) | Detailed spec of every object field | Most complete reference for object behaviors beyond the header files |
| `griljor.doc` (7.9K) | Player-facing game instructions | Source material for writing a help/tutorial page |
| `global.doc` (6.9K) | Global game mechanics | Reference for understanding mechanics not obvious from code |
| `Wishlist` (1.8K) | Original developers' feature wishlist | Interesting to compare against current todo.md |
| `magic.doc` (195B) | Notes on the magic system | Reference if magic items are ever implemented |
| `spell.ideas` (252B) | Spell system brainstorming | Historical context |

### 14.7 Utility tools (`legacy/util/`)
None of these are ported or needed for the modern rewrite, but some are relevant as reference:

| Tool | Purpose | Status |
|------|---------|--------|
| `bmconvert/` | Bitmap format conversion | Superseded by Python pipeline |
| `mask/` | Auto-generate masks from bitmaps | Superseded by Python pipeline |
| `obvert/` | Object definition converter | Could help if re-running pipeline against new `.obj` formats |
| `rotate/` | Rotate/flip bitmaps | Useful if directional sprite variants are needed |
| `textbit/` | Render text as a bitmap | Not needed for web |
| `headers/` | Code generators for `plot_order.h`, `circle.h`, `rulepack.h` | These define rendering draw order and raycasting tables — relevant if line-of-sight is ever implemented |

### 14.8 Design specification (`legacy/design/overview`)
Griljor Design Spec v1.0 (1995 revision). Describes the planned redesign as an object-oriented multiplayer network game support system. Has architectural context for understanding why certain fields exist in the object definitions and what the original team intended the system to eventually become.

---

## Summary: Highest-Impact Missing Features

Ranked roughly by how much they would affect gameplay feel:

1. **Fire rate limiting** — currently you can fire infinitely fast; all weapons should have an 850ms cooldown (adjustable per weapon via `refire` field)
2. **Health regeneration** — in the original, players slowly heal between fights (1 HP/sec); without it, the only way to restore health is consumables
3. **Periodic item placement (`.pla` files)** — weapons and items respawn on a timer; the game gets very stale without this
4. **Team UI + friendly fire** — teams exist in data but have no visible UI or enforcement
5. **Win conditions** — no game ever ends; there's nothing to play toward
6. **Diagonal movement** — original game feels very different with 8-direction movement
7. **Speed penalty when hurt** — creates tactical depth (a badly wounded player is also slow)
8. **Dark rooms + flashlight** — several maps have dark rooms; the castle, outdoor, and hack maps use lighting as a core mechanic
9. **Wearable items** — armor, rings, and clothing exist in item definitions but cannot be worn
10. **Multi-missile weapons (spread fire)** — shotguns and area-effect weapons fire multiple projectiles; this is a major weapon category
11. **Weapon spread/arc** — affects how grenades, shotguns, and magic staves feel to use
12. **Explosion system** — grenades and certain weapons are supposed to spawn explosion objects that deal area damage; currently they just deal damage directly
13. **AFK timeout** — players who leave their browser open hold server slots indefinitely
14. **Ammo reload from quiver** — bows currently can't reload from arrows in inventory
15. **Object look/examine messages** — many items have flavor text that provides hints and atmosphere; currently nothing is shown except a tooltip name
