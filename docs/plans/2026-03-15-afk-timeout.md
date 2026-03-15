# AFK Timeout Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Detect idle players and kick them after an idle period, warning them each minute (or second during testing) for a configurable grace period.

**Architecture:** Track `lastActivityAt` per player in the `Player` interface. On each non-PING message, reset two per-player timers: an idle timer (`AFK_IDLE_MS`) that starts the warning phase, and a warning interval that fires `AFK_GRACE_MINUTES` countdown messages. After the last warning, call `onLeave` and close the socket. If activity resumes during the warning phase, cancel timers and send a "welcome back" private GM message.

**Tech Stack:** TypeScript, Node.js `setTimeout`/`clearTimeout`, Vitest with `vi.useFakeTimers()`

---

### Task 1: Add constants and `lastActivityAt` to the Player interface

**Files:**
- Modify: `server/src/session.ts` (top of file, near existing constants; Player interface)

**Step 1: Add AFK constants** after the existing constants block (after `RESPAWN_DELAY_MS`):

```ts
// AFK idle detection
// NOTE: Set to small values for testing; restore to minutes for production.
const AFK_IDLE_MS = 5 * 1000;          // idle time before first warning (testing: 5s)
const AFK_WARN_INTERVAL_MS = 1 * 1000; // interval between warnings (testing: 1s)
const AFK_GRACE_MINUTES = 5;           // number of warnings before kick
```

**Step 2: Add fields to the `Player` interface** (after `lastFireTime`):

```ts
  lastActivityAt: number;
  afkIdleTimer: ReturnType<typeof setTimeout> | null;
  afkWarnTimer: ReturnType<typeof setTimeout> | null;
  afkWarningsLeft: number;
```

**Step 3: Initialize the new fields** in `onJoin`, in the `player` object literal (after `lastFireTime: 0`):

```ts
      lastActivityAt: Date.now(),
      afkIdleTimer: null,
      afkWarnTimer: null,
      afkWarningsLeft: 0,
```

**Step 4: Run lint/format check**

```bash
cd server && npm run lint && npm run format:check
```

Expected: pass (no logic changes yet, just type additions)

**Step 5: Commit**

```bash
git add server/src/session.ts
git commit -m "feat: add AFK constants and Player fields (no logic yet)"
```

---

### Task 2: Add AFK timer helper methods

**Files:**
- Modify: `server/src/session.ts` (add private methods near the bottom helpers section)

**Step 1: Add `startAfkTimer` method** in the `// ── Helpers ───` section:

```ts
  private startAfkTimer(player: Player): void {
    this.clearAfkTimers(player);
    player.afkIdleTimer = setTimeout(() => {
      player.afkIdleTimer = null;
      player.afkWarningsLeft = AFK_GRACE_MINUTES;
      this.sendAfkWarning(player);
    }, AFK_IDLE_MS);
  }

  private clearAfkTimers(player: Player): void {
    if (player.afkIdleTimer !== null) {
      clearTimeout(player.afkIdleTimer);
      player.afkIdleTimer = null;
    }
    if (player.afkWarnTimer !== null) {
      clearTimeout(player.afkWarnTimer);
      player.afkWarnTimer = null;
    }
  }

  private sendAfkWarning(player: Player): void {
    const mins = player.afkWarningsLeft;
    this.send(player.ws, {
      type: 'MESSAGE',
      from: 0,
      name: 'GM',
      to: player.id,
      text: `You'll be kicked from the game in another ${mins} minute${mins === 1 ? '' : 's'} if you are still inactive.`,
    });
    player.afkWarningsLeft--;
    if (player.afkWarningsLeft <= 0) {
      // Grace period exhausted — kick the player
      player.afkWarnTimer = setTimeout(() => {
        player.afkWarnTimer = null;
        this.onLeave(player.id);
        try { player.ws.close(); } catch { /* already closed */ }
      }, AFK_WARN_INTERVAL_MS);
    } else {
      player.afkWarnTimer = setTimeout(() => {
        player.afkWarnTimer = null;
        this.sendAfkWarning(player);
      }, AFK_WARN_INTERVAL_MS);
    }
  }

  private resetAfkTimer(player: Player): void {
    const wasWarning = player.afkWarningsLeft > 0 || player.afkWarnTimer !== null;
    player.lastActivityAt = Date.now();
    this.startAfkTimer(player);
    if (wasWarning) {
      player.afkWarningsLeft = 0;
      this.send(player.ws, {
        type: 'MESSAGE',
        from: 0,
        name: 'GM',
        to: player.id,
        text: 'Welcome back, I see you are still active!',
      });
    }
  }
```

**Step 2: Run lint/format check**

```bash
cd server && npm run lint && npm run format:check
```

Expected: pass

**Step 3: Commit**

```bash
git add server/src/session.ts
git commit -m "feat: add AFK timer helper methods"
```

---

### Task 3: Wire up the AFK timers

**Files:**
- Modify: `server/src/session.ts`

**Step 1: Start the AFK timer on join** — at the end of `onJoin`, after `console.log(...)`:

```ts
    this.startAfkTimer(player);
```

**Step 2: Reset the AFK timer on every non-PING message** — in `handleConnection`, inside the `else if (playerId !== undefined)` block, add one line before the `switch`:

```ts
        const player = this.players.get(playerId);
        if (player && msg.type !== 'PING') this.resetAfkTimer(player);
```

**Step 3: Clear AFK timers on leave** — at the top of `onLeave`, after clearing `respawnTimer`:

```ts
    this.clearAfkTimers(player);
```

**Step 4: Run lint/format check**

```bash
cd server && npm run lint && npm run format:check
```

Expected: pass

**Step 5: Commit**

```bash
git add server/src/session.ts
git commit -m "feat: wire AFK timers to join/message/leave"
```

---

### Task 4: Write integration tests for AFK timeout

**Files:**
- Create: `server/src/__tests__/integration/afk-timeout.test.ts`

**Step 1: Write the test file**

```ts
import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';

describe('AFK timeout', () => {
  let session: GameSession;

  beforeEach(() => {
    vi.useFakeTimers();
    session = new GameSession(buildTestWorld());
  });

  afterEach(() => {
    session.destroy();
    vi.useRealTimers();
  });

  it('sends a warning after idle period', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance past AFK_IDLE_MS (5s in test config)
    vi.advanceTimersByTime(5001);

    const msgs = alice.ws.messagesOfType('MESSAGE');
    expect(
      msgs.some((m) => m.name === 'GM' && m.text.includes('kicked') && m.text.includes('5')),
    ).toBe(true);
  });

  it('counts down each warning interval', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(5001); // idle timer fires → first warning (5 mins left)
    vi.advanceTimersByTime(1001); // second warning (4 mins left)
    vi.advanceTimersByTime(1001); // third warning (3 mins left)

    const msgs = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(msgs.length).toBe(3);
    expect(msgs[0].text).toContain('5 minutes');
    expect(msgs[1].text).toContain('4 minutes');
    expect(msgs[2].text).toContain('3 minutes');
  });

  it('kicks the player after all warnings expire', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.flush();

    // Idle timer + 5 warning intervals
    vi.advanceTimersByTime(5001 + 5 * 1001);

    const leaving = bob.ws.messagesOfType('LEAVING_GAME');
    expect(leaving.some((m) => m.id === alice.id)).toBe(true);
  });

  it('singular "minute" in final warning', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Fire idle timer + 4 warning intervals to reach the "1 minute" warning
    vi.advanceTimersByTime(5001 + 4 * 1001);

    const msgs = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(msgs[4].text).toContain('1 minute');
    expect(msgs[4].text).not.toContain('minutes');
  });

  it('resets timer and does not warn when player sends a message during idle', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance most of idle period, then send activity
    vi.advanceTimersByTime(3000);
    alice.ws.receive({ type: 'MESSAGE', to: 'all', text: 'hello' });
    alice.ws.flush();

    // Advance past original idle expiry — but timer was reset, so no warning yet
    vi.advanceTimersByTime(3000);

    const gm = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gm.length).toBe(0);
  });

  it('sends welcome-back message when activity resumes during warning phase', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    vi.advanceTimersByTime(5001); // first warning fires
    alice.ws.flush();

    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });

    const gm = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gm.some((m) => m.text.toLowerCase().includes('welcome back'))).toBe(true);
  });

  it('does not send welcome-back if active before warning phase starts', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Activity before idle timer fires — no warning phase yet
    vi.advanceTimersByTime(3000);
    alice.ws.receive({ type: 'MY_LOCATION', room: 0, x: 5, y: 5 });

    const gm = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gm.some((m) => m.text.toLowerCase().includes('welcome back'))).toBe(false);
  });

  it('PING does not reset the AFK timer', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();

    // Advance to just before idle expiry, then PING
    vi.advanceTimersByTime(4500);
    alice.ws.receive({ type: 'PING' });
    alice.ws.flush();

    // Advance remaining — idle timer should still fire
    vi.advanceTimersByTime(600);

    const gm = alice.ws.messagesOfType('MESSAGE').filter((m) => m.name === 'GM');
    expect(gm.some((m) => m.text.includes('kicked'))).toBe(true);
  });
});
```

**Step 2: Run the tests to verify they fail** (implementation not wired yet at this step — if Task 3 is already done, they may pass)

```bash
cd server && npx vitest run src/__tests__/integration/afk-timeout.test.ts
```

Expected: all pass (Tasks 1–3 already completed)

**Step 3: Run full test suite**

```bash
npm test
```

Expected: all tests pass

**Step 4: Commit**

```bash
git add server/src/__tests__/integration/afk-timeout.test.ts
git commit -m "test: AFK timeout integration tests"
```

---

### Task 5: Lint, format, and final check

**Step 1: Lint and format both packages**

```bash
cd server && npm run lint && npm run format:check
cd client && npm run lint && npm run format:check
```

Expected: both pass with exit code 0. If formatting issues: `npm run format` in the relevant directory.

**Step 2: Run full test suite one final time**

```bash
cd .. && npm test
```

Expected: all tests pass

**Step 3: Commit if any formatting fixes were needed**

```bash
git add -p
git commit -m "style: fix lint/format issues after AFK timeout"
```
