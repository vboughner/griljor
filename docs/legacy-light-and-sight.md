# Legacy Light & Sight System

Investigation of light, darkness, and illumination in the legacy C codebase (`legacy/src/`).

---

## Data Model (fully defined)

### Room properties (`map.h`)

- `dark` field — lighting state:
  - `0` = `LIT` — room always lit
  - `1` = `DARK` — room completely dark
  - `2` = `DAYLIT` — room follows a day/night cycle
- `limited_sight` (bool) — player cannot see the whole room (never used in any map)
- `cycles` (bool) — room follows light cycle (never used in any map)
- Helper macros: `ROOM_IS_DARK(r)`, `ROOM_IS_LIT(r)`, `ROOM_IS_GLOBAL(r)`
- `GLOBAL` rooms: entire room visible but line-of-sight still gates seeing other players

### Object properties (`objects.h`)

- `glows` (bool) — object is visible in the dark even without player light (e.g. stairs, trapdoors)
- `flashlight` (short) — radius in tiles that this object illuminates in dark rooms when carried

### Player (`person.h`)

- `OWN_LIGHT(pnum)` = **6 tiles** — player's natural light radius in dark rooms (no item required)

---

## What Was Implemented vs. Not

### Implemented

- **`sight.c`** — full line-of-sight system using a diagonal pathfinding algorithm:
  - `spot_is_visible(roomnum, x1, y1, x2, y2)` — returns TRUE if one tile can see another; checks intermediate squares for view obstructions; adjacent tiles always visible
  - `update_person_visibility(looker, looked_upon, array)` — updates visibility of one person relative to another
  - `update_vision(looker, array)` — updates visibility of all non-player persons for a given looker
  - This system is active and used for `GLOBAL` rooms
- The three rendering branches in `movement.c` are structurally complete:
  ```c
  if (ROOM_IS_GLOBAL(room))     flag_all_visibilities();
  else if (ROOM_IS_LIT(room))   DrawEgoRaysWithoutLight(room, x, y);
  else if (ROOM_IS_DARK(room))  DrawEgoRaysWithLight(room, x, y, OWN_LIGHT(num));
  ```
- The `glows`, `flashlight`, and `dark` fields are all parsed and stored in memory
- The object editor (`objprops.c`) exposes "Glows in dark" and "Provides light" UI fields

### NOT Implemented — the critical gap

`DrawEgoRaysWithLight()`, `DrawEgoRaysWithoutLight()`, `InitRaypack()`, and `EraseOwnSquare()` are **called** inside `#ifdef RAYCODE` blocks in `movement.c` and `player.c` but are **never defined anywhere** in the source tree.

Without `RAYCODE` defined (the default build), the dark-room rendering path collapses to the non-`RAYCODE` fallback in `playerX11.c`, which uses the simple `can_see[]` boolean array — no radius-based darkness rendering, no fog-of-war.

The `flashlight` radius on held items is never consulted by any rendering code.

**Conclusion:** The whole per-player dark-room fog-of-war / flashlight mechanic was designed and partially wired up but the core rendering functions were never written. `RAYCODE` marks an abandoned branch.

---

## Objects With `flashlight` Set

All instances use `flashlight: 1` (radius = 1 tile). The lamp post and table lamp in `default.json` also have `glows: true` — they are the only objects clearly intended as carriable light sources.

| Object file | Objects |
|---|---|
| `default.json` | candle (~ID 91), laserbolt (~105), unnamed (~121), lamp post (238), table lamp (239) |
| `flames.json` | explosion, wand zap, wall flame gun, wand zap variants, potted plants (~IDs 107–119), radio weapon wavelets |
| `ring.json` | ~22 objects — similar animation-heavy set |

Note: most `flashlight`-tagged objects in `flames.json` and `ring.json` are projectile/animation frames, not carriable items.

---

## Objects With `glows` Only (visible in dark, no illumination)

In `default.json`: trapdoor in floor, cave mouth, ladder, house, all stairs (IDs ~176–190), teleporter cloak, lamp post, table lamp.

In `standard.json`: ~15 stair and transition variants.

In `ring.json`, `trevor.json`: additional stair variants.

---

## Maps With Dark Rooms

| Map file | Dark rooms | `dark` value | Notes |
|---|---|---|---|
| `default.json` | Rooms 0–6 | `1` (DARK) | 7 consecutive dark rooms |
| `paradise.json` | Rooms 0–6 | `1` (DARK) | Same layout pattern |
| `paradise3.json` | Rooms 0–6 | `1` (DARK) | Same layout pattern |
| `hack1.json` | Room 3 | `2` (DAYLIT) | Only day/night room in any map |
| `trek.json` | Room 1 | `1` (DARK) | |
| `sword.json` | Room 0 | `1` (DARK) | Entry room |
| `castle.json` | 1 room | `1` (DARK) | Interior dungeon room |
| `hometown.json` | Room 0 | `1` (DARK) | |

No map uses `cycles: true` or `limited_sight: true`. The `dark: 2` DAYLIT room in `hack1` is the only attempt at a day/night lighting feature anywhere in the map data.

---

## Key Source Files

| File | Role |
|---|---|
| `legacy/src/objects.h` | `glows` and `flashlight` field definitions |
| `legacy/src/map.h` | `dark`, `cycles`, `limited_sight` fields; `DARK`/`DAYLIT`/`LIT`/`GLOBAL` constants and macros |
| `legacy/src/person.h` | `OWN_LIGHT(pnum)` macro (= 6) |
| `legacy/src/sight.c` | Line-of-sight implementation (`spot_is_visible`, `update_vision`) |
| `legacy/src/movement.c` | Three-branch rendering dispatch; `#ifdef RAYCODE` wraps the dark/lit ray functions |
| `legacy/src/playerX11.c` | `can_see[]` / `old_see[]` visibility arrays; fallback visibility logic under `#ifndef RAYCODE` |
| `legacy/src/player.c` | `InitRaypack(0)` call at startup under `#ifdef RAYCODE` |
| `legacy/src/objprops.c` | Object editor UI: "Glows in dark" and "Provides light" fields |
| `legacy/src/MassEdit.h` | Mass-edit UI entry for `flashlight` field |
| `pipeline/out/data/objects/` | Parsed JSON object definitions with `glows` / `flashlight` values |
| `pipeline/out/data/maps/` | Parsed JSON map definitions with `dark` values per room |
