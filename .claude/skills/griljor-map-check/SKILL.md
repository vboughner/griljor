---
name: griljor-map-check
description: Use when checking or fixing Griljor map JSON files for tile data problems, such as swapped floor/wall object IDs, floor tiles erroneously placed in the wall slot, or wall tiles in the floor slot where the wall slot is empty.
---

# Griljor Map Tile Checker

## Overview

Map rooms use `spot[x][y] = [floor_id, wall_id]`. The floor layer renders first, then the wall layer draws on top. When these IDs are swapped or misassigned, wall tiles visually appear as floor (or vice versa).

## Tile Slot Rules

| Slot | Index | Should contain |
|------|-------|---------------|
| floor | `[0]` | The base tile (floor, ground, grass, etc.) |
| wall  | `[1]` | An object drawn on top — wall, hill, tree, etc. — or `0` for nothing |

The room's `floor` field gives the default floor tile ID. Interior tiles typically look like `[floor_id, 0]`.

## Common Problems

### 1. Swapped floor/wall (hardest to see)
`[wall_id, floor_id]` — wall tile in floor slot, floor tile in wall slot.
The floor tile renders on top, hiding the wall graphic entirely.
**Fix:** swap to `[floor_id, wall_id]`.

### 2. Floor tile bleeding into wall slot
`[wall_id, 4]` where 4 is the floor tile — same visual result as above.
Often appears on one or two edges of a room but not others (asymmetric corruption).
**Fix:** swap to `[floor_id, wall_id]`.

### 3. Wall tile stuck in floor slot, wall slot empty
`[wall_id, 0]` — may render correctly if the wall object looks fine as a floor graphic.
Compare with the symmetric wall on the opposite edge; if that edge has `[wall_id, 0]` too, it's probably intentional.

## How to Check a Room

```js
// Run from repo root: node -e "..." (replace mapName and roomIndex)
const data = require('./pipeline/out/data/maps/FLAG_MAP.json');
const room = data.rooms[ROOM_INDEX];  // 0-based
const floorId = room.floor;

// Find tiles where the floor slot looks like a non-floor object
// and/or the wall slot contains the floor tile ID
const suspect = [];
for (let x = 0; x < 20; x++) {
  for (let y = 0; y < 20; y++) {
    const [fl, wl] = room.spot[x][y];
    if (wl === floorId && fl !== floorId) {
      suspect.push({ x, y, tile: [fl, wl], issue: 'floor_in_wall_slot' });
    }
  }
}
console.log(suspect.length, 'suspect tiles:', suspect);
```

## How to Fix (swap floor/wall slots)

After confirming the affected region (e.g. right column x=19, bottom row y=19):

```js
const fs = require('fs');
const path = './pipeline/out/data/maps/FLAG_MAP.json';
const data = JSON.parse(fs.readFileSync(path, 'utf8'));
const room = data.rooms[ROOM_INDEX];
const floorId = room.floor;
let fixed = 0;

for (let x = 0; x < 20; x++) {
  for (let y = 0; y < 20; y++) {
    const tile = room.spot[x][y];
    if (tile[1] === floorId && tile[0] !== floorId) {
      [tile[0], tile[1]] = [tile[1], tile[0]];  // swap
      fixed++;
    }
  }
}

fs.writeFileSync(path, JSON.stringify(data, null, 2));
console.log('Fixed', fixed, 'tiles');
```

## Whole-Map Scan

To check every room in every map for this pattern:

```js
const fs = require('fs'), path = require('path');
const mapDir = './pipeline/out/data/maps';
for (const file of fs.readdirSync(mapDir).filter(f => f.endsWith('.json'))) {
  const data = JSON.parse(fs.readFileSync(path.join(mapDir, file), 'utf8'));
  for (let r = 0; r < data.rooms.length; r++) {
    const room = data.rooms[r];
    const floorId = room.floor;
    for (let x = 0; x < 20; x++) {
      for (let y = 0; y < 20; y++) {
        const [fl, wl] = room.spot[x][y];
        if (wl === floorId && fl !== floorId) {
          console.log(`${file} room ${r} [${x}][${y}]: [${fl},${wl}] — floor_in_wall_slot`);
        }
      }
    }
  }
}
```

## Notes

- Object data is in `pipeline/out/data/objects/standard.json` — check `objects[id].name` to confirm which ID is a wall vs floor tile.
- After fixing, commit only the affected JSON file and do `git pull` + `pm2 restart all` on production (no rebuild needed for data-only changes).
- The flag map bug (2026-03-09): room 11, right column (x=19) and bottom row (y=19) had `[wall_id, 4]` — fixed by swapping to `[4, wall_id]`.
