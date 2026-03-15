# Griljor Object Property Reference

Properties that can appear on objects in `pipeline/out/data/objects/*.json`.
Each map references its own object file — see `docs/implementation-notes.md` for the per-map mapping.

---

## Identity & Display

| Property | Type | Description |
|----------|------|-------------|
| `name` | string | Human-readable name shown in inventory and examine messages |
| `type` | integer | Bitmask grouping objects into categories (used by `opens`, `ignoring`, `piercing`, `destroys`) |
| `color` | string | X11 color name used when rendering without a bitmap |
| `masked` | boolean | Object has a mask bitmap; transparent pixels are skipped when drawing |
| `recorded` | boolean | Object uses per-instance variables (`detail`, `infox`, `infoy`, etc. in recorded_objects) |

---

## Messages

| Property | Type | Description |
|----------|------|-------------|
| `lookmsg` | string | Shown to player when looking at the object from a distance |
| `examinemsg` | string | Shown when examining the object while holding it |
| `usemsg` | string | Shown to the user when they use the object |
| `roommsg` | string | Shown to other players in the same room when the object is used |
| `allmsg` | string | Shown to all players in the game when the object is used |
| `hitmsg` | string | Shown to the attacker when a missile from this weapon hits someone |
| `victimmsg` | string | Shown to the player who is hit by a missile from this weapon |
| `usealtermsg` | string | Shown to the user when the object swings/alternates state (e.g. opens a door) |
| `roomaltermsg` | string | Shown to other players in the room when the object alternates state |
| `allaltermsg` | string | Shown to all players in the game when the object alternates state |

---

## Movement & Navigation

| Property | Type | Description |
|----------|------|-------------|
| `movement` | integer | Player walkability: absent or 0 = impassable; 1–9 = walkable at that speed (9 = fastest) |
| `exit` | boolean | Tile is an exit; destination room/coordinates are stored in the recorded_object instance |
| `permeable` | boolean | Missiles can pass through this tile (does NOT affect player movement) |
| `transparent` | boolean | Players can see through this object |
| `concealing` | boolean | Players standing on this tile are hidden from others |

---

## Visibility & Lighting

| Property | Type | Description |
|----------|------|-------------|
| `glows` | boolean | Object is visible in dark rooms without a light source |
| `flashlight` | integer | Radius (in tiles) that this object illuminates when held in a dark room |

---

## Pickup & Carrying

| Property | Type | Description |
|----------|------|-------------|
| `takeable` | boolean | Players can pick this object up from the floor |
| `weight` | integer | Encumbrance weight when carried in inventory |
| `lost` | boolean | Object is consumed/destroyed when used or fired |
| `important` | boolean | Server tracks this object type specially (e.g. flags for win conditions) |

---

## Weapons & Firing

| Property | Type | Description |
|----------|------|-------------|
| `weapon` | boolean | Object can be wielded and fired/thrown |
| `range` | integer | Maximum tiles a fired missile travels |
| `movingobj` | integer | Object ID of the missile created when this weapon fires |
| `refire` | integer | Refire speed modifier (affects how quickly the weapon can fire again) |
| `numbered` | boolean | Weapon has a finite charge count tracked in `quantity`; depletes on each shot |
| `capacity` | integer | Maximum number of charges the weapon can hold |
| `stop` | boolean | Missile stops at the target tile rather than traveling in a direction indefinitely |
| `arc` | integer | Angular width of the firing arc in degrees (0 = straight line) |
| `spread` | integer | Number of missiles fired within the arc |
| `fan` | integer | Random angular deviation applied to each fired missile |

---

## Missiles & Projectiles

| Property | Type | Description |
|----------|------|-------------|
| `directional` | boolean | Missile uses direction-specific bitmaps depending on travel angle |
| `speed` | integer | How fast the object moves as a missile or explosion |
| `damage` | integer | Hit points of damage dealt when missile strikes a player |
| `mowdown` | boolean | Missile passes through players (wounding each), rather than stopping on first hit |
| `explodes` | integer | Non-zero: missile explodes at end of travel; value controls explosion radius/style |
| `boombit` | integer | Object ID this one transforms into when it explodes |
| `flammable` | integer | Non-zero radius for a secondary explosion triggered on impact |

---

## Missile vs Environment

| Property | Type | Description |
|----------|------|-------------|
| `vulnerable` | boolean | Object can be destroyed by explosions whose `destroys` bitmask matches this object's `type` |
| `destroyed` | integer | Object ID this becomes after being blown up |
| `ignoring` | integer | Bitmask of object types (by `type` field) this missile passes through without triggering |
| `piercing` | integer | Bitmask of wall types this missile can penetrate |
| `destroys` | integer | Bitmask of object types this missile destroys if they are `vulnerable` |

---

## Doors & Alternating State

| Property | Type | Description |
|----------|------|-------------|
| `swings` | boolean | Object can be toggled open/closed; its `type` identifies what kind of opener is required |
| `alternate` | integer | Object ID this becomes when toggled (e.g. open door ↔ closed door) |
| `opens` | integer | Bitmask of object `type` values this object can open/toggle when used |
| `hasid` | boolean | Instance has an ID stored for matching against `opens` (used for keyed locks) |

---

## Flags & Win Conditions

| Property | Type | Description |
|----------|------|-------------|
| `flag` | boolean | Object is a game flag; collecting or placing it can trigger a win condition |
| `flagteams` | integer | Bitmask of which teams must possess this flag to win |

---

## Usage Requirements

> **Note:** The modern rewrite does not use player levels, XP, or class systems. All level-based and XP-based restrictions (`maxlevel`, for example) are ignored. Any object is usable by any player regardless of these fields. Only the properties listed below have any current relevance, and even those are not yet enforced. Also, we don't keep track of mana in the modern rewrite either.

| Property | Type | Description |
|----------|------|-------------|
| `health` | integer | HP cost per use (negative value = heals the player) |
| `mana` | integer | Magic point cost per use (negative value = restores mana) - ignored |
| `strength` | integer | Minimum health percentage required to use |
| `maxlevel` | integer | Maximum player level that may use this object (0 = no limit) — ignored |
| `needanother` | boolean | Player must hold a specific object in the other hand to use this one |
| `otherobj` | integer | Object ID required in the other hand (paired with `needanother`) |

---

## Per-Instance Variables (`def` / `set`)

Recorded objects can store up to 7 instance-specific integers. These are used for things like ammo counts, door state, or exit destinations.

| Property | Type | Description |
|----------|------|-------------|
| `def0`–`def6` | integer | Default value for instance variable slot 0–6 (e.g. `def0` = initial ammo count) |
| `set0`–`set6` | boolean | Present when the corresponding `defN` value should be applied to new instances |
