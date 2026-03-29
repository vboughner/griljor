# Help Screens + Key Remap Design

## Key Remapping

| Key | Old Action | New Action |
|-----|-----------|------------|
| `s` | Pickup item | Move south (duplicate of `x`) |
| `g` | (unbound) | Pickup item at feet |
| `b` | (unbound) | Drop active hand item |
| `Shift+Z` | Drop active hand item | (removed) |

## Help Modal

### Trigger
- **"Help" button** next to Respawn and Leave Game buttons in the right sidebar
- **`h`** key opens the help modal
- **Escape** or close button dismisses it

### Layout
- Centered modal overlay with dimmed background
- Two pages with Next/Prev navigation

### Page 1 — "How to Play"
- Movement: QWEASDZXC keys + right-click to walk toward a tile
- Combat: left-click to fire or use held weapon
- Picking up items: `g` key or left-click on floor items nearby
- Dropping items: `b` key drops held item
- Chat: `t` to type, Enter to send, Escape to cancel

### Page 2 — "Keyboard Shortcuts"
- Full 8-direction movement key diagram (QWEASDZXC, arrows, numpad)
- Item keys: `g` pickup, `b` drop
- Toggle keys: `v` fog, `p` pickups, `o` player boxes, `L` light/dark, `?` debug
- Mouse controls reference (left-click contextual actions, right-click to walk)
