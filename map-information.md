# Map Information

## Team & Spawn Structure

Maps have a `teams_supported` field (1 or 2) and rooms have a `team` field (0=neutral, 1=team1, 2=team2). The actual per-tile spawn coordinates live in binary `.pla` files in `lib/map/` — not yet parsed by the pipeline.

---

## Map Inventory (24 total)

### 2-Team Maps (17)

| Map | Teams |
|-----|-------|
| battle | Administration vs. Custodians |
| castle | White Knights vs. Black Knights |
| flag | Order vs. Chaos |
| flames | Pyros vs. Agents |
| flash | Pyros vs. Agents |
| hack | Team 1 vs. Team 2 |
| hack1 | Winds vs. Brass |
| hometown | Leftists vs. Right Ons |
| outdoor | Boy Scouts vs. Purple People Eaters |
| paradise | Iranians vs. Iraquis |
| shooter | Mutants vs. Humans |
| sword | Protectors of The Queen's Virtue vs. Servants of the Evil King |
| trek | Federation vs. Romulans |
| tunnel | Up and Coming vs. Down and Out |
| two | Lookers vs. Looked Upon |
| twoperson | Charmed vs. Strange |
| title | Team 1 vs. Team 2 (placeholder) |

### Free-for-All Maps (7)

`blowup`, `ivarr`, `main`, `playtesters`, `ring`, `shelter`, `three`

---

## Issues Found

### Broken/Questionable

- **trek** — All spawn rooms (0–3) are assigned to Team 1 (Federation). Team 2 (Romulans) has no dedicated spawn room, only neutral rooms.
- **hack1** — Team 2 (Brass) has 2 spawn rooms; Team 1 (Winds) has 0 dedicated spawns.

### Incomplete/Placeholder

- **title** — Both rooms unnamed; appears to be a template/test map, not a real playable map.
- **flag** — Only 2 of 14 rooms are named (the two fortresses); 12 unnamed neutral rooms suggest the map was never finished.
- **paradise** — 1 room with a null name.

### Minor Issues

- **playtesters** — 3 unnamed rooms (out of 44)
- **hack** — 1 unnamed room
- **shooter** — 1 unnamed room

These rooms were probably just never given names in the editor.

---

## Pipeline Gap

16 maps reference `.pla` files for spawn point data, but those binary files aren't parsed into JSON yet. Maps affected:

`battle`, `castle`, `flames`, `flash`, `hack1`, `hometown`, `ivarr`, `outdoor`, `paradise` (uses `paradise3.pla`), `ring`, `shelter`, `shooter`, `three`, `tunnel`, `two`, `twoperson`

Maps without `.pla` files (use JSON-only room `team` values for spawns):

`blowup`, `flag`, `hack`, `main`, `playtesters`, `sword`, `title`, `trek`

When implementing team spawning, the `.pla` files will need to be parsed or the room-level `team` values used as a fallback.
