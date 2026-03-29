# Help Screens + Key Remap Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a two-page help modal and remap pickup/drop keys so QWEASDZXC is a clean 8-directional movement block.

**Architecture:** Key remaps are single-line edits in `game.ts`. The help modal is pure DOM — a `<div id="help-modal">` overlay in `index.html` with CSS styling, toggled by a Help button and `h` key. Two pages swapped via JS in `main.ts`. No new dependencies.

**Tech Stack:** TypeScript, HTML/CSS (inline in `index.html`), Vitest (client tests)

---

### Task 1: Remap keys — `s` to south, `g` to pickup, `b` to drop

**Files:**
- Modify: `client/src/game.ts:158-173` (add `s` to movement keyDirs)
- Modify: `client/src/game.ts:234-244` (change pickup from `s` to `g`, drop from `Z` to `b`)

**Step 1: Write failing tests**

Create `client/src/__tests__/keybindings.test.ts`:

```typescript
import { describe, it, expect } from 'vitest';

/**
 * These tests verify the keyDirs and item-action key mappings by importing
 * nothing — they assert against the source text of game.ts to confirm the
 * key bindings are wired correctly.  A more robust approach would extract
 * the key maps into a shared constant, but YAGNI — the source-grep is
 * sufficient for a config-level change.
 */
import { readFileSync } from 'fs';
import { resolve } from 'path';

const src = readFileSync(resolve(__dirname, '../game.ts'), 'utf-8');

describe('key bindings', () => {
  it('maps s to south movement', () => {
    // s should appear in the keyDirs block as a movement key
    expect(src).toMatch(/keyDirs[\s\S]*?s.*\[0,\s*1\]/);
  });

  it('uses g for pickup', () => {
    expect(src).toMatch(/e\.key === 'g'/);
    expect(src).toMatch(/sendPickup/);
  });

  it('uses b for drop', () => {
    expect(src).toMatch(/e\.key === 'b'/);
    expect(src).toMatch(/sendDrop/);
  });

  it('does not use s for pickup', () => {
    // s should NOT appear as a pickup trigger
    expect(src).not.toMatch(/e\.key === 's'[\s\S]*?sendPickup/);
  });

  it('does not use Shift+Z for drop', () => {
    expect(src).not.toMatch(/e\.key === 'Z'[\s\S]*?sendDrop/);
  });
});
```

**Step 2: Run tests to verify they fail**

Run: `cd client && npx vitest run src/__tests__/keybindings.test.ts`
Expected: FAIL — `s` is still pickup, `g`/`b` not present

**Step 3: Edit game.ts — add `s` to movement, change pickup to `g`, drop to `b`**

In `client/src/game.ts` around line 158-173, add `s: [0, 1]` to `keyDirs`:

```typescript
      const keyDirs: Record<string, [number, number]> = {
        // Arrow keys
        ArrowUp: [0, -1],
        ArrowDown: [0, 1],
        ArrowRight: [1, 0],
        ArrowLeft: [-1, 0],
        // QWERTY 8-directional (original layout)
        q: [-1, -1],
        w: [0, -1],
        e: [1, -1],
        a: [-1, 0],
        s: [0, 1],
        d: [1, 0],
        z: [-1, 1],
        x: [0, 1],
        c: [1, 1],
      };
```

Around lines 234-244, change pickup key from `s` to `g` and drop from `Z` to `b`:

```typescript
      // Item actions
      if (e.key === 'g') {
        e.preventDefault();
        if (!this.isDead) this.network?.sendPickup(this.px, this.py);
        return;
      }
      if (e.key === 'b') {
        e.preventDefault();
        if (!this.isDead) this.network?.sendDrop('active');
        return;
      }
```

**Step 4: Run tests to verify they pass**

Run: `cd client && npx vitest run src/__tests__/keybindings.test.ts`
Expected: PASS

**Step 5: Run lint and format**

Run: `cd client && npm run lint && npm run format:check`
Expected: PASS (fix any issues with `npm run format` if needed)

**Step 6: Commit**

```bash
git add client/src/game.ts client/src/__tests__/keybindings.test.ts
git commit -m "feat: remap s to south movement, g for pickup, b for drop"
```

---

### Task 2: Add Help button to sidebar

**Files:**
- Modify: `client/index.html:488-504` (add `#help-btn` CSS)
- Modify: `client/index.html:588-591` (add Help button to `#game-btns`)

**Step 1: Add Help button HTML**

In `client/index.html` around line 590, add a Help button after the Leave Game button inside `#game-btns`:

```html
          <div id="game-btns">
            <button id="respawn-btn">Respawn</button>
            <button id="leave-btn">Leave Game</button>
            <button id="help-btn">Help</button>
          </div>
```

**Step 2: Add Help button CSS**

Add after the `#respawn-btn:disabled` rule (around line 505), styling that matches the existing button theme:

```css
    #help-btn {
      background: #1e1910; border: 1px solid #52380e; color: #b8842a;
      padding: 5px 10px; cursor: pointer; font-family: monospace; font-size: 13px;
      transition: background 0.1s, border-color 0.1s; width: 100%;
    }
    #help-btn:hover { background: #272212; border-color: #7a5220; color: #d4a040; }
```

**Step 3: Verify visually**

Run: `cd client && npm run dev` — open browser, join a game, confirm Help button appears below Leave Game.

**Step 4: Commit**

```bash
git add client/index.html
git commit -m "feat: add Help button to game sidebar"
```

---

### Task 3: Add help modal HTML and CSS

**Files:**
- Modify: `client/index.html` (add modal markup at end of `#game-screen`, add modal CSS)

**Step 1: Add modal CSS**

Add to the `<style>` block (before closing `</style>` tag around line 507):

```css
    /* Help modal */
    #help-modal {
      display: none; position: fixed; inset: 0; z-index: 1000;
      background: rgba(0,0,0,0.7); justify-content: center; align-items: center;
    }
    #help-modal.open { display: flex; }
    #help-modal-inner {
      background: #1a1a1a; border: 2px solid #52380e; border-radius: 8px;
      padding: 24px 32px; max-width: 520px; width: 90%; color: #ccc;
      font-family: monospace; position: relative; max-height: 80vh; overflow-y: auto;
    }
    #help-modal h2 { color: #d4a040; margin-bottom: 12px; font-size: 1.2em; }
    #help-modal h3 { color: #b8842a; margin: 12px 0 6px; font-size: 1em; }
    #help-modal p, #help-modal li { font-size: 0.9em; line-height: 1.5; }
    #help-modal ul { list-style: none; padding: 0; }
    #help-modal li { padding: 2px 0; }
    #help-modal kbd {
      background: #2a2a2a; border: 1px solid #555; border-radius: 3px;
      padding: 1px 6px; font-family: monospace; font-size: 0.9em; color: #e0c080;
    }
    #help-modal-nav {
      display: flex; justify-content: space-between; align-items: center;
      margin-top: 16px; padding-top: 12px; border-top: 1px solid #333;
    }
    #help-modal-nav button {
      background: #1e1910; border: 1px solid #52380e; color: #b8842a;
      padding: 4px 14px; cursor: pointer; font-family: monospace; font-size: 0.85em;
      border-radius: 3px;
    }
    #help-modal-nav button:hover { background: #272212; border-color: #7a5220; color: #d4a040; }
    #help-modal-nav button:disabled { opacity: 0.3; cursor: default; }
    #help-close {
      position: absolute; top: 8px; right: 12px; background: none; border: none;
      color: #888; font-size: 1.4em; cursor: pointer; font-family: monospace;
    }
    #help-close:hover { color: #d4a040; }
    #help-page-num { color: #888; font-size: 0.85em; }
```

**Step 2: Add modal HTML**

Add just before the closing `</div>` of `#game-screen` (find the game-screen container):

```html
      <div id="help-modal">
        <div id="help-modal-inner">
          <button id="help-close">&times;</button>
          <div id="help-page-1" class="help-page">
            <h2>How to Play</h2>
            <h3>Movement</h3>
            <p>Use <kbd>W</kbd><kbd>A</kbd><kbd>S</kbd><kbd>D</kbd> or arrow keys to move. Diagonal keys: <kbd>Q</kbd><kbd>E</kbd><kbd>Z</kbd><kbd>C</kbd>. Right-click a tile to walk toward it.</p>
            <h3>Combat</h3>
            <p>Left-click on the map to fire your weapon or use a held item. Pick up weapons from the ground to arm yourself.</p>
            <h3>Items</h3>
            <p>Press <kbd>G</kbd> to pick up an item at your feet (or left-click it). Press <kbd>B</kbd> to drop what you're holding. Click inventory slots to equip items.</p>
            <h3>Chat</h3>
            <p>Press <kbd>T</kbd> to type a message, <kbd>Enter</kbd> to send, <kbd>Escape</kbd> to cancel.</p>
          </div>
          <div id="help-page-2" class="help-page" style="display:none">
            <h2>Keyboard Shortcuts</h2>
            <h3>Movement (8 directions)</h3>
            <ul>
              <li><kbd>Q</kbd> <kbd>W</kbd> <kbd>E</kbd> &mdash; NW, North, NE</li>
              <li><kbd>A</kbd> &ensp;&bull;&ensp; <kbd>D</kbd> &mdash; West, East</li>
              <li><kbd>Z</kbd> <kbd>S</kbd>/<kbd>X</kbd> <kbd>C</kbd> &mdash; SW, South, SE</li>
            </ul>
            <p>Arrow keys and numpad also work.</p>
            <h3>Items</h3>
            <ul>
              <li><kbd>G</kbd> &mdash; Pick up item at your feet</li>
              <li><kbd>B</kbd> &mdash; Drop held item</li>
            </ul>
            <h3>Toggles</h3>
            <ul>
              <li><kbd>V</kbd> &mdash; Fog of war overlay</li>
              <li><kbd>P</kbd> &mdash; Highlight nearby pickups</li>
              <li><kbd>O</kbd> &mdash; Player indicator boxes</li>
              <li><kbd>L</kbd> &mdash; Light / dark mode</li>
              <li><kbd>?</kbd> &mdash; Tile debug hover</li>
            </ul>
            <h3>Mouse</h3>
            <ul>
              <li><strong>Left-click</strong> &mdash; Fire weapon / pick up / use item</li>
              <li><strong>Right-click</strong> &mdash; Walk toward tile</li>
            </ul>
          </div>
          <div id="help-modal-nav">
            <button id="help-prev" disabled>&larr; Prev</button>
            <span id="help-page-num">1 / 2</span>
            <button id="help-next">Next &rarr;</button>
          </div>
        </div>
      </div>
```

**Step 3: Commit**

```bash
git add client/index.html
git commit -m "feat: add help modal HTML and CSS"
```

---

### Task 4: Wire help modal toggle logic

**Files:**
- Modify: `client/src/main.ts` (add help modal open/close/page logic)

**Step 1: Add help modal JS**

In `main.ts`, after the existing `getElementById` calls for `leaveBtn`/`respawnBtn` (around line 241), add:

```typescript
  const helpBtn = document.getElementById('help-btn') as HTMLButtonElement;
  const helpModal = document.getElementById('help-modal') as HTMLDivElement;
  const helpClose = document.getElementById('help-close') as HTMLButtonElement;
  const helpPrev = document.getElementById('help-prev') as HTMLButtonElement;
  const helpNext = document.getElementById('help-next') as HTMLButtonElement;
  const helpPageNum = document.getElementById('help-page-num') as HTMLSpanElement;
  const helpPages = document.querySelectorAll<HTMLDivElement>('.help-page');
  let helpPage = 0;

  function showHelpPage(i: number): void {
    helpPage = i;
    helpPages.forEach((p, idx) => (p.style.display = idx === i ? '' : 'none'));
    helpPrev.disabled = i === 0;
    helpNext.disabled = i === helpPages.length - 1;
    helpPageNum.textContent = `${i + 1} / ${helpPages.length}`;
  }

  function openHelp(): void {
    helpModal.classList.add('open');
    showHelpPage(0);
  }

  function closeHelp(): void {
    helpModal.classList.remove('open');
  }

  helpBtn.addEventListener('click', openHelp);
  helpClose.addEventListener('click', closeHelp);
  helpModal.addEventListener('click', (e) => {
    if (e.target === helpModal) closeHelp();
  });
  helpPrev.addEventListener('click', () => showHelpPage(helpPage - 1));
  helpNext.addEventListener('click', () => showHelpPage(helpPage + 1));
```

**Step 2: Add `h` key handler and Escape-to-close**

In the existing `window.addEventListener('keydown', ...)` block in `main.ts` (around line 516-528), add the `h` key to open help and Escape to close it:

```typescript
  window.addEventListener('keydown', (e) => {
    if (gameScreen.style.display === 'none') return;
    // Escape closes help modal if open
    if (e.key === 'Escape' && helpModal.classList.contains('open')) {
      e.preventDefault();
      closeHelp();
      return;
    }
    const tag = (document.activeElement as HTMLElement)?.tagName;
    if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;
    if (e.key === 'h') {
      e.preventDefault();
      if (helpModal.classList.contains('open')) closeHelp();
      else openHelp();
      return;
    }
    if (e.key === 't') {
      e.preventDefault();
      chatInput.focus();
    }
    if (e.key === 'L') {
      e.preventDefault();
      void toggleMode();
    }
  });
```

**Step 3: Verify manually**

Run: `cd client && npm run dev` — open browser, join game, press `h` or click Help. Verify:
- Modal appears centered with dimmed background
- Page 1 shows "How to Play"
- Next button goes to page 2 "Keyboard Shortcuts"
- Prev button goes back
- Escape, X button, and clicking outside all close it
- `h` key toggles it

**Step 4: Run lint and format**

Run: `cd client && npm run lint && npm run format:check`

**Step 5: Commit**

```bash
git add client/src/main.ts
git commit -m "feat: wire help modal open/close/page navigation"
```

---

### Task 5: Run full test suite and final lint

**Step 1: Run all tests**

Run: `npm test`
Expected: All tests pass (existing + new keybindings tests)

**Step 2: Run lint for both client and server**

Run: `cd client && npm run lint && npm run format:check && cd ../server && npm run lint && npm run format:check`
Expected: PASS

**Step 3: Final commit if any format fixes were needed**

---
