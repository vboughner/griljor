# Map Information

All playable maps live in `pipeline/out/data/maps/`. Each map JSON has a
`map.objfilename` field pointing to the object set whose IDs the tile data
uses — see `implementation-notes.md` for the full per-map object file table.

---

## Map Inventory

| Map file | Title | Rooms | Teams | Max players | Object file | `.pla` |
|----------|-------|------:|-------|------------:|-------------|--------|
| `battle.json` | Janitorial War | 9 | Administration vs. Custodians | 12 | default | yes |
| `blowup.json` | Duel | 1 | free-for-all | 4 | default | — |
| `castle.json` | Castles | 14 | White Knights vs. Black Knights | 16 | default | yes |
| `default.json` | Desert Isle Paradise 3 | 16 | Iranians vs. Iraquis | 16 | default | — |
| `flag.json` | No-Man's Land | 14 | Order vs. Chaos | 12 | default | — |
| `flames.json` | Flamer's Paradise | 5 | Pyros vs. Agents | 10 | flames | yes |
| `flash.json` | Powder Flash | 5 | Pyros vs. Agents | 10 | flames | yes |
| `hack.json` | What a map | 3 | Team 1 vs. Team 2 | 6 | main | — |
| `hack1.json` | 8.5th Position | 9 | Winds vs. Brass | 12 | main | yes |
| `hometown.json` | Home Bla Home | 7 | Leftists vs. Right Ons | 10 | default | yes |
| `ivarr.json` | Ivarr's Forum of Fun | 6 | free-for-all | 8 | flames | yes |
| `main.json` | Standards Example | 1 | free-for-all | 4 | main | — |
| `outdoor.json` | Capture the Flag | 3 | Boy Scouts vs. Purple People Eaters | 8 | default | yes |
| `paradise.json` | Desert Island Paradise | 16 | Iranians vs. Iraquis | 16 | default | — |
| `paradise2.json` | Playtester Paradise #2 | 44 | free-for-all | 24 | default | — |
| `paradise3.json` | Desert Isle Paradise 3 | 16 | Iranians vs. Iraquis | 16 | default | yes |
| `playtesters.json` | Playtester Paradise | 44 | free-for-all | 24 | default | — |
| `ring.json` | Ring World | 4 | free-for-all | 6 | ring | yes |
| `shelter.json` | Nuclear Shelter | 5 | free-for-all | 6 | flames | yes |
| `shooter.json` | Mutant Shooters | 8 | Mutants vs. Humans | 10 | default | yes |
| `sword.json` | Swashbucklers | 3 | Protectors of The Queen's Virtue vs. Servants of the Evil King | 6 | default | — |
| `three.json` | Three Floors | 4 | free-for-all | 8 | default | yes |
| `trek.json` | Starship Enterprise | 5 | Federation vs. Romulans | 10 | trek | — |
| `tunnel.json` | Tunnel of Eternal Peril | 5 | Up and Coming vs. Down and Out | 10 | flames | yes |
| `two.json` | Double Vision | 2 | Lookers vs. Looked Upon | 4 | default | yes |
| `twoperson.json` | Quarks | 4 | Charmed vs. Strange | 4 | default | yes |

**Not playable** (no `maxPlayers` set, excluded from normal game listings):

| Map file | Title | Notes |
|----------|-------|-------|
| `standard.json` | Playtester Paradise 4 | Diag-format reference map, 2 rooms |
| `title.json` | map1 | Title screen backdrop, not a game map |

---

## Free-for-All Maps (1 team)

These 8 maps have `teams_supported: 1` — all players compete individually with no team assignments:

| Map file | Title |
|----------|-------|
| `blowup.json` | Duel |
| `ivarr.json` | Ivarr's Forum of Fun |
| `main.json` | Standards Example |
| `paradise2.json` | Playtester Paradise #2 |
| `playtesters.json` | Playtester Paradise |
| `ring.json` | Ring World |
| `shelter.json` | Nuclear Shelter |
| `three.json` | Three Floors |

---

## Player Limit Rationale

Max player counts were set based on room count, walkable area, number of
teams, and map theme. Two-team maps always get even numbers so teams can
be balanced.

| Map | Max | Reasoning |
|-----|----:|-----------|
| paradise2, playtesters | 24 | Largest maps at 44 rooms each — plenty of space |
| castle, default, paradise, paradise3 | 16 | 14–16 rooms, spacious multi-area layouts |
| battle, flag, hack1 | 12 | 9–14 rooms, designed for competitive team play |
| flames, flash, hometown, shooter, trek, tunnel | 10 | 5–8 rooms, medium-sized |
| outdoor | 8 | Only 3 rooms but large open terrain |
| ivarr | 8 | 6 rooms, free-for-all arena |
| three | 8 | 4 rooms, free-for-all |
| hack, ring, shelter, sword | 6 | 3–5 small rooms |
| two | 4 | 2 rooms, designed for small 2v2 play |
| twoperson | 4 | Name says it all — built for 1v1 or 2v2 |
| blowup, main | 4 | Single room, very cramped |

---

## `.pla` Files — Periodic Random Item Drops

Maps with a `.pla` file in `legacy/lib/map/` support periodic random item
drops during gameplay. These are **not** spawn point definitions — player
spawning uses the room `team` field and a random walkable tile.

A `.pla` file specifies an interval (e.g. every 45 seconds) and a list of
items to drop into specific rooms or team-owned rooms on each cycle. For
example, `battle.pla` drops neutron grenades, repair kits, food, and ammo
at regular intervals to keep the map stocked during long games.

See `modern-rewrite-plan.md` for the full format description and
implementation notes. This feature is **not yet implemented** in the modern
rewrite.

Maps **with** `.pla` files: battle, castle, flames, flash, hack1, hometown,
ivarr, outdoor, paradise3, ring, shelter, shooter, three, tunnel, two, twoperson.

Maps **without** `.pla` files (items from `recorded_objects` only, no
replenishment): blowup, default, flag, hack, main, paradise, paradise2,
playtesters, sword, trek.

---

## Known Issues

### Spawn imbalance
- **trek** — All spawn rooms (0–3) are assigned to Team 1 (Federation). Team 2 (Romulans) has no dedicated spawn room, only neutral rooms.
- **hack1** — Team 2 (Brass) has 2 dedicated spawn rooms; Team 1 (Winds) has none.

### Incomplete / placeholder maps
- **title** — Title screen backdrop, not a playable game map.
- **standard** — Diag-format reference/test map used during development.
- **flag** — Only 2 of 14 rooms are named (the two fortresses); 12 unnamed neutral rooms suggest the map was never fully built out.
- **paradise** — Contains 1 room with a null name.

### Unnamed rooms (minor)
- **playtesters** — 3 unnamed rooms out of 44
- **hack** — 1 unnamed room
- **shooter** — 1 unnamed room
