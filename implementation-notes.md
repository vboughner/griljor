# Griljor Web Rewrite — Implementation Notes: Phases 1 & 2

This document records what was actually built, the decisions made, and
the technical details discovered along the way.

---

## Phase 1 — Asset Pipeline

**Goal**: Extract all game assets from the original binary format and
produce web-friendly files in `pipeline/out/`.

### Scripts

| Script | Purpose |
|--------|---------|
| `pipeline/xbm_to_png.py` | Convert raw 1-bit XBM bitmaps to 8-bit grayscale PNGs |
| `pipeline/parse_objs.py` | Parse binary `.obj` files → `data/objects/*.json` |
| `pipeline/parse_maps.py` | Parse binary `.map` files → `data/maps/*.json` |
| `pipeline/run_pipeline.sh` | Run all three scripts in order |

### Bitmap Conversion (`xbm_to_png.py`)

The original bitmaps are raw binary files (no file header, no extension)
`#include`d directly into C source. Each file encodes a 32×32 grid of
1-bit pixels in XBM byte-reversed order (LSB-first within each byte).

The conversion:
1. Read raw bytes
2. Reverse bit order within each byte (XBM convention)
3. Map bit 0 → black (255 after inversion for display), bit 1 → white (0)
4. Write as 8-bit grayscale PNG via Pillow

Three source directories were processed:
- `bitmaps/` + `bit/` → `data/objects/bitmaps/{objset}/` (terrain/object tiles)
- `facebits/` → `sprites/facebits/` (player avatar pairs: `*bit` + `*mask`)

### Object Definition Parsing (`parse_objs.py`)

`.obj` files are a fixed-size binary record array. Each record encodes
object properties including name, flags (permeable, transparent, masked,
exit, takeable, etc.), bitmap filename references, and combat stats.

Output: `data/objects/{name}.json` with an `objects` array indexed by
object ID (0–254).

Key fields preserved:
- `bitmap` / `mask` — PNG filenames for the tile sprites
- `masked` — whether the mask file must be applied
- `permeable` — whether missiles can pass through (NOT player movement — see Phase 6)
- `exit` — whether stepping on the tile triggers a room transition
- `movement` — movement cost (used for passability classification)

### Map Parsing (`parse_maps.py`)

`.map` files contain a header (map name, linked object file, team info)
followed by a fixed array of room records. Each room has:
- Name, floor, team assignment
- Four directional exits (`exit_north/east/south/west` = room index, or -1)
- A 20×20 grid of tile pairs: `spot[x][y] = [floor_id, wall_id]`
- A list of `recorded_objects` (items/NPCs/exits placed in the room)

Output: `data/maps/{name}.json`

**26 maps** and **7 object sets** were extracted.

### Output Structure

```
pipeline/out/
├── data/
│   ├── maps/          — 26 JSON map files
│   └── objects/
│       ├── *.json     — 7 object set definitions (255 objects each)
│       └── bitmaps/
│           └── {objset}/   — 32×32 grayscale PNGs (bitmap + mask pairs)
└── sprites/
    └── facebits/      — 25 player avatar pairs (*bit.png + *mask.png)
```

---

## Phase 2 — Static Renderer

**Goal**: A browser-side TypeScript app that loads pipeline output,
renders a navigable map, and supports basic player movement.

### Stack

- **Vite 5** — dev server + bundler
- **TypeScript 5** — strict mode
- No runtime dependencies, no framework

Vite's `publicDir` is pointed at `../pipeline/out`, so all pipeline
output is served at `/data/...` and `/sprites/...` with no copying.

### File Structure

```
client/
├── index.html
├── package.json
├── vite.config.ts
├── tsconfig.json
└── src/
    ├── main.ts       — entry point, map/avatar/mode selectors
    ├── types.ts      — TypeScript interfaces for JSON shapes
    ├── assets.ts     — image loading, color mode, caching
    ├── renderer.ts   — Canvas 2D tile rendering
    └── game.ts       — player state, movement, collision, exits
```

### Bitmap Rendering (`assets.ts`)

All bitmaps are 8-bit grayscale PNGs where **white = background**
and **dark = drawn pixel**. Two rendering conventions are supported:

**Dark mode** (default): white → transparent (alpha 0), dark pixels
inverted (`255 - value`) so black becomes white — bright art on a
dark canvas background.

**Light mode**: white → transparent, dark pixels kept as-is —
dark art on a light canvas background, matching the original X11 look.

The image cache is keyed by `"{mode}:{url}"` so both modes can
coexist in memory. Switching modes uses cached data without re-fetching.

**Mask handling**: Object sprites with `masked: true` have a separate
mask PNG. Mask files are loaded raw (no color transform) and applied
after the bitmap is processed: mask pixels ≥ 200 (white) zero the
corresponding alpha channel, cutting the sprite to its correct shape.

`ImageData` objects are converted to `ImageBitmap` via a `WeakMap`
cache once per image — `ImageBitmap` can be drawn directly with
`ctx.drawImage()`, which is significantly faster than the
`putImageData` → `OffscreenCanvas` → `drawImage` chain.

### Tile Rendering (`renderer.ts`)

Canvas: 640×640 px (20×20 tiles × 32 px each).

**Background pre-render** (`buildRoomBackground`): on room entry or
mode change, all static content is drawn once to an `OffscreenCanvas`:
1. Background fill (#333 dark / #e8e8e8 light)
2. Floor layer (`spot[x][y][0]`)
3. Wall layer (`spot[x][y][1]`)
4. Recorded objects that have a `bitmap` field

**Per-frame composite** (`renderFrame`): synchronous, two draw calls:
1. `ctx.drawImage(offscreenBg, 0, 0)`
2. Player avatar bitmap at `(px * 32, py * 32)`

This makes movement rendering essentially instant — no tile iteration
on every keypress.

### Game Logic (`game.ts`)

**Player position** tracked as tile coordinates `(px, py)`, starting
at `(10, 10)` (room center).

**Movement**: arrow keys and N/S/E/W buttons move one tile per press.
Edge-of-room movement triggers directional exits (`exit_north` etc.)
when the exit index ≥ 0, placing the player at the opposite edge.

**Wall collision**: a tile is blocked if the wall-layer object has
`movement` absent or 0, or if the floor-layer object has `movement === 0`
explicitly. `permeable` controls missile passage only (see Phase 6).

**Exit tile teleportation**: `recorded_objects` with `exit: true` and
`detail ≥ 0` are "invisible exits" (object type 49). An exit map
(`Map<"x,y", {destRoom, landX, landY}>`) is built once per room.
Exit tiles are checked **before** collision — they underlie non-permeable
floor tiles (laserbolt, frap bolt) and would otherwise be unreachable.
The `detail` field is the destination room index; `infox`/`infoy` is
the landing tile.

### UI Features

| Feature | Implementation |
|---------|---------------|
| Map selector | Dropdown of all 26 maps; switching reloads map + objects |
| Avatar selector | Dropdown of all 25 facebits; hot-swaps sprite in place |
| Light/dark toggle | Invalidates background cache and re-renders |
| Arrow keys | `keydown` listener, `preventDefault` to suppress scroll |
| Room info | Shows room index, name, and current player coords |

### Key Discoveries During Implementation

**Exit tile floor objects**: The tiles at exit positions (stairs etc.)
happen to contain non-permeable objects (`laserbolt`, `frap bolt`) in
the floor layer — probably leftover from object indexing. Collision
must be checked *after* exit detection, not before.

**Mask inversion bug**: Initially, mask PNGs were passed through the
same color-inversion pipeline as bitmaps. This inverted the mask logic
(white ↔ black) and made masked sprites disappear. Fix: load mask
files raw with no color transforms.

**No minimap in original**: The original X11 client's `exitwin` is
simply a 1-tile-wide border (22×22 tiles) framing the 20×20 room view.
No bird's-eye map feature existed.

---

---

## Phase 5 — Inventory System

**Goal**: Server-authoritative inventory — 35 slots + 2 hand slots,
weight limits, floor items, pickup/drop, multiplayer sync.

### Data Model

`InventoryItem { type: number; quantity: number }` is the unit of
currency everywhere. `type` is an `ObjDef` array index; `quantity`
is 1 for non-numbered items and the charge count for `numbered` items
(e.g. a hand gun with 13 rounds).

Each player on the server holds:
- `leftHand / rightHand: InventoryItem | null`
- `inventory: Array<InventoryItem | null>` — 35 slots (`INV_SIZE = 35`)
- `currentWeight: number` — sum of `weight * quantity` for all held items
- `MAX_WEIGHT = 150` (flat limit, no leveling system)

Floor items live in `roomItems: Map<roomIdx, Map<"x,y", InventoryItem>>`,
initialized from each room's `recorded_objects` filtered by `obj.takeable`.

### New Protocol Messages

**C→S**: `PICKUP { x, y, hand }`, `DROP { source }`, `INV_SWAP { slot, hand }`

**S→C**: `ITEM_REMOVED { room, x, y }`, `ITEM_ADDED { room, x, y, item }`,
`YOUR_INVENTORY { leftHand, rightHand, inventory, currentWeight, maxWeight }`,
`ITEMS_SYNC { items[] }` (sent once to new joiners to replace local defaults)

`YOUR_INVENTORY` is only sent to the acting player. `ITEM_REMOVED` /
`ITEM_ADDED` are broadcast to all players.

### Server Handler Details (`session.ts`)

**PICKUP**: checks item exists at `(x,y)` in player's current room, that
weight won't exceed limit, and that the target hand or inventory has a free
slot. Places item in hand if empty, else first free inventory slot. Rejects
with a GM chat message if overweight or all slots full.

**DROP**: removes item from hand/slot, finds the nearest free tile via spiral
search (`nearbyFreeTile`), and places the item there. "Free" means: inside
the 20×20 grid, no existing floor item, and wall/floor objects both have
`permeable: true` (same passability rule as player movement). Radius ≤ 5;
items with nowhere to go are lost (logged server-side).

**INV_SWAP**: swaps a named hand slot with an inventory slot in-place.
No weight check needed — total weight doesn't change.

**onLeave**: iterates all non-null hand + inventory slots and drops each
item to the floor near the player's last position, broadcasting `ITEM_ADDED`
for each. This restores items to the world when a player disconnects.

### Client Side

**`renderer.ts`**: `buildRoomBackground` now skips `recorded_objects` where
`obj.takeable === true` (they are rendered dynamically). `renderFrame` gains
`floorItems`, `objects`, and `objset` parameters; floor items are drawn as
a layer between the background and players.

**`game.ts`**: `floorItems: Map<roomIdx, Map<"x,y", InventoryItem>>` is
initialized from the map file on startup and then kept in sync by the
network callbacks. `onItemsSync` replaces the local state entirely (server
is authoritative). Canvas left-click on a floor item tile → `sendPickup`
(right hand); middle-click → left hand. Right-click retains its original
"move toward" behavior.

**`main.ts`** / **`index.html`**: 35-slot CSS grid (`7 col × 5 row`,
34×34 px cells with 32×32 canvas children). Weight displayed as
`{current} / {max}`. Click on occupied slot → `sendDrop(slotIndex)`.
Right-click → `sendInvSwap(slot, 'right')`. Click on hand-slot canvases
→ `sendDrop('left'|'right')`. Item bitmaps drawn via `loadMaskedSprite`
/ `loadSprite`; charge count overlaid in yellow for `numbered` items.

### Key Edge Cases

- **Overweight**: server rejects pickup with GM chat "That is too heavy to carry."
- **All slots full**: server rejects with GM chat "Your hands are full."
- **Drop on occupied tile**: spiral search finds nearest free tile.
- **`detail = -1`** (e.g. keys without a set charge): quantity defaults to 1.
- **`ITEMS_SYNC` on join**: replaces client's map-file-derived floor items
  entirely, ensuring late joiners see the live world state.
- **Numbered items**: `quantity` is preserved through pickup/drop cycles;
  no stack splitting is implemented.

---

## Phase 6 — Client UI Refinements

### Avatar Sprite Masking

Player avatars use paired XBM files: `*bit` (face features) and `*mask`
(silhouette). Original `loadMaskedSprite` ran the bitmap through
`loadAndProcess` (which made all white pixels transparent) before
applying the mask. This caused interior skin pixels — white in the face
bitmap, black in the mask (inside silhouette) — to be made transparent,
letting floor tiles bleed through the hollow face.

**Fix**: `loadMaskedSprite` now uses `loadRaw` for the bitmap (no color
transform) and processes both layers in one pass:
- Outside mask (mask pixel ≥ 200) → alpha 0 (transparent)
- Inside mask + dark bitmap pixel → foreground color (255 dark mode, 0 light mode)
- Inside mask + light bitmap pixel → background color (0 dark mode, 255 light mode)

### Mouse Slot Mapping and Hand Icons

The original code had left/middle mouse buttons swapped relative to the
on-screen mouse graphic (left button activated the middle box). Fixed in
`game.ts`: `e.button === 0` = left hand, `e.button === 1` = right hand.

Hand slot canvases previously showed text labels. Fixed in
`mouse-widget.ts`: `setHandItem` now accepts `ImageData | null` and
renders the item sprite via `OffscreenCanvas → drawImage`.

### Inventory Cell Interactions

Changed from `click` to `mousedown` event (required for middle button
detection). Mapping:
- Left click on inv cell → swap with left hand
- Middle click on inv cell → swap with right hand
- Right click on inv cell → drop item (via `contextmenu`)

### Item Tooltips (`tooltip.ts`)

New file with `showTooltip(html, x, y)`, `hideTooltip()`,
`moveTooltip(x, y)`. The `#tooltip` div is fixed-position with
viewport-edge detection (flips left or above when near an edge).

`buildItemHtml(obj, item)` formats: name, weapon stats (damage/range/
speed), weight, capacity, movement modifier, quantity.

Wired up in `main.ts` via `mouseover` on inventory cells and hand
canvases. `currentLeftHand`, `currentRightHand`, `slotItems[]` track
what is in each slot for tooltip lookup.

### Clickable Map Border

Canvas expanded from 640×640 to 704×704 (22×22 tiles). A 1-tile blank
border (`BORDER = TILE = 32`) surrounds the 20×20 map area. All tile
draws in `renderer.ts` are offset by `BORDER`. `TILE` and `BORDER` are
exported constants.

`drawBorderIndicators(room)` draws filled triangle arrows at the center
of each border strip where a room exit exists (uses `getColorMode()` for
color). Border clicks call `startMovingTo(tx, ty)` with an off-grid
coordinate, which navigates the player to the nearest edge tile then
steps off.

### Click-to-Move (`game.ts`)

Right-click sets a move target and walks the player tile-by-tile at a
timed rate. Implementation:

- `computeBresenhamPath(x0,y0,x1,y1)` — pre-computes the tile sequence
  for a straight-line path (Bresenham algorithm).
- `startMovingTo(x, y)` — stops any existing movement, pre-computes
  path via Bresenham, schedules first step.
- `doMoveStep()` — advances one tile. If the next Bresenham tile is
  blocked, falls back to `findNextStep` (BFS) and recomputes the
  remainder of the path from the redirected position.
- `scheduleMoveStep()` — `setTimeout` chain (not `setInterval`) so
  delay can vary per step based on floor tile speed.
- `getMoveDelay()` — reads `movement` field from the floor tile at the
  player's current position, converts via `stepDelay(spd)`.
- `stepDelay(spd)` — `Math.max(50, Math.round(150 * 9 / spd))`.
  Speed 9 → 150 ms, speed 5 → 270 ms, absent movement → blocked.

Left/middle mouse clicks do **not** interrupt ongoing movement — they
only fire weapons or pick up items from where the player currently stands.

Keyboard movement rate-limited by `moveReadyAt` timestamp so keys can't
outrun click-to-move speed.

### Movement Blocking: `permeable` vs `movement`

**Original semantics** (confirmed from `src/objects.h` and
`src/mapfunc.c`):
- `permeable` — "May fire over": controls missile passage only.
  Labeled as such in the object editor (`objprops.c`).
- `movement` — controls player walkability: 0 = blocked (wall),
  > 0 = walkable at that speed.

**Pipeline output**: `BOLBOX` fields only appear in JSON when `true`.
Wall objects have neither `movement` nor `permeable` set (both default
to absent/false). Floor objects have `movement > 0` and `permeable: true`.

**Fixed `isTileBlocked`**:
- Wall layer: blocked unless `(movement ?? 0) > 0`
- Floor layer: blocked only if `movement === 0` explicitly
- `permeable` is no longer used for player movement anywhere in the client

The server's missile tracing in `session.ts` correctly uses `permeable`
for projectile passage — unchanged.

The `nearbyFreeTile` drop helper in `session.ts` still uses `permeable`
as its free-tile criterion (will be corrected to use `movement` in a
future pass).

### BFS Pathfinding

`findNextStep(sx, sy, tx, ty, room, objects)` uses BFS on the 20×20
grid with a `Uint8Array` visited set. Cardinal directions are tried
first (preferred for straight paths), then diagonals. Returns the
`[dx, dy]` of the first step toward the target, or `null` if no path
exists. Used as fallback when the Bresenham path is blocked.

### Tile Hover Debug Mode

`?` key toggles `hoverMode`. When active, canvas cursor changes to
crosshair and `mousemove` calls `buildTileHtml(tx, ty)` to show a
tooltip with:
- Floor tile name + speed (ms/step or "blocked")
- Wall tile name + [passable] / [blocks] (based on `movement`)
- Recorded objects at the tile with [spawn] for `takeable` items
- Dropped items (from live `floorItems`) with [dropped] label
- "empty" with default speed for bare tiles

The spawn/dropped distinction: `recorded_objects` with `takeable: true`
are item spawn points ([spawn]); items dropped by players appear via the
live `floorItems` map ([dropped]).

**Empty tile display**: bare tiles (no floor, wall, or recorded object)
show `floor: empty` and `speed: 150 ms/step` (speed 9, the open-air
default), formatted identically to tiles with named objects.

**Inventory item tooltips**: the `Move` line is omitted when movement = 9
(the default; not informative). Non-default walkable speeds are shown as
`N ms/step` (same formula as tile tooltips), not as a raw number.

### Border Click Bug Fix

The original border-click handler always passed `this.px` or `this.py`
as the fixed axis, ignoring where on the border the player actually
clicked. Clicking the top border at column 15 while standing at column 5
would walk straight up instead of diagonally. Fixed by passing the actual
`(tx, ty)` coordinates directly to `startMovingTo`, which clamps them to
the nearest edge tile and computes a Bresenham path there first.

---

## Phase 7 — Combat System

**Goal**: Weapon firing, tick-based projectile animation, damage,
death/respawn, and XP/level progression.

### Protocol Messages

**C→S**: `FIRE_WEAPON { hand, targetX, targetY }`

**S→C**: `YOUR_STATS { hp, maxHp, power, maxPower, xp, level }`,
`PLAYER_HEALTH { id, hp, maxHp }`,
`MISSILE_START { id, room, path[], objType, msPerStep, dx, dy }`,
`MISSILE_END { id }`,
`REPORT { text }`,
`YOU_DIED { killedBy, killerName, respawnRoom, respawnX, respawnY }`

### Weapon Firing (`session.ts :: onFireWeapon`)

Left/middle canvas click sends `FIRE_WEAPON` with the target tile. The
server:
1. Checks the hand item exists and has `weapon: true`.
2. For `numbered` items (guns, staves), requires `quantity > 0` and
   decrements it; clears the hand slot if it hits 0.
3. Looks up damage: `obj.damage ?? bulletObj?.damage ?? 10` where
   `bulletObj = objects[obj.movingobj]`. Guns (hand gun 180, machine gun
   182) carry no `damage` field — damage lives on the bullet object
   (184/185). Without this fallback they would always deal 10.
4. Computes a Bresenham path from the player to the target tile, capped
   at `obj.range` steps, stopping at the first non-`permeable` wall/floor
   cell or the grid boundary.
5. Finds the first player occupying a cell along the path (`hitPlayer`).
6. Broadcasts `MISSILE_START` to all players in the room, then schedules
   `MISSILE_END` + damage application via `setTimeout(travelMs)`.

### Missile Speed Formula

Derived from the original C source constants:
`CLICKS_PER_MOVE = 500`, `MISSILE_SPEED_FACTOR = 2.2`

```
msPerStep = Math.max(50, Math.round(2500 / (speed * 2.2)))
```

`speed` comes from `bulletObj.speed ?? obj.speed ?? 5`.

### Tick-Based Missile Animation (client)

`MISSILE_START` carries the full pre-computed path array plus `msPerStep`,
`objType`, and the unit direction vector `(dx, dy)`. The client:
- Stores a `MissileAnim` entry in a `missiles: Map<id, MissileAnim>`.
- Steps through path positions via a `setTimeout` chain at `msPerStep`
  intervals, re-rendering each step.
- `MISSILE_END` cancels any pending timer and deletes the entry.

**Directional sprites**: objects with `directional: true` are rotated on
the canvas. The base bitmap faces UP; rotation angle is
`Math.atan2(dx, -dy)` radians clockwise. `ctx.save / rotate / restore`
wraps a centered `drawImage` call.

### Damage, Death, and Respawn

`dealDamage(victim, damage, attacker)` clamps HP to 0 and broadcasts
`PLAYER_HEALTH`. `REPORT` messages are sent privately to attacker and
victim ("You hit X for N." / "X hits you for N!").

On death (`hp === 0`), `killPlayer`:
- Increments victim's `deaths`, attacker's `kills`; broadcasts `PLAYER_STATS`.
- Awards XP to attacker: `10 + victim.level * 10`; triggers level-up if
  threshold crossed (`level * 100` XP needed per level).
- Each level up increases `maxHp` (+20) and `maxPower` (+10).
- Drops all victim inventory to the floor (`onLeave`-style drop loop).
- Sends `YOU_DIED` to victim; victim is teleported to room 0 at (10,10).
- GM chat broadcast: "X was slain by Y."

### Stats UI

Right sidebar above the inventory panel shows HP/MP bars and XP/Level.
`YOUR_STATS` is sent on join and after any stat change. `PLAYER_HEALTH`
updates other players' visible HP bars (in the player list).

---

## Phase 8 — Door / Key Mechanism

**Goal**: Players can use held openers (keys, repair kits) to toggle
swinging objects (doors) on adjacent tiles.

### How the Original Game Worked

From `src/play.c :: use_opener` and `open_something_on_square`:
- Left/middle click on an adjacent tile calls `use_object`, which
  detects `obj.opens > 0` and calls `use_opener`.
- `use_opener` iterates `recorded_objects` at the target tile, looking
  for any with `swings: true`.
- Match condition: `(opener.opens & door.type) != 0` (bitwise AND) AND
  compatible `id` fields (0 = universal).
- If matched: `door.type = door.alternate` (swap in-place).
- Key (`opens: 8`) is NOT consumed — not `numbered`, not `lost`.
- Repair kit (`opens: 128`, `numbered: true`) consumes one charge.

### Standard Object Data

The `standard.obj` binary does not encode the `type` bitmask field for
door objects (absent = 0 by default in C). Since `opens & 0 = 0` would
never match, the web rewrite skips the bitmask check and allows any
held item with `opens > 0` to toggle any adjacent `swings: true` object.
In practice only keys and repair kits are takeable openers, so this
simplification is safe for the current map set.

### Protocol

**C→S**: `USE_ITEM { hand, targetX, targetY }`

**S→C**: `ROOM_OBJECT_CHANGED { room, x, y, newType }`

### Server Handler (`session.ts :: onUseItem`)

1. Verify hand item exists and `obj.opens > 0`.
2. Chebyshev distance check: target must be adjacent (distance = 1).
3. Iterate `room.recorded_objects` at `(targetX, targetY)`. For each with
   `swings: true` and a defined `alternate`: swap `ro.type = doorDef.alternate`.
4. Broadcast `ROOM_OBJECT_CHANGED` for each toggled object.
5. If opener is `numbered`, decrement charge; clear hand slot at 0.

### Client Handler

`onRoomObjectChanged`: finds the first `recorded_object` at `(x, y)`
whose current type has `swings: true`, updates its `type` to `newType`,
invalidates `roomBg`, and re-renders. Door appearance updates for all
players in the room immediately.

### Click Routing

Left/middle canvas click priority order:
1. Floor item present at tile → `sendPickup`
2. Hand item has `opens > 0` AND target is Chebyshev-adjacent → `sendUseItem`
3. Otherwise → `sendFireWeapon`

Hand item state is tracked in `Game` via `setHands(left, right)`, called
from `main.ts` whenever `YOUR_INVENTORY` is received.
