# Voluntary Respawn — Design

## Overview

Add a "Respawn" button above the "Leave Game" button. The player gets a 5-second cancellable countdown; on completion, all carried items are dropped at the current location and the player teleports to a new spawn point. No death or kill is recorded. All players receive a GM announcement.

## Protocol

- **New C→S message:** `VOLUNTARY_RESPAWN` (no payload)
- **Reused S→C messages:** `YOU_RESPAWNED` (location), `MESSAGE` (GM announcement), `PLAYER_INFO` (broadcast position change), `YOUR_INVENTORY` (empty inventory), `ITEMS_SYNC` / `ITEM_ADDED` (dropped items)

## Server (`session.ts`)

New handler for `VOLUNTARY_RESPAWN`:

1. Ignore if `player.dead` is true (already in a death-respawn cycle)
2. `dropPlayerItems(player)` — same spiral-scatter logic used on death
3. `randomSpawnForTeam(player.team)` — same spawn logic as death respawn
4. Update `player.room / x / y`; broadcast `PLAYER_INFO`
5. Send `YOU_RESPAWNED` to the player with new room/x/y
6. Broadcast GM `MESSAGE`: `"<name> chose to respawn."`
7. Send updated inventory (now empty) and stats to the player
8. **HP is not restored** — player keeps current HP

## Client (`index.html` + `main.ts`)

- New `<button id="respawn-btn">Respawn</button>` inserted above `#leave-btn` in `#game-btns`
- Same cancellable-countdown pattern as Leave Game:
  - First click: starts 5-second countdown, button text → "Respawn 5…"
  - Click again during countdown: cancels, button text → "Respawn"
  - Countdown reaches 0: sends `VOLUNTARY_RESPAWN` to server
- The two countdowns (respawn and leave) are independent; starting one does not cancel the other
- No tombstone / dead state shown — player teleports immediately on `YOU_RESPAWNED`
- Button only shown while in-game (same visibility rules as Leave Game)

## Constraints

- HP is **not** restored
- Deaths and kills stats are **not** incremented
- The drop logic is identical to the on-death drop (spiral search from current tile)
- The spawn logic is identical to the on-death respawn (`randomSpawnForTeam`)

## Testing

- Integration test: send `VOLUNTARY_RESPAWN`, verify items appear near old location, player moves to new location, GM message broadcast, no stat changes
- Verify `VOLUNTARY_RESPAWN` is ignored when player is already dead
