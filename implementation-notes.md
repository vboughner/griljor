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
- `legacy/bitmaps/` + `legacy/bit/` → `data/objects/bitmaps/{objset}/` (terrain/object tiles)
- `legacy/facebits/` → `sprites/facebits/` (player avatar pairs: `*bit` + `*mask`)

### Object Definition Parsing (`parse_objs.py`)

`.obj` files are a fixed-size binary record array. Each record encodes
object properties including name, flags (permeable, transparent, masked,
exit, takeable, etc.), bitmap filename references, and combat stats.

Output: `data/objects/{name}.json` with an `objects` array indexed by
object ID (0–254).

Key fields preserved:
- `bitmap` / `mask` — PNG filenames for the tile sprites
- `masked` — whether the mask file must be applied
- `permeable` — whether missiles can pass through (NOT player movement — see Phase 4)
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

#### Object File per Map

Each `.map` file's header contains an `objfilename` field that names the
object set whose IDs the map tile data references. The server reads this
field and loads the matching `data/objects/{name}.json` at startup.
**Object IDs are not universal — they are relative to the map's own object
file.** The same integer ID means a completely different object in
`default.obj` vs `standard.obj`.

| Object file | Maps that use it |
|-------------|-----------------|
| `default.obj` | battle, blowup, castle, flag, hometown, outdoor, paradise, paradise2, paradise3, playtesters, shooter, sword, three, title, two, twoperson |
| `flames.obj` | flames, flash, ivarr, shelter, tunnel |
| `main.obj` | hack, hack1, main |
| `ring.obj` | ring |
| `trek.obj` | trek |
| `standard.obj` | standard (diag format) |

**Critical implication**: any script that inspects or manipulates map tile
IDs (e.g. to identify weapons, flags, or other takeable objects) must load
the object JSON that matches the map's `objfilename`, not hardcode
`standard.json`. The pipeline regenerates maps with `python3
parse_maps.py legacy/lib/map/*.map` — after regenerating, re-run the
`recorded_objects` fix sweep (see "Weapons in Spot Data" section below)
using each map's correct object file.

#### Weapons and Takeable Items in Spot Data

The original map editor allowed placing any object — including weapons and
takeable items — directly in the `spot[x][y]` tile grid. Such items render
visually as floor/wall tiles but the server's item system only surfaces
items from `recorded_objects`, so they cannot be picked up.

All maps have been post-processed to move weapon/takeable objects out of
the spot grid and into `recorded_objects`. The fix script pattern (run from
repo root):

```js
const fs = require('fs'), path = require('path');
const mapDir = 'pipeline/out/data/maps';
const objDir = 'pipeline/out/data/objects';
for (const file of fs.readdirSync(mapDir).filter(f => f.endsWith('.json'))) {
  const filePath = path.join(mapDir, file);
  const data = JSON.parse(fs.readFileSync(filePath, 'utf8'));
  const objBasename = path.basename((data.map?.objfilename || 'standard.obj').replace(/\.obj$/, ''));
  const objs = JSON.parse(fs.readFileSync(path.join(objDir, objBasename + '.json'), 'utf8')).objects;
  for (let r = 0; r < data.rooms.length; r++) {
    const room = data.rooms[r];
    if (!room.spot) continue;
    const newRO = [];
    for (let x = 0; x < 20; x++) for (let y = 0; y < 20; y++) {
      const tile = room.spot[x][y];
      for (const si of [0, 1]) {
        const id = tile[si]; if (!id) continue;
        const obj = objs[id]; if (!obj || (!obj.weapon && !obj.takeable)) continue;
        newRO.push({ x, y, type: id, detail: obj.def0 ?? 1, infox:-1, infoy:-1, zinger:-1, extra:[-1,-1,-1] });
        tile[si] = si === 0 ? room.floor : 0;
      }
    }
    if (newRO.length) room.recorded_objects = (room.recorded_objects || []).concat(newRO);
  }
  fs.writeFileSync(filePath, JSON.stringify(data, null, 2));
}
```

This must be re-run after any pipeline regeneration, and must use each
map's own object file (as shown above) — using `standard.json` for all
maps will misidentify IDs and either miss items or incorrectly move
non-takeable objects.

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
explicitly. `permeable` controls missile passage only (see Phase 4).

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

## Phase 3 — Inventory System

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

## Phase 4 — Client UI Refinements

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

**Original semantics** (confirmed from `legacy/src/objects.h` and
`legacy/src/mapfunc.c`):
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

## Phase 5 — Combat System

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

## Phase 6 — Door / Key Mechanism

**Goal**: Players can use held openers (keys, repair kits) to toggle
swinging objects (doors) on adjacent tiles.

### How the Original Game Worked

From `legacy/src/play.c :: use_opener` and `open_something_on_square`:
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

---

## Phase 7 — Map State Reset on Empty Server

**Goal**: Allow a map to automatically revert to its initial state (items,
doors, chat history) a configurable number of seconds after the last player
leaves. Maps that don't set this flag retain state indefinitely until the
server process restarts.

### Map-level configuration (`pipeline/out/data/maps/*.json`)

Two optional fields added to the `map` object in each map's JSON file:

| Field | Type | Default | Meaning |
|-------|------|---------|---------|
| `resetOnEmpty` | boolean | `false` | Whether to reset state when the server empties |
| `resetAfterSeconds` | number | `30` | Seconds to wait before resetting |

`castle.json` is set to `resetOnEmpty: true, resetAfterSeconds: 10`.

### Server (`world.ts`)

`World` interface gains `resetOnEmpty: boolean` and `resetAfterSeconds: number`.
`loadWorld` reads both from `data.map`, applying defaults if absent.

### Server (`session.ts`)

**Construction**: The constructor deep-copies all `recorded_objects` arrays
into `originalRecordedObjects: RecObj[][]`. This snapshot is the source of
truth for resets, since door toggling mutates `recorded_objects` in-place.

**On last player leaving** (`onLeave`): if `world.resetOnEmpty`, schedules
`resetWorldState()` via `setTimeout(delay * 1000)` and stores the handle in
`resetTimer`. If `resetOnEmpty` is false, only chat history is cleared (existing behaviour).

**On new player joining** (`onJoin`): if `resetTimer` is non-null, the timer
is cancelled (`clearTimeout`) before the player is admitted, so a player who
arrives during the countdown prevents the reset.

**`resetWorldState()`**:
1. Restores each room's `recorded_objects` from the deep-copied snapshot (fresh `{ ...ro }` spread per object so subsequent mutations don't corrupt the snapshot).
2. Clears and re-runs `initRoomItems()` to rebuild floor item maps from the restored objects.
3. Clears `chatHistory`.

---

## Phase 8 — Lobby Redesign

**Goal**: Replace the lobby's polling model with WebSocket push, enrich each
server row with live player avatars and map metadata, and tighten the UI.

### Design Decisions

- **No player profile persistence** — every game session starts fresh at
  level 1. Stats are in-memory only and lost on disconnect or server restart.
- **No item-level requirements** — all items are usable at any level.
- **AI monsters deferred** — unfinished in the original; excluded from the
  rewrite scope and tracked as a separate future project if desired.

### WebSocket Push (`server/src/lobby.ts`)

The lobby server previously served only HTTP (`GET /games`, `POST /register`,
etc.). It now also accepts WebSocket connections on path `/watch`, sharing the
existing `http.Server` instance via `new WebSocketServer({ server, path: '/watch' })`.

- `watchers: Set<WebSocket>` tracks connected lobby clients.
- `broadcast()` purges stale entries then sends the full games list as JSON to
  every open watcher socket.
- Called after every `/register`, `/heartbeat`, and `/unregister` mutation.
- On connect, the current list is sent immediately so the client doesn't wait
  for the next mutation.

**Client (`client/src/lobby.ts`)**: `watchGames(onUpdate, onError)` opens a
WebSocket to `ws://localhost:3000/watch`, calls `onUpdate` on each push,
and returns the socket for lifecycle management.

**Immediate heartbeat on join/leave**: Previously the game server notified
the registry only via a 5-second `setInterval`, so lobby player counts
lagged by up to 5 seconds. The heartbeat call was extracted into a
`sendHeartbeat()` function in `server/src/main.ts` and passed into
`GameSession` as `opts.onPlayerCountChange`. `GameSession` stores it as
`private onPlayerCountChange?: () => void` and calls it immediately after
`this.players.set()` in `onJoin` and after `this.players.delete()` in
`onLeave`. The 5-second interval is kept as a safety net. Lobby now
reflects join/leave within ~100–200 ms.

**Client (`client/src/main.ts`)**:
- `startLobbyWatcher()` / `stopLobbyWatcher()` called from `showLobby()` /
  `showGame()`.
- On WebSocket error, automatically falls back to a 20-second HTTP poll so
  the lobby still works if the server doesn't support WebSockets.
- The manual Refresh button was removed; a subtle `updated H:MM AM/PM`
  timestamp under the table updates on every render.

### Avatar Strip in Server Rows

The game server's heartbeat now includes `avatars: Array<{ avatar, name }>`
(from `GameSession.playerAvatars`). The lobby stores and broadcasts these
with each push.

Each server row renders a 32×32 canvas per player, drawn via the same
`loadMaskedSprite` pipeline used in-game. Hovering shows the player's chosen
name via the shared `showTooltip` / `moveTooltip` / `hideTooltip` system
from `tooltip.ts`.

The avatar strip uses `flex-wrap: wrap` with `width: 300px`, fitting 8
avatars per row and expanding naturally for larger player counts.

**Join button disabled** when:
- The selected avatar matches one already in the game (`data-avatars` CSV
  checked against `avatarSelect.value`); or
- The game is full (`players >= maxPlayers`, stored as `data-full`).

Both conditions are re-evaluated whenever the avatar select changes
(`updateJoinButtons()`).

### Map Metadata in Lobby

`World` gains `title`, `teams`, and `maxPlayers` fields read from the map JSON:

| Field | Source | Default |
|-------|--------|---------|
| `title` | `map.name` | map filename |
| `teams` | `map.teams_supported` | `0` (FFA) |
| `maxPlayers` | `map.maxPlayers` | `16` |
| `rooms` | `rooms.length` | — |

These are sent in the `/register` call and displayed as separate fixed-width
columns in the lobby table. Teams shows as `"FFA"` when 0, otherwise the raw
number. Rooms shows the room count.

### Lobby Table Layout

The table uses fixed-width columns aligned between header and data rows:

| Column | Class | Width |
|--------|-------|-------|
| Map title | `.server-map` | 160px, wraps |
| Player avatars | `.server-avatars` | 300px, flex-wrap |
| Count | `.server-players` | 55px |
| Teams | `.server-teams` | 50px |
| Rooms | `.server-rooms` | 50px |
| Join | `.join-btn` | — |

The `#lobby-screen` was widened to `max-width: 780px`. Row height is
content-driven (no fixed `min-height`) so empty rows are compact and
rows with many players expand naturally.

Note: the old `avatarSelect.value` reference in `updateJoinButtons` and
elsewhere was replaced by a `selectedAvatar` variable once the native
`<select>` was removed.

### GRILJOR Letter Logo

The `<h1>GRILJOR</h1>` text heading in the lobby was replaced with a
canvas that renders the same letter bitmaps used on the title screen
(`/sprites/bitmaps/g.png`, `r1.png`, `i.png`, `l.png`, `j.png`, `o1.png`,
`r2.png`).

`drawLogo(canvas)` was exported from `title.ts`. It reuses the same scale
calculation as the title screen (fit letters to canvas height, then
constrain total width to canvas width minus padding). The `j` letter
retains its 20% vertical drop. The canvas is sized in `main.ts` to
`Math.max(60, Math.floor(window.innerHeight / 4) - 30)` px tall × 780px
wide, matching the approximate letter size from the title screen's top band.

---

## Phase 9 — Consumables

**Goal**: Allow players to use held items that restore HP or Power by clicking their own tile.

### Object Data Fields

Two new fields on `ObjDef` (both server `world.ts` and client `types.ts`):

| Field | Type | Meaning |
|-------|------|---------|
| `health` | `number` | Negative = restores that many HP on use. Positive = costs HP (not enforced per design). |
| `mana` | `number` | Negative = restores that many Power on use. Positive = costs Power (not enforced). |
| `lost` | `boolean` | Already present in JSON; declared explicitly now. Consumed on use when combined with `numbered`. |

### Consumable Objects (standard.json)

| Index | Name | Effect |
|-------|------|--------|
| 236 | potion | `health: -50` — restores 50 HP |
| 237 | jug | `health: -25` — restores 25 HP |

Both are `numbered: true` + `lost: true`, meaning each use decrements quantity and the item disappears from the hand slot when charges reach 0.

No mana-restoring items exist in the current object set. The scroll (index 232) has `mana: 20` which is a positive value — a cost-to-fire field for its weapon behaviour, not a restore.

### Server (`session.ts :: onUseItem`)

The consumable branch runs before the door-opener branch. Guard conditions:
- If item only heals HP: blocked when `player.hp >= player.maxHp`
- If item only restores Power: blocked when `player.power >= player.maxPower`
- If item restores both: blocked only when both are already full

On success:
- `player.hp = clamp(hp - obj.health, 0, maxHp)` (subtracting a negative = addition)
- `player.power = clamp(power - obj.mana, 0, maxPower)`
- Decrement `handItem.quantity`; clear hand slot at 0; call `sendInventory`
- Send `YOUR_STATS` to self (updates HP/MP bars)
- Broadcast `PLAYER_HEALTH` to room (updates other players' HP bar displays)

### Client (`game.ts`)

Click routing priority (updated):
1. Floor item at tile → `sendPickup`
2. Hand has `opens > 0` AND target is Chebyshev-adjacent → `sendUseItem` (door)
3. **Hand has `health < 0` or `mana < 0` AND click is on own tile → `sendUseItem` (consume)**
4. Target tile differs from player tile → `sendFireWeapon`

### Tooltip (`tooltip.ts`)

`buildItemHtml` shows heal/restore lines after the weapon stats block:
- `health < 0` → `Heals N HP`
- `mana < 0` → `Restores N Power`

---

### Custom Avatar Picker

The native `<select id="avatar-select">` was replaced with a custom
dropdown built from canvas elements:

- `#avatar-picker` wraps `#avatar-preview` (the 32×32 trigger canvas)
  and `#avatar-dropdown` (an absolute-positioned CSS grid, 5 columns,
  hidden by default).
- A `▾` caret below the preview signals it is clickable.
- Clicking the preview toggles the dropdown; clicking outside closes it
  via a `document` click listener with `stopPropagation` on internal clicks.
- Each dropdown cell is a 32×32 canvas drawn via `drawAvatarOnCanvas`,
  with a `title` showing the avatar name.
- A `↻` random avatar button picks a different avatar at random.

**Name field sync**: `nameManuallyEdited: boolean` tracks whether the
player has typed in the name field. `setSelectedAvatar(name)` only
overwrites `playerNameInput.value` when `nameManuallyEdited` is false.
Once the player types anything, the name is left alone for the rest of
the session regardless of avatar changes.

**Ordering fix**: `currentGame` was declared after the avatar picker
setup, causing a temporal dead zone error. The state variable block
(`currentGame`, `currentNetwork`, `currentMode`, etc.) was moved to
immediately after the DOM refs, before the avatar picker setup.

---

## Phase 10 — Rendering Fixes and Map Data Corrections

### Unmasked Tile Rendering (Floor/Wall Bleed-Through)

In the original X11 game, all sprites were drawn via `XCopyPlane`: white
pixels drew the background colour (opaque), black pixels drew the
foreground colour. Nothing was ever transparent — only the mask shape
(for masked sprites) controlled whether pixels were drawn at all.

The web renderer's `loadAndProcess` made white pixels transparent for all
sprites, which was correct for masked sprites (where transparency outside
the mask shape is intended) but wrong for unmasked tiles. An unmasked
tile like "plains" (a mostly-white speckled grass texture) became nearly
invisible, letting the floor tile beneath it show through.

**Fix** (`assets.ts`): Added `loadOpaqueTile(url)`. For unmasked tiles,
white pixels are rendered as the map background colour (`#333` dark /
`#e8e8e8` light) rather than transparent. Dark pixels are inverted as
usual for dark mode.

**`renderer.ts`**: `spriteForObj` gains an `opaque` flag. The floor and
wall layers in `buildRoomBackground` pass `opaque = true`, routing
unmasked tiles through `loadOpaqueTile`. Masked sprites (corners, walls,
etc.) continue to use `loadMaskedSprite` unchanged — their outside-mask
transparency is correct and intentional.

The `spriteMap` local cache in `buildRoomBackground` was changed from
`Map<number, …>` to `Map<string, …>` to allow separate cache entries for
the same object ID rendered in opaque vs transparent mode.

### Map Data: Swapped Floor/Wall Tiles

The `parse_maps.py` pipeline writes `spot[x][y] = [floor_id, wall_id]`
from the binary map files, but several tiles in the original data have
the two IDs in the wrong order — an architectural wall object (no
`movement` field) ends up in the floor slot, and a walkable floor tile
ends up in the wall slot. This caused the tile to render as a passable
floor while drawing a wall on top of it, and blocked the tile from
movement when the floor slot's wall object stopped it.

**Fixed tiles (corrected by hand in the JSON map files)**:

| Map | Room | Tiles |
|-----|------|-------|
| `paradise.json` | 1 | (5,16) |
| `paradise.json` | 0 | (0,5) (0,6) (0,7) (1,7) (2,7) (12,15) (17,5) (17,6) |
| `battle.json` | 0–4 | (13,19) (17,19) in each room |

The pattern to recognise a swap: floor slot has an object with no
`movement` field (a wall/blocked type), and wall slot has an object with
`movement > 0` (a walkable type).

### Exit Tiles Unreachable via Mouse (Click-to-Move)

Exit tiles — `recorded_objects` entries with `exit: true` — sometimes
have non-walkable objects in their floor slot (e.g. "laserbolt", "frap
bolt" in `battle.json` room 6 tile 16,18). These are artefacts of the
original object indexing. The `move()` method already checks exits before
collision (`exitMap` lookup comes first), so keyboard movement works fine.

However, the BFS pathfinder (`findNextStep`) and the Bresenham blocked
check in `doMoveStep` both called `isTileBlocked` without any exit
awareness, so they treated exit tiles as impassable and could not route
toward them.

**Fix** (`game.ts`): `isTileBlocked` and `findNextStep` accept an
optional `exitKeys?: Set<string>` parameter. When a tile key is in
`exitKeys` the function returns `false` immediately (walkable). In
`doMoveStep`, `new Set(this.exitMap.keys())` is passed to both the
Bresenham blocked check and the BFS fallback, making exit tiles
transparent to pathfinding while the movement system still triggers the
room transition when the player steps onto them.

---

## Phase 11 — Numbered Weapon Charges (Ammo)

**Goal**: Numbered weapon items (guns, staves) consume one charge per shot
and disappear from the hand slot when exhausted. The charge count is visible
on the hand slot UI.

### Data Model

`numbered: true` items use `quantity` to track remaining charges.
`capacity` in the object definition is the maximum (e.g. hand gun:
`capacity: 13`). Floor items pick up their initial charge count from
`ro.detail` in the map's `recorded_objects` array; if `detail` is 0 the
fallback is 1. Maps with pre-placed guns carry the correct detail value
(e.g. castle.json: `"detail": 20` for object 64, a gun with `capacity: 40`).

### Server (already implemented in Phase 5)

`onFireWeapon` in `session.ts`:
1. Guards: `if (obj.numbered && handItem.quantity <= 0) return` — prevents
   firing an empty weapon.
2. Decrement: `handItem.quantity--` on each successful shot.
3. Removal: when `quantity` hits 0, the hand slot is set to `null`.
4. Sync: `sendInventory(player)` is called after every decrement so the
   client immediately sees the updated count.

### Client — Hand Slot Charge Badges

**Previously missing**: inventory grid cells already showed a yellow
charge count badge (`inv-count` span) for numbered items, but the hand
slot canvases in the mouse widget had no equivalent display.

**Added** (`index.html` + `main.ts`):
- Two `<span>` elements (`#hand-left-count`, `#hand-middle-count`) added
  inside `#mouse-bitmap-wrap`, positioned absolutely at the bottom of each
  hand slot canvas (same position logic as `inv-count`).
- `.hand-count` CSS class: `position: absolute; top: 40px; font-size: 9px;
  color: #ff0; pointer-events: none; line-height: 1`. Specific slots are
  right-aligned within their canvas via `right: 90px` (left slot) and
  `right: 50px` (middle slot) — these values place the text flush with the
  right edge of each 32×32 canvas inside the 128-wide wrapper.
- `updateInventoryPanel` sets `textContent` to the quantity string when
  `obj.numbered` is true for the hand item, or `''` when the slot is empty
  or holds a non-numbered item. Clears automatically on item removal.

---

## Lobby UI Style — Design Decisions

These decisions reflect a deliberate visual direction established during a
styling pass. Avoid reverting these without good reason.

### Color Palette

The lobby uses a strict dark monochrome base with a single amber accent:

| Role | Value |
|------|-------|
| Page background | `#1a1a1a` |
| Panel border | `#303030` |
| Panel header background | `#202020` |
| Panel header border-bottom | `#303030` |
| Row dividers | `#282828` |
| Server column header background | `#1e1e1e` |
| Primary text | `#ccc` |
| Secondary text (counts, stats) | `#888` |
| Dim text (header labels) | `#606060` |
| **Amber accent** — interactive borders | `#52380e` |
| **Amber accent** — interactive text | `#b8842a` |
| **Amber accent** — interactive background | `#1e1910` |
| **Amber hover** — border | `#7a5220` |
| **Amber hover** — text | `#d4a040` |

The amber accent (`#b8842a` / `#52380e`) is used **only** on interactive
elements: join buttons, avatar preview border, player name input border,
and the random avatar button. Everything else is gray/black. This makes
clickable/editable elements immediately identifiable without adding noise.

### Font Sizes (all `px`, no `em` in the lobby)

| Element | Size |
|---------|------|
| Panel header labels (PLAYER SETUP, AVAILABLE GAMES) | `13px` |
| Server column header row (MAP, PLAYERS, etc.) | `11px` |
| Server data rows | `15px` |
| Player name input | `15px` |
| Join buttons | `15px` |
| Random avatar button (↻) | `24px` |
| Status text (`#lobby-status`) | `13px` |

### Panel Structure

The lobby uses two bordered panels (`.lobby-panel`) with dark header bars
(`.lobby-panel-hdr`). These create clear visual sections without relying
on whitespace alone. Panels have `margin-bottom: 40px` between them.

The **Available Games** panel uses class `.lobby-panel-games` which removes
the `border-bottom` from its header bar, visually merging it with the
column header row below. The **Player Setup** panel retains its border.

### Server Row Layout

- All rows (column headers + data) share the same `16px` horizontal padding
  so columns align perfectly edge-to-edge.
- `min-height: 54px` is set on data rows so the row height does not shift
  when players join or leave (avatar canvases are 32px tall; 54px is enough
  to contain them comfortably with 10px top/bottom padding).
- The join button uses `margin-left: auto` to sit flush against the right
  edge of the row, regardless of the widths of the columns to its left.
- Row hover is intentionally absent — only the join button is interactive,
  so highlighting the whole row would be misleading.

### Avatar Picker

- The avatar preview has an amber border matching the other interactive elements.
- `box-shadow: inset 0 0 0 2px #111` creates a 2px visual gap between the
  avatar sprite and the border without shrinking the 32×32 canvas.
- The dropdown opens on CSS hover (`#avatar-picker:hover #avatar-dropdown`)
  rather than on click, removing the need for JS click toggles and a close
  handler.
- The caret was removed; the amber border signals interactivity.
- The dropdown is positioned at `left: 4px; top: 38px` to account for the
  4px padding on `#avatar-picker` and to overlap slightly with the avatar
  border, ensuring the CSS `:hover` chain is never broken as the mouse
  moves from the avatar to the dropdown.

### Disabled Join Button Tooltip

When the join button is disabled because the player's selected avatar is
already in use in that game, hovering the button shows a tooltip:
`"Your avatar is already in use in this game. Pick a different one to join."`

This uses the existing `showTooltip` / `moveTooltip` / `hideTooltip`
system. The tooltip is attached via `btn.onmouseenter` / `btn.onmouseleave`
(not `addEventListener`) so it is cleanly replaced on each call to
`updateJoinButtons()` without accumulating duplicate listeners.
Mouse events fire on disabled buttons (there is no `pointer-events: none`
on disabled join buttons).

---

## Design Decision — Mana/Power Removed

### Rationale

The original C game had a concept of "power" (mana) as a player resource
alongside HP. When porting to the web rewrite, a power stat was scaffolded
in — the player interface tracked `power`/`maxPower`, it was included in
the `YOUR_STATS` protocol message, and an MP bar was displayed in the
stats panel.

However, an audit of the object data revealed the mana system was never
meaningfully used:

- Only **one object type** (the `scroll`, object 232 in `standard.json`)
  had a `mana` field, and its value was positive (`mana: 20`), meaning it
  was never a mana-restoring consumable.
- The weapon-fire handler (`onFireWeapon` in `session.ts`) never checked
  or consumed `player.power` before allowing a weapon to fire. In other
  words, no weapon had a mana cost in practice.
- Only 3 maps used the scroll at all (`main`, `playtesters`, `twoperson`),
  and two of those are test/demo maps.

Given that the power system was invisible to gameplay, added protocol
complexity, and cluttered the UI with a stat bar that never changed
meaningfully, the decision was made to remove it entirely and keep the
game's resource model to a single stat: HP.

### What Was Removed

**Server (`server/src/session.ts`)**
- `power` and `maxPower` fields removed from the `Player` interface
- `BASE_POWER`, `POWER_PER_LEVEL` constants removed
- `maxPowerForLevel()` helper removed
- Player initialisation no longer sets power/maxPower
- Consumable item handler simplified: only checks `obj.health < 0` (HP
  restore); mana-restore branch (`restoresMp`, `mpFull`) removed
- Level-up handler no longer increases `maxPower`
- Respawn no longer restores `player.power`
- `sendStats()` no longer includes `power`/`maxPower` in `YOUR_STATS`

**Protocol (`server/src/protocol.ts`, `client/src/network.ts`)**
- `power` and `maxPower` fields removed from the `YOUR_STATS` message type

**Object interfaces (`server/src/world.ts`, `client/src/types.ts`)**
- `mana?` field removed from `ObjDef` / object type interfaces
- The raw JSON object data files in `pipeline/out/data/objects/` are left
  unchanged (the `mana` field on the scroll is simply ignored at runtime)

**Client UI (`client/src/main.ts`, `client/index.html`)**
- `updateStats()` signature reduced to `(hp, maxHp, xp, level)`
- MP bar (`#mp-bar`), MP text (`#mp-text`), and `.mp-fill` CSS class
  removed from the stats panel
- Tooltip for mana-restoring items removed from `tooltip.ts`
- Consumable detection in `game.ts` simplified to `health < 0` only

### Scroll Behaviour After Change

The scroll (`strength: 80`, `weapon: true`, `range: 25`) works as a
straightforward ranged weapon dealing 80 damage per hit. The `mana: 20`
field it carried in the object data is now simply ignored. No map changes
were required.

---

## Design Decision — XP/Levels Removed

### Rationale

The original Griljor was a persistent game where players logged in repeatedly
over days or weeks. Level progression rewarded long-term investment. The web
rewrite has no session persistence — every player starts fresh at level 1 each
time they join, and stats are discarded when they leave. In this context,
levelling up carries no lasting meaning: any HP advantage gained during a
session is lost the moment the player disconnects.

The only in-session effect of the XP/level system was HP scaling (+20 max HP
per level). Since there were no item level requirements (the `maxlevel` field
on the scroll object existed in raw JSON but was never checked by any code),
no gameplay gating was lost. The kill-announcement REPORT messages simply drop
the `(+N XP)` suffix.

### What Was Removed

**Server (`server/src/session.ts`)**
- `BASE_HP`, `HP_PER_LEVEL`, `XP_PER_LEVEL` constants removed
- `maxHpForLevel()` and `levelForXp()` helper functions removed
- `xp` and `level` fields removed from the `Player` interface
- Player initialisation simplified to `hp: 100, maxHp: 100`
- `killPlayer()`: XP reward calculation, level-up check, and level-up REPORT
  removed; kill announcement reduced to `"You killed X!"`
- `sendStats()` no longer includes `xp`/`level` in `YOUR_STATS`

**Protocol (`server/src/protocol.ts`, `client/src/network.ts`)**
- `xp` and `level` fields removed from the `YOUR_STATS` message type

**Client UI (`client/src/main.ts`, `client/index.html`)**
- `updateStats()` signature reduced to `(hp, maxHp)`
- `xpLine` DOM lookup and `Lvl N · XP: N` display removed
- Orphaned `#xp-line` CSS rule removed from `index.html`

### Unchanged

All players now have a fixed 100 HP for the duration of every session. The
kill/death scoreboard and REPORT messages still work as before. Item tooltips
are unaffected. No object data files were modified.

---

## Phase 12 — Game Screen UI Overhaul

### Title Screen: River Tile Masking

River tiles in the original bitmaps have sparse dark ripple marks on a white
background. When displayed with white transparency, the white background turned
invisible but left too much bare space. Without explicit mask files, a
dilation algorithm was used to infer the river body:

`loadRiverBitmapMasked(url)`:
- Marks all dark pixels (value < 200) in an `isDark` byte array.
- For each white pixel, performs a radius-6 neighbourhood search. If any dark
  pixel falls within that radius, the white pixel is considered part of the
  river body (kept opaque); otherwise it is made transparent (alpha = 0).
- RADIUS = 6 was tuned interactively (3 left too much black space; 6 was right).

A background pre-pass draws terrain tiles (forest or sand) under each river
tile position before the masked rivers are drawn on top, replicating the
layered look of the in-game tiles.

### Stale Compiled Output Bug

All title screen changes appeared to have no effect for several sessions. Root
cause: a previous `tsc` run had emitted `main.js`, `title.js`, etc. alongside
their `.ts` sources in `client/src/`. Vite served those pre-compiled files
instead of the TypeScript source.

**Fix**: deleted all `.js` files from `client/src/`. Changed the build script
in `client/package.json` from `"tsc && vite build"` to `"tsc --noEmit && vite
build"` so `tsc` only type-checks and never emits output files.

### `drawLogo` j-Descender Fix

The `drawLogo` function in `title.ts` scales letters to fill the canvas height
H, then shifts the `j` down by `extraY = 0.2 * h`. Since the letter is already
scaled to fill the full canvas, the j always clips at the bottom regardless of
canvas height.

**Fix**: changed the scale line to use `H / 1.4` as the effective height:

```typescript
scale = Math.min(scale, (H / 1.4) / bm.height);
```

With this, `tallest ≈ H/1.4`, `baseY ≈ H*0.083`, and the j's bottom lands at
`baseY + extraY + tallest ≈ H`. The letters are smaller within the canvas but
the descender is fully visible.

### Game Screen Header

**Before**: A plain `<h1>GRILJOR</h1>` text heading above the room info line
`Room N: "name" — (x, y)`.

**After**: A flex row (`#game-header`, `width: 704px`) containing:
- `#game-logo` — a `400×100` canvas rendering the GRILJOR letter bitmaps via
  `drawLogo()`, initialized at startup alongside the lobby logo.
- `#room-info` — a flex column containing:
  - `#map-label` (18px, `#aaa`) — the map title, set once when joining.
  - `#room-label` (14px, `#666`) — the current room name, updated each
    render. Rooms named `"no name"` display an empty string.

Player coordinates are no longer shown.

In `main.ts`, `roomInfo` now points to `#room-label` (was `#room-info`). A new
`mapLabel` ref points to `#map-label`. The game logo canvas is drawn at
startup: `gameLogoCanvas.width = 400; gameLogoCanvas.height = 100`.

In `game.ts`, the `roomInfo.textContent` assignment was simplified:
```typescript
this.roomInfo.textContent = (room.name && room.name !== 'no name') ? room.name : '';
```

### Chat Panel Relocated

Chat was moved from below the map canvas to the right column
(`#player-list-panel`), below the player list. The right column is
`min-height: 704px` with `flex: 1` on the player list, so the chat naturally
aligns to the bottom as the player list grows.

Chat log: 5 visible lines (`height: calc(5 * 1.2em + 12px)`), `overflow-y:
scroll`, `scrollbar-gutter: stable`. Custom dark scrollbar: 9px wide,
`#333` thumb on `#111` track (`::-webkit-scrollbar-*` rules).

### Navigation Buttons Removed / Mode Toggle → Hotkey

`#nav { display: none }` — the N/W/E/S direction buttons were hidden (they
duplicate keyboard movement and cluttered the layout).

The light/dark mode toggle button was removed. Pressing **Shift+L** (capital
`L`) calls `toggleMode()`. The key was chosen to be harder to press
accidentally.

```typescript
if (e.key === 'L') { e.preventDefault(); void toggleMode(); }
```

### Leave Game Countdown

The leave button counts down from 5 seconds. Clicking again cancels:

```typescript
leaveCountdown = setInterval(() => {
  secs--;
  if (secs <= 0) { doLeave(); return; }
  leaveBtn.textContent = `Leaving ${secs}…`;
}, 1000);
```

A popup confirmation dialog was tried and then removed in favour of this
in-button countdown approach.

### Right-Click Context Menu

`contextmenu` is suppressed globally while the game screen is visible:
```typescript
document.addEventListener('contextmenu', (e) => {
  if (gameScreen.style.display !== 'none') e.preventDefault();
});
```

Hand slot canvases (`hand-left-canvas`, `hand-middle-canvas`) retain
right-click drop behavior via `oncontextmenu` handlers that call `sendDrop`.
This works because the handler runs before the global suppressor and the
`sendDrop` call does not require the browser menu.

### Stats Panel

Simplified to a single stat row:
- MP bar, MP text, XP line, and the `STATS` section heading were all removed.
- `HP` label renamed to `Health`; separator line below the health row removed.

### Burden Bar (replaces Inventory heading)

The `INVENTORY` heading was replaced with a `Burden` stat row:
- `stat-label`: "Burden"
- `stat-bar` / `stat-fill` (`#burden-fill`, background `#6b4210`): filled
  proportionally to `currentWeight / maxWeight`.
- `stat-val` (`#inv-weight`): displays `N/150`.

The inventory grid sits directly below with `margin-top: 4px`.

### Player List Layout

The player list (`#player-list`) uses `grid-template-columns: 1fr 1fr`
(two columns). No border, no background — it blends into the surrounding panel
color. `min-height: 214px` reserves space when few players are online.

Player name display: `max-width: 68px; overflow: hidden; text-overflow:
ellipsis; white-space: nowrap` — names longer than ~9 chars show with an
ellipsis.

K/D display: `K:N  D:N` (kills and deaths on one line, two spaces between).

### Player Name Input

`maxlength="10"` restricts name entry to 10 characters.

**Avatar name sync**: `nameManuallyEdited` is set to `false` whenever the
field is cleared to empty. While false, every avatar pick (dropdown or random
button) updates `playerNameInput.value` to the new avatar name. The player
reasserts control by typing anything — at which point `nameManuallyEdited` is
set to `true` and avatar changes no longer overwrite the field.

If the name field is still blank when the player clicks Join, the player's
in-game name defaults to the selected avatar name:
```typescript
const playerName = playerNameInput.value.trim() || selectedAvatar;
```

### Tile Hover Tooltip: Room Number

The tile hover tooltip (`buildTileHtml` in `game.ts`) now includes a `Room N`
line above the `Tile (x, y)` line:
```typescript
const rows: string[] = [
  `<div class="tip-name">Room ${this.currentRoom}</div>`,
  `<div class="tip-name">Tile (${tx}, ${ty})</div>`,
];
```

---

## Team Spawn and Walkability

### Team-Based Respawn

Players are assigned to a team on join (currently hardcoded to team 1 until
team selection UI is implemented). On spawn or respawn, the server calls
`randomSpawnForTeam(team)` which:

1. Collects all rooms whose `room.team` matches the player's team.
2. Shuffles that candidate list.
3. For each candidate room, calls `randomWalkableTile(roomIdx)` to find an
   unoccupied walkable tile.
4. Falls back to any room (team=-1) if no team room has walkable space.

The spawn position is sent in the `ACCEPTED` message so the client navigates
to the correct room immediately on join.

### Walkable Tile Classification

Two map conventions exist, distinguished by `room.floor`:

**Void-floor maps** (`room.floor === 0`, e.g. battle): tiles `[0, 0]` are
open floor (no object at all). All void tiles are walkable.

**Floor-tile maps** (`room.floor > 0`, e.g. ring): tile `[floorId, 0]` is
the real floor inside the room; void tiles `[0, 0]` represent the space
*outside* the room walls and are **not walkable**.

The rule applied in both `randomWalkableTile` (server, `session.ts`) and
`isTileBlocked` (client, `game.ts`):

```typescript
if (!flId && !wlId) {
  // void tile: walkable only on void-floor maps (floor === 0)
  if (!room.floor) walkable.push({ x, y });
  continue;
}
// non-void: walkable only if all objects present allow movement
if (wallObj  && !wallObj.movement)  /* blocked */;
if (floorObj && !floorObj.movement) /* blocked */;
```

`room.floor` is loaded from the map JSON and stored in `RoomData` on both
the server (`world.ts`) and client (`types.ts`, already present).

---

## Dead State and Respawn Timer

### Overview

When a player is killed they enter a **dead state** for 5 seconds before
respawning. During this time:
- Their sprite appears as a tombstone at the death location (visible to all)
- They cannot move, pick up or drop items, fire weapons, or use items
- They can still send chat messages or leave the game
- After 5 seconds the server picks a spawn tile and teleports them there

The delay constant is `RESPAWN_DELAY_MS = 5000` in `server/src/session.ts`.

### Protocol Changes

`PLAYER_INFO` gained a required `dead: boolean` field. Every broadcast of
this message now includes the player's dead state so all clients can render
the tombstone correctly.

`YOU_DIED` was changed: the respawn coords (`respawnRoom/X/Y`) were removed
and replaced with `deadForMs: number` (how long to show the tombstone). The
client uses this only to display a countdown message; the actual respawn
position arrives later.

New `YOU_RESPAWNED` message: `{ type: 'YOU_RESPAWNED'; room: number; x: number; y: number }`.
Sent to the victim after the timer expires with the final spawn location.

### Server (`session.ts`)

`respawnPlayer` was split into two functions:

**`scheduleRespawn(victim, killer)`** — called immediately on death:
- Restores `victim.hp` to full (HP bar resets)
- Sets `victim.dead = true`
- Broadcasts `PLAYER_INFO` with `dead: true` at the death location
- Sends `YOU_DIED` with `deadForMs`
- Schedules `setTimeout(RESPAWN_DELAY_MS)` → `doRespawn`

**`doRespawn(victim, killer)`** — called after the timer fires:
- Calls `randomSpawnForTeam` to find a spawn tile
- Updates `victim.room/x/y`, sets `victim.dead = false`
- Broadcasts `PLAYER_INFO` with `dead: false` at the new location
- Sends `YOU_RESPAWNED` to the victim
- Broadcasts `PLAYER_HEALTH`, sends stats and inventory

Dead-state guards (`if (player.dead) return;`) were added to `onLocation`,
`onPickup`, `onDrop`, `onInvSwap`, `onFireWeapon`, and `onUseItem` so the
server ignores all positional/action messages from a dead player.

`onLeave` calls `clearTimeout(player.respawnTimer)` if a timer is pending.

### Tombstone Sprite

The original game's `legacy/bitmaps/tombbit` + `tombmask` pair was already
converted by the pipeline to:
- `pipeline/out/sprites/bitmaps/tombbit.png`
- `pipeline/out/sprites/bitmaps/tombmask.png`

Served as `/sprites/bitmaps/tombbit.png` and `/sprites/bitmaps/tombmask.png`
(Vite `publicDir` is `pipeline/out`).

Loaded lazily in `game.ts` via `loadMaskedSprite` on the first `goToRoom`
call and stored as `this.tombstoneSprite`.

### Client — Game Canvas (`game.ts`, `renderer.ts`)

`RemotePlayer` gained `dead: boolean`. `onPlayerInfo` stores this field
(and now skips self — see duplicate sprite fix below).

`OtherPlayer` in `renderer.ts` gained `dead?: boolean`. `renderFrame` takes
an additional `tombstoneSprite` parameter; dead remote players draw the
tombstone instead of their avatar sprite.

For the local player, `render()` passes
`this.isDead ? this.tombstoneSprite : this.playerSprite` as the player sprite.

`notifyDied()` — public method: sets `isDead = true`, calls `stopMoving()`,
triggers a re-render.

`notifyRespawned(room, x, y)` — public method: sets `isDead = false`, calls
`goToRoom` to teleport to the spawn location.

`move()` and `doMoveStep()` return early when `isDead`. Canvas mouse handlers
(border clicks, left/middle/right-click for fire/pickup/move) also return
early when `isDead`.

### Client — Player List (`main.ts`)

`PlayerEntry` stores `avatarCanvas: HTMLCanvasElement` so it can be updated
after creation.

`setPlayerDeadDisplay(id, dead)` swaps the canvas content between the
player's avatar sprite and the tombstone sprite. Called:
- From the `onPlayerInfo` wrapper (for all players including remote)
- From `onYouDied` for the local player (`localPlayerId`)
- From `onYouRespawned` for the local player

The tombstone `ImageData` is loaded once and cached in `tombstoneImageData`.

### Bug Fixes in This Session

**`goToRoom` didn't clear the movement timer**: added `this.stopMoving()` at
the start of `goToRoom` so any in-progress click-to-move path is cancelled
before teleporting.

**`onYouDied` in `main.ts` overrode the `game.ts` handler**: the `main.ts`
handler was setting only the report text and not calling `goToRoom`. Fixed
by having `main.ts` call `game.notifyDied()` and a separate `onYouRespawned`
handler call `game.notifyRespawned()`.

**Duplicate sprite after respawn**: `onPlayerInfo` in `game.ts` was storing
the local player in `otherPlayers`, causing a second sprite stuck at the
last known PLAYER_INFO position. Fixed with `if (msg.id === this.myId) return;`
at the top of the handler (matching the existing guard in `onLocation`).
