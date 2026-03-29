# Single Active Hand + Dual Action Cards Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the two-hand system with a single active-item hand, make ammo auto-reload from inventory, and replace the XBM mouse widget with retro dual action cards.

**Architecture:** Protocol messages lose their `hand` parameter (there is only one hand now). The server's `Player` drops `rightHand`; all `rightHand` references become `leftHand`. Ammo reloading scans `player.inventory[0..34]` instead of the opposite hand. The client mouse widget HTML/CSS/TS is replaced with two styled cards.

**Tech Stack:** TypeScript (server + client), Vite (client), Vitest (tests), CSS in `index.html`

---

### Task 1: Update protocol types

**Files:**
- Modify: `server/src/protocol.ts`

**Step 1: Write the failing tests** (compile errors count as failures here — just note that the later tasks will fix them)

No dedicated test for this task — protocol changes are validated by TypeScript compilation and subsequent task tests.

**Step 2: Update C2SMessage**

Replace the five affected union members:

```typescript
| { type: 'PICKUP'; x: number; y: number }
| { type: 'DROP'; source: 'active' | number }
| { type: 'INV_SWAP'; slot: number }
| { type: 'FIRE_WEAPON'; targetX: number; targetY: number }
| { type: 'USE_ITEM'; targetX: number; targetY: number }
```

**Step 3: Update YOUR_INVENTORY in S2CMessage**

Remove `rightHand` field:

```typescript
| {
    type: 'YOUR_INVENTORY';
    leftHand: InventoryItem | null;
    inventory: Array<InventoryItem | null>;
    currentWeight: number;
    maxWeight: number;
  }
```

**Step 4: Verify TypeScript compiles (errors expected from callers — fix in later tasks)**

```bash
cd server && npx tsc --noEmit 2>&1 | head -40
```

**Step 5: Commit**

```bash
git add server/src/protocol.ts
git commit -m "protocol: remove hand params, drop rightHand from YOUR_INVENTORY"
```

---

### Task 2: Update server Player state and session handlers

**Files:**
- Modify: `server/src/session.ts`

This is the largest task. Work through the file top-to-bottom.

**Step 1: Remove `rightHand` from `Player` interface and initialization**

In the `Player` interface (around line 140), delete:
```typescript
rightHand: InventoryItem | null;
```

In the player construction object (around line 356), delete:
```typescript
rightHand: null,
```

**Step 2: Simplify `onPickup` (~line 696)**

Replace the hand-routing logic:
```typescript
// OLD: checks hand === 'left' or 'right'
// NEW: always goes to leftHand, or inventory if leftHand occupied
const handOccupied = player.leftHand !== null;

if (!handOccupied) {
  player.leftHand = item;
} else {
  const freeSlot = player.inventory.indexOf(null);
  if (freeSlot === -1) {
    this.send(player.ws, {
      type: 'MESSAGE',
      from: 0,
      name: 'GM',
      to: player.id,
      text: 'Your hands are full.',
    });
    return;
  }
  player.inventory[freeSlot] = item;
}
```

**Step 3: Simplify `onDrop` (~line 725)**

Replace `'left'` and `'right'` branches with a single `'active'` branch:
```typescript
if (msg.source === 'active') {
  item = player.leftHand;
  player.leftHand = null;
} else if (typeof msg.source === 'number' && msg.source >= 0 && msg.source < INV_SIZE) {
  item = player.inventory[msg.source];
  player.inventory[msg.source] = null;
}
```

**Step 4: Simplify `onInvSwap` (~line 763)**

```typescript
private onInvSwap(playerId: number, msg: Extract<C2SMessage, { type: 'INV_SWAP' }>): void {
  const player = this.players.get(playerId);
  if (!player) return;
  if (player.dead) return;
  if (msg.slot < 0 || msg.slot >= INV_SIZE) return;

  const slotItem = player.inventory[msg.slot];
  player.inventory[msg.slot] = player.leftHand;
  player.leftHand = slotItem;

  this.sendInventory(player);
}
```

**Step 5: Replace `tryReloadFromOtherHand` with `tryReloadFromInventory`**

Delete `tryReloadFromOtherHand` entirely (~line 838). Add:

```typescript
/**
 * When the active weapon has 0 charges, scan inventory[0..N] left-to-right
 * for compatible ammo (ammoObj.charges & weaponObj.type !== 0) and transfer
 * charges into the weapon. Consumes ammo items when depleted.
 */
private tryReloadFromInventory(player: Player): void {
  const weaponItem = player.leftHand;
  if (!weaponItem) return;
  const weaponObj = this.world.objects[weaponItem.type];
  if (!weaponObj?.numbered || !weaponObj.type) return;

  const capacity = weaponObj.capacity ?? Infinity;
  if (weaponItem.quantity >= capacity) return;

  for (let i = 0; i < player.inventory.length; i++) {
    const ammoItem = player.inventory[i];
    if (!ammoItem) continue;
    const ammoObj = this.world.objects[ammoItem.type];
    if (!ammoObj?.charges) continue;
    if ((ammoObj.charges & weaponObj.type) === 0) continue;

    const needed = capacity - weaponItem.quantity;
    const transfer = Math.min(ammoItem.quantity, needed);
    if (transfer <= 0) continue;

    weaponItem.quantity += transfer;

    if (!ammoObj.numbered) {
      player.currentWeight = Math.max(0, player.currentWeight - transfer * (ammoObj.weight ?? 0));
    }
    ammoItem.quantity -= transfer;

    if (ammoItem.quantity <= 0) {
      if (ammoObj.numbered) {
        player.currentWeight = Math.max(0, player.currentWeight - (ammoObj.weight ?? 0));
      }
      player.inventory[i] = null;
      // Pull next ammo of same type from further in inventory
      const nextSlot = player.inventory.findIndex((it, j) => j > i && it?.type === ammoItem.type);
      // (no-op: already consumed in-place; next fire will find the next slot)
      void nextSlot;
    }

    if (weaponItem.quantity >= capacity) break;
  }
}
```

**Step 6: Delete `tryReloadWeaponFromAmmo` (~line 788)**

This method handled the case of "ammo in a hand fires into weapon in other hand". That path is gone. Delete the entire method.

**Step 7: Simplify `autoReloadHand` (~line 967)**

Remove the `hand` parameter — it always reloads `leftHand`:

```typescript
private autoReloadHand(player: Player, itemType: number): void {
  const reloadSlot = player.inventory.findIndex(
    (item) => item !== null && item.type === itemType,
  );
  if (reloadSlot !== -1) {
    player.leftHand = player.inventory[reloadSlot];
    player.inventory[reloadSlot] = null;
  }
}
```

**Step 8: Simplify `onFireWeapon` (~line 978)**

- Remove the `msg.hand` variable — always use `player.leftHand`
- Replace `tryReloadFromOtherHand` calls with `tryReloadFromInventory(player)`
- Remove the "non-weapon with charges: treat as ammo use" branch (that was for firing ammo from right hand — irrelevant now)
- Fix `lost` item branch to use `player.leftHand = null` directly
- Fix `autoReloadHand` calls to drop the `hand` arg

Key diff (pseudocode):
```typescript
const handItem = player.leftHand;              // was: msg.hand === 'left' ? ... : ...
// ...
if (obj.numbered && handItem.quantity <= 0) {
  this.tryReloadFromInventory(player);          // was: tryReloadFromOtherHand(player, msg.hand)
  // ...
}
// ...
if (obj.numbered) {
  handItem.quantity--;
  if (handItem.quantity <= 0) {
    this.tryReloadFromInventory(player);        // was: tryReloadFromOtherHand(player, msg.hand)
  }
} else if (obj.lost) {
  player.currentWeight = Math.max(0, player.currentWeight - calcItemWeight(obj, handItem));
  player.leftHand = null;                       // was: if (msg.hand === 'left') ... else ...
  this.autoReloadHand(player, handItem.type);   // was: autoReloadHand(player, msg.hand, ...)
}
```

**Step 9: Simplify `onUseItem` (~line 1235)**

- `const handItem = player.leftHand;` (remove `msg.hand` check)
- All `if (msg.hand === 'left') player.leftHand = null; else player.rightHand = null;` → `player.leftHand = null;`
- `autoReloadHand(player, msg.hand, handItem.type)` → `autoReloadHand(player, handItem.type)`
- In opener branch: `if (handItem.quantity <= 0) { player.leftHand = null; }` (remove `msg.hand` check)

**Step 10: Simplify `dropPlayerItems` (~line 1395)**

```typescript
private dropPlayerItems(player: Player): void {
  const items: Array<InventoryItem | null> = [
    player.leftHand,
    ...player.inventory,
  ];
  player.leftHand = null;
  player.inventory.fill(null);
  player.currentWeight = 0;
  // ... rest unchanged
```

**Step 11: Simplify `sendInventory` (~line 1760)**

```typescript
private sendInventory(player: Player): void {
  this.send(player.ws, {
    type: 'YOUR_INVENTORY',
    leftHand: player.leftHand,
    inventory: player.inventory,
    currentWeight: player.currentWeight,
    maxWeight: MAX_WEIGHT,
  });
}
```

**Step 12: Compile check**

```bash
cd server && npx tsc --noEmit
```

Expected: no errors in `session.ts` (test files may still have errors — fix in Task 4).

**Step 13: Commit**

```bash
git add server/src/session.ts
git commit -m "server: single active hand, ammo reloads from inventory"
```

---

### Task 3: Update client network.ts

**Files:**
- Modify: `client/src/network.ts`

**Step 1: Update all send methods**

```typescript
sendPickup(x: number, y: number): void {
  this.send({ type: 'PICKUP', x, y });
}

sendDrop(source: 'active' | number): void {
  this.send({ type: 'DROP', source });
}

sendInvSwap(slot: number): void {
  this.send({ type: 'INV_SWAP', slot });
}

sendFireWeapon(targetX: number, targetY: number): void {
  this.send({ type: 'FIRE_WEAPON', targetX, targetY });
}

sendUseItem(targetX: number, targetY: number): void {
  this.send({ type: 'USE_ITEM', targetX, targetY });
}
```

**Step 2: Compile check**

```bash
cd client && npx tsc --noEmit 2>&1 | head -30
```

**Step 3: Commit**

```bash
git add client/src/network.ts
git commit -m "client: remove hand params from all network send methods"
```

---

### Task 4: Update client game.ts

**Files:**
- Modify: `client/src/game.ts`

**Step 1: Remove `rightHand` field**

In the class body, find:
```typescript
private leftHand: InventoryItem | null = null;
private rightHand: InventoryItem | null = null;
```
Delete the `rightHand` line.

**Step 2: Simplify the canvas `mousedown` handler**

Replace the entire handler body with the 2-button version:

```typescript
this.canvas.addEventListener('mousedown', (e) => {
  e.preventDefault();
  const rect = this.canvas.getBoundingClientRect();
  const tx = Math.floor((e.clientX - rect.left) / TILE) - 1;
  const ty = Math.floor((e.clientY - rect.top) / TILE) - 1;

  // Border click: right-click walks toward exit; left fires into next room
  if (tx < 0 || tx >= GRID || ty < 0 || ty >= GRID) {
    if (this.isDead) return;
    if (e.button === 2) {
      this.startMovingTo(tx, ty);
    } else if (e.button === 0) {
      this.network?.sendFireWeapon(tx, ty);
    }
    return;
  }

  if (e.button === 0) {
    if (this.isDead) return;
    const handObj = this.leftHand ? this.objects[this.leftHand.type] : null;
    const key = `${tx},${ty}`;
    const tileOccupied = [...this.otherPlayers.values()].some(
      (p) => p.room === this.currentRoom && p.x === tx && p.y === ty,
    );
    const room = this.mapData.rooms[this.currentRoom];
    const dist = Math.max(Math.abs(tx - this.px), Math.abs(ty - this.py));
    const hasDoorAtTile =
      dist === 1 &&
      room?.recorded_objects?.some(
        (ro) => ro.x === tx && ro.y === ty && (this.objects[ro.type]?.swings ?? false),
      );
    if (handObj?.opens && hasDoorAtTile) {
      this.network?.sendUseItem(tx, ty);
    } else if (!tileOccupied && this.floorItems.get(this.currentRoom)?.has(key)) {
      this.network?.sendPickup(tx, ty);
    } else if ((handObj?.health ?? 0) < 0) {
      this.network?.sendUseItem(this.px, this.py);
    } else if (handObj?.opens && dist === 1) {
      this.network?.sendUseItem(tx, ty);
    } else if (tx !== this.px || ty !== this.py) {
      this.network?.sendFireWeapon(tx, ty);
    }
    return;
  }

  if (e.button === 2) {
    if (this.isDead) return;
    this.startMovingTo(tx, ty);
  }
});
```

**Step 3: Simplify keyboard shortcuts**

Find and remove the `'S'` (shift+s → pick up to right hand) and `'X'` (shift+x → drop right hand) shortcuts. Update remaining shortcuts:

```typescript
// 's' — pick up to active hand
if (e.key === 's') {
  e.preventDefault();
  if (!this.isDead) this.network?.sendPickup(this.px, this.py);
  return;
}
// 'Z' — drop active hand item
if (e.key === 'Z') {
  e.preventDefault();
  if (!this.isDead) this.network?.sendDrop('active');
  return;
}
```

**Step 4: Simplify `setHands`**

```typescript
setHands(left: InventoryItem | null): void {
  this.leftHand = left;
}
```

**Step 5: Compile check**

```bash
cd client && npx tsc --noEmit 2>&1 | head -30
```

**Step 6: Commit**

```bash
git add client/src/game.ts
git commit -m "client: single hand canvas input, remove right-hand references"
```

---

### Task 5: Update client main.ts

**Files:**
- Modify: `client/src/main.ts`

**Step 1: Remove right-hand tracking and tooltip**

Delete the `currentRightHand` variable and the `hand-middle-canvas` tooltip setup in `initHandTooltips`. Keep only `currentLeftHand` and `hand-left-canvas`.

**Step 2: Update inventory cell click handlers**

In `buildInvGrid`, replace the `mousedown` handler:

```typescript
cell.addEventListener('mousedown', (e) => {
  e.preventDefault();
  if (!invNetwork) return;
  if (e.button === 0) {
    invNetwork.sendInvSwap(i);    // swap with active hand
  }
  // right-click handled by contextmenu
});
cell.addEventListener('contextmenu', (e) => {
  e.preventDefault();
  if (!invNetwork) return;
  invNetwork.sendDrop(i);         // drop from inventory slot
});
```

**Step 3: Update `updateInventory` call signature**

Find the function that receives `YOUR_INVENTORY` and update it:
- Remove `rightHand` from the destructured message
- Remove `setHandItem('right', ...)` call
- Remove `hand-middle-count` update
- Remove `handMiddleCanvas` click/contextmenu handlers
- Update `game.setHands(msg.leftHand)` (was `setHands(msg.leftHand, msg.rightHand)`)

**Step 4: Update hand slot click handlers**

`hand-left-canvas` click/contextmenu should send `sendDrop('active')` (was `sendDrop('left')`).
Remove all `hand-middle-canvas` handler setup.

**Step 5: Compile check**

```bash
cd client && npx tsc --noEmit
```

Expected: clean.

**Step 6: Commit**

```bash
git add client/src/main.ts
git commit -m "client: remove right-hand UI wiring, update inv/hand click handlers"
```

---

### Task 6: Build dual action cards UI

**Files:**
- Modify: `client/index.html`
- Modify: `client/src/mouse-widget.ts` (repurpose as `action-cards` logic)
- Modify: `client/src/main.ts` (update import)

**Step 1: Replace the mouse widget HTML in `index.html`**

Find and remove the `#mouse-widget` div (with `#mouse-bitmap-wrap`, all three hand canvases and count spans). Replace with:

```html
<div id="action-cards">
  <div class="action-card" id="card-use">
    <span class="card-badge lmb">LMB</span>
    <canvas id="active-item-icon" width="32" height="32"></canvas>
    <span id="active-item-name">nothing</span>
  </div>
  <div class="action-card" id="card-move">
    <span class="card-badge rmb">RMB</span>
    <canvas id="move-icon" width="32" height="32"></canvas>
    <span class="card-label">MOVE</span>
  </div>
</div>
```

**Step 2: Add CSS for the action cards**

Add to the `<style>` block in `index.html` (in the `/* ── Mouse widget ──*/` area, replacing old mouse widget styles):

```css
/* ── Action Cards ── */
#action-cards {
  display: flex;
  gap: 8px;
  margin-bottom: 10px;
}
.action-card {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 6px 4px 5px;
  background: #1a1a1a;
  border: 1px solid #333;
  border-radius: 3px;
  position: relative;
  min-width: 0;
}
#card-use  { border-color: #7a5a10; box-shadow: inset 0 0 8px #3a2a0040; }
#card-move { border-color: #1a3a5a; box-shadow: inset 0 0 8px #0a1a2a40; }
.card-badge {
  position: absolute;
  top: 3px;
  left: 4px;
  font-size: 8px;
  font-weight: bold;
  letter-spacing: 0.5px;
  padding: 1px 3px;
  border-radius: 2px;
  line-height: 1.4;
}
.lmb { background: #7a5a10; color: #ffc; }
.rmb { background: #1a3a5a; color: #adf; }
.action-card canvas { margin-top: 14px; display: block; }
#active-item-name, .card-label {
  font-size: 9px;
  color: #888;
  text-align: center;
  margin-top: 3px;
  max-width: 64px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}
#active-item-name { color: #bba060; }
```

**Step 3: Rewrite `mouse-widget.ts`**

Replace the entire file with action-card logic:

```typescript
import movexbm from '../../pipeline/bit/movemark?raw';

const SLOT_W = 32;
const SLOT_H = 32;

function parseXbmBytes(raw: string): number[] {
  const match = raw.match(/\{([\s\S]*)\}/);
  if (!match) throw new Error('Invalid XBM');
  return match[1]
    .split(',')
    .map((s) => s.trim())
    .filter((s) => /^0x/i.test(s))
    .map((s) => parseInt(s, 16));
}

function drawMoveIcon(canvas: HTMLCanvasElement): void {
  const bytes = parseXbmBytes(movexbm);
  const ctx = canvas.getContext('2d')!;
  const img = ctx.createImageData(SLOT_W, SLOT_H);
  for (let y = 0; y < SLOT_H; y++) {
    for (let x = 0; x < SLOT_W; x++) {
      const idx = y * SLOT_W + x;
      const isSet = (bytes[idx >> 3] >> (idx & 7)) & 1;
      const p = idx * 4;
      img.data[p] = 150; img.data[p + 1] = 200; img.data[p + 2] = 255;
      img.data[p + 3] = isSet ? 0 : 200;
    }
  }
  ctx.putImageData(img, 0, 0);
}

export function initActionCards(): void {
  drawMoveIcon(document.getElementById('move-icon') as HTMLCanvasElement);
}

export function setActiveItem(imgData: ImageData | null, name: string | null): void {
  const canvas = document.getElementById('active-item-icon') as HTMLCanvasElement;
  const nameEl = document.getElementById('active-item-name') as HTMLElement;
  const ctx = canvas.getContext('2d')!;
  ctx.clearRect(0, 0, SLOT_W, SLOT_H);
  if (imgData) {
    const tmp = new OffscreenCanvas(imgData.width, imgData.height);
    tmp.getContext('2d')!.putImageData(imgData, 0, 0);
    ctx.drawImage(tmp, 0, 0, SLOT_W, SLOT_H);
  }
  nameEl.textContent = name ?? 'nothing';
}
```

**Step 4: Update `main.ts` import**

Replace:
```typescript
import { initMouseWidget, setHandItem } from './mouse-widget';
```
With:
```typescript
import { initActionCards, setActiveItem } from './mouse-widget';
```

Update call sites:
- `initMouseWidget()` → `initActionCards()`
- `setHandItem('left', imgData)` → `setActiveItem(imgData, leftHandName)`

Where `leftHandName` is the name from the objects lookup for the current left hand item (or `null` for empty).

Also remove all `setHandItem('right', ...)` calls, `setHandItems(...)` helper if present, and hand-count span updates for the middle slot.

**Step 5: Build check**

```bash
cd client && npm run build 2>&1 | tail -20
```

**Step 6: Commit**

```bash
git add client/index.html client/src/mouse-widget.ts client/src/main.ts
git commit -m "client: dual action cards UI replacing XBM mouse widget"
```

---

### Task 7: Update tests

**Files:**
- Modify: `server/src/__tests__/integration/inventory.test.ts`
- Modify: `server/src/__tests__/integration/combat.test.ts`

**Step 1: Fix inventory tests**

- Change all `{ type: 'PICKUP', x, y, hand: 'left' }` → `{ type: 'PICKUP', x, y }`
- Change all `{ type: 'PICKUP', x, y, hand: 'right' }` → `{ type: 'PICKUP', x, y }` (these now go to leftHand or inventory)
- Change `{ type: 'DROP', source: 'left' }` → `{ type: 'DROP', source: 'active' }`
- Change `{ type: 'INV_SWAP', slot: 0, hand: 'right' }` → `{ type: 'INV_SWAP', slot: 0 }`
- Remove all assertions on `inv.rightHand` — there is no rightHand field anymore
- Update the `INV_SWAP` test: previously it tested swapping into rightHand; rewrite to test swapping slot with active hand (leftHand)

Example rewrite of the INV_SWAP test:
```typescript
it('INV_SWAP moves item from inventory slot to active hand', () => {
  const alice = joinPlayer(session, 'Alice');
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
  alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 }); // sword → leftHand
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 6 });
  alice.ws.receive({ type: 'PICKUP', x: 6, y: 6 }); // potion → inventory[0]

  alice.ws.flush();
  alice.ws.receive({ type: 'INV_SWAP', slot: 0 }); // swap: potion ↔ sword

  const inv = alice.ws.lastOfType('YOUR_INVENTORY');
  expect(inv!.leftHand).not.toBeNull();
  expect(inv!.leftHand!.type).toBe(3); // potion now in hand
  expect(inv!.inventory[0]?.type).toBe(2); // sword now in slot 0
});
```

**Step 2: Fix ammo/reload combat tests**

The `buildAmmoWorld` function and tests in the `'ammo reload'` describe block need updating. The arrows now live in inventory, not the right hand.

Update `armAliceWithBowAndArrows`:
```typescript
function armAliceWithBowAndArrows(alice: TestPlayer) {
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
  alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 }); // bow → leftHand
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 5 });
  alice.ws.receive({ type: 'PICKUP', x: 6, y: 5 }); // arrows (qty=10) → inventory[0]
}
```

Rewrite the reload tests. The new expected behavior: fire bow (qty=1 → 0), server auto-reloads from inventory[0] (arrows, qty=10) → bow=5, arrows=5 remains in inventory[0].

```typescript
it('auto-reloads weapon from inventory when weapon empties mid-fire', () => {
  const alice = joinPlayer(session, 'Alice');
  armAliceWithBowAndArrows(alice);
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
  alice.ws.flush();

  alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

  const inv = alice.ws.lastOfType('YOUR_INVENTORY');
  expect(inv).toBeDefined();
  expect(inv!.leftHand?.type).toBe(2); // bow still in hand
  expect(inv!.leftHand?.quantity).toBe(5); // reloaded to capacity
  expect(inv!.inventory[0]?.quantity).toBe(5); // arrows partially consumed
});

it('weapon stays in hand when empty with no ammo in inventory', () => {
  const alice = joinPlayer(session, 'Alice');
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
  alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 }); // bow (qty=1), no arrows picked up
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
  alice.ws.flush();

  alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

  const inv = alice.ws.lastOfType('YOUR_INVENTORY');
  expect(inv!.leftHand?.type).toBe(2);
  expect(inv!.leftHand?.quantity).toBe(0); // empty, no reload possible
});

it('reloads from slot 0 before slot 1', () => {
  const alice = joinPlayer(session, 'Alice');
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });
  alice.ws.receive({ type: 'PICKUP', x: 5, y: 5 }); // bow → leftHand
  // Pick up arrows at (6,5) into slot 0, and extra arrows at (7,5) into slot 1
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 6, y: 5 });
  alice.ws.receive({ type: 'PICKUP', x: 6, y: 5 }); // 10 arrows → inventory[0]
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 7, y: 5 });
  alice.ws.receive({ type: 'PICKUP', x: 7, y: 5 }); // 5 arrows → inventory[1]
  alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 1, y: 1 });
  alice.ws.flush();

  alice.ws.receive({ type: 'FIRE_WEAPON', targetX: 10, targetY: 10 });

  const inv = alice.ws.lastOfType('YOUR_INVENTORY');
  // Slot 0 arrows (qty=10) used to reload, not slot 1 (qty=5)
  expect(inv!.inventory[0]?.quantity).toBe(5);   // slot 0 consumed 5 to fill bow
  expect(inv!.inventory[1]?.quantity).toBe(5);   // slot 1 untouched
});
```

Remove the `'active ammo use (FIRE_WEAPON with ammo hand)'` test — that code path (firing ammo from hand to reload weapon in other hand) no longer exists.

**Step 3: Run the full test suite**

```bash
cd /path/to/repo && npm test
```

Expected: all tests pass.

**Step 4: Commit**

```bash
git add server/src/__tests__/integration/inventory.test.ts \
        server/src/__tests__/integration/combat.test.ts
git commit -m "tests: update inventory and ammo-reload tests for single-hand system"
```

---

### Task 8: Lint, format, verify

**Step 1: Lint and format both packages**

```bash
cd server && npm run lint && npm run format:check
cd ../client && npm run lint && npm run format:check
```

Fix any issues with `npm run lint -- --fix` and `npm run format`.

**Step 2: Full test suite**

```bash
cd .. && npm test
```

All tests must pass.

**Step 3: Final commit (if any lint/format fixes)**

```bash
git add -p
git commit -m "chore: lint and format fixes for single-hand refactor"
```
