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
- `permeable` — whether players can walk through
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

**Wall collision**: a tile is blocked if either its floor-layer or
wall-layer object lacks `permeable: true`. All floor tiles have
`permeable: true`; wall objects do not.

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

## What's Next

**Phase 3 — Server + WebSocket connection**

The `grildriver` C server speaks raw TCP. To connect a browser client
it needs either:
- A WebSocket-to-TCP proxy (bridge approach), or
- A new TypeScript/Node.js server implementing the same protocol

The original packet structures are defined in `src/socket.h`. The
protocol uses fixed-size binary structs (`DriverPack`, `AcceptancePack`,
etc.) over TCP with a separate UDP channel for in-game packets.
