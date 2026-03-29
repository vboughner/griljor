# Design: Single Active Hand + Dual Action Cards

**Date:** 2026-03-29

## Motivation

The current two-hand system requires a middle mouse button for the right hand, making
the game unplayable with a standard 2-button mouse. This redesign eliminates the right
hand entirely, makes the left hand the single "active item", and replaces the mouse
widget bitmap with a clear dual action card UI.

## Server Changes

### Player state
- Remove `rightHand` field from `Player`. Only `leftHand` (the active item) remains.

### Reload logic
- Remove `tryReloadFromOtherHand`. Replace with `tryReloadFromInventory`: when the
  active weapon hits 0 charges, scan `inventory[0..34]` left-to-right (topmost row,
  leftmost column first) for an item whose `charges` bitmask is compatible with the
  weapon's `type` field (`charges & weaponObj.type !== 0`), then transfer charges using
  the existing transfer logic.
- `autoReloadHand` (refills the hand after a `lost` item is consumed) is kept but
  operates on `leftHand` only.

## Protocol Changes

All `hand: 'left' | 'right'` parameters are removed or simplified — there is only one
hand now.

| Message | Change |
|---------|--------|
| `PICKUP` | Remove `hand` field |
| `DROP` | `source: 'active' \| number` — `'active'` drops the held item, number drops from that inventory slot |
| `INV_SWAP` | Remove `hand` field (always swaps with active hand) |
| `FIRE_WEAPON` | Remove `hand` field |
| `USE_ITEM` | Remove `hand` field |
| `YOUR_INVENTORY` | Remove `rightHand` field |

## Client Changes

### Input
- Canvas left-click: use active item (fire / pickup / open door) — behavior unchanged
- Canvas right-click: move — behavior unchanged
- All middle-click handlers removed (canvas, inventory cells)
- Inventory cell left-click: swap with active hand
- Inventory cell right-click: drop (unchanged)

### UI: Dual Action Cards
Replace the `#mouse-widget` (XBM bitmap + overlay canvases) with two side-by-side
retro-bordered action cards.

**Left card — active item**
- `[LMB]` badge in top-left corner (amber accent)
- Active item icon (32×32) centered
- Item name below icon (truncated if needed)
- Empty state: fist icon + label "nothing"
- Updates live whenever `YOUR_INVENTORY` is received

**Right card — move**
- `[RMB]` badge in top-left corner (grey/blue accent)
- Pixel-art 4-way directional arrow icon (drawn on canvas or CSS)
- Label "MOVE" below
- Static — never changes

**Styling**
- Dark background matching the existing sidebar palette (`#1a1a1a` / `#222`)
- 1px inset border with subtle colour tint per card (amber left, steel-blue right)
- Small caps label font consistent with existing panel headers
- Cards sit where the mouse widget currently sits, same vertical position in the sidebar

### Files affected (client)
- `index.html`: replace `#mouse-widget` markup
- `mouse-widget.ts`: replace with new `action-cards.ts` (or rewrite in place)
- `main.ts`: update `setHandItem` / `setHandItems` calls, remove right-hand tooltip setup, remove middle-click handlers on inventory cells and hand canvases
- `game.ts`: remove middle-click handler on canvas, remove `msg.hand` from all `network.send*` calls

## Tests

- Update all inventory integration tests to remove right-hand references
- Regression: fire weapon to 0 charges → auto-reloads from inventory slot 0 before slot 1
- Regression: `DROP { source: 'active' }` drops the held item correctly
- Regression: `INV_SWAP` without a hand field swaps inventory slot with active hand
