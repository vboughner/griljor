# Item Flavor Text Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Display `lookmsg` and `examinemsg` flavor text in item tooltips, matching the original game's look/examine context logic.

**Architecture:** Add two optional string fields to the client `ObjDef` type. Update `buildItemHtml` to accept a `context` parameter (`'floor' | 'inventory'`) that controls which message to show: inventory context shows `examinemsg` (falling back to `lookmsg`), floor context shows `lookmsg` only. The data already exists in the pipeline JSON — no server or pipeline changes needed.

**Tech Stack:** TypeScript (client only), Vitest for tests

---

### Task 1: Add flavor text fields to ObjDef type

**Files:**
- Modify: `client/src/types.ts:42-67`

**Step 1: Add fields to ObjDef interface**

Add after `lost?: boolean;` (line 66):

```typescript
  lookmsg?: string;
  examinemsg?: string;
```

**Step 2: Commit**

```bash
git add client/src/types.ts
git commit -m "feat: add lookmsg/examinemsg fields to client ObjDef type"
```

---

### Task 2: Write failing tests for flavor text in tooltips

**Files:**
- Modify: `client/src/__tests__/tooltip.test.ts`

**Step 1: Write tests for all flavor text scenarios**

Add these tests at the end of the `describe('buildItemHtml', ...)` block:

```typescript
  it('shows examinemsg in inventory context when both messages exist', () => {
    const html = buildItemHtml(
      makeObj({ lookmsg: 'Looks sharp.', examinemsg: 'Finely crafted steel.' }),
      makeItem(),
      'inventory',
    );
    expect(html).toContain('Finely crafted steel.');
    expect(html).not.toContain('Looks sharp.');
  });

  it('falls back to lookmsg in inventory context when no examinemsg', () => {
    const html = buildItemHtml(
      makeObj({ lookmsg: 'Looks sharp.' }),
      makeItem(),
      'inventory',
    );
    expect(html).toContain('Looks sharp.');
  });

  it('shows lookmsg in floor context', () => {
    const html = buildItemHtml(
      makeObj({ lookmsg: 'Looks sharp.', examinemsg: 'Finely crafted steel.' }),
      makeItem(),
      'floor',
    );
    expect(html).toContain('Looks sharp.');
    expect(html).not.toContain('Finely crafted steel.');
  });

  it('shows nothing in floor context when no lookmsg', () => {
    const html = buildItemHtml(
      makeObj({ examinemsg: 'Finely crafted steel.' }),
      makeItem(),
      'floor',
    );
    expect(html).not.toContain('Finely crafted steel.');
    expect(html).not.toContain('tip-flavor');
  });

  it('shows nothing when no flavor text at all', () => {
    const html = buildItemHtml(makeObj(), makeItem(), 'inventory');
    expect(html).not.toContain('tip-flavor');
  });

  it('defaults to inventory context when context omitted', () => {
    const html = buildItemHtml(
      makeObj({ examinemsg: 'Finely crafted steel.' }),
      makeItem(),
    );
    expect(html).toContain('Finely crafted steel.');
  });

  it('escapes HTML in flavor text', () => {
    const html = buildItemHtml(
      makeObj({ lookmsg: '<script>alert("xss")</script>' }),
      makeItem(),
      'floor',
    );
    expect(html).not.toContain('<script>');
    expect(html).toContain('&lt;script&gt;');
  });
```

**Step 2: Run tests to verify they fail**

Run: `cd client && npx vitest run src/__tests__/tooltip.test.ts`
Expected: FAIL — `buildItemHtml` doesn't accept a third argument yet, and no flavor text appears in output.

**Step 3: Commit**

```bash
git add client/src/__tests__/tooltip.test.ts
git commit -m "test: add failing tests for item flavor text in tooltips"
```

---

### Task 3: Implement flavor text in buildItemHtml

**Files:**
- Modify: `client/src/tooltip.ts:44-80`

**Step 1: Add HTML-escaping helper and update buildItemHtml**

Add this helper above `buildItemHtml`:

```typescript
function escapeHtml(s: string): string {
  return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
}
```

Update the `buildItemHtml` signature to accept an optional context parameter:

```typescript
export function buildItemHtml(
  obj: ObjDef,
  item: InventoryItem,
  context: 'floor' | 'inventory' = 'inventory',
): string {
```

Add flavor text display after the name row (after the `rows.push` for `tip-name`, before the weapon row):

```typescript
  const flavorMsg =
    context === 'inventory'
      ? (obj.examinemsg ?? obj.lookmsg)
      : obj.lookmsg;
  if (flavorMsg) {
    rows.push(`<div class="tip-row tip-flavor">${escapeHtml(flavorMsg)}</div>`);
  }
```

**Step 2: Run tests to verify they pass**

Run: `cd client && npx vitest run src/__tests__/tooltip.test.ts`
Expected: ALL PASS

**Step 3: Commit**

```bash
git add client/src/tooltip.ts
git commit -m "feat: display lookmsg/examinemsg flavor text in item tooltips"
```

---

### Task 4: Style the flavor text

**Files:**
- Modify: `client/index.html` (or wherever tooltip CSS is defined)

Find the existing tooltip styles and add:

```css
.tip-flavor {
  font-style: italic;
  opacity: 0.85;
  white-space: pre-line;
}
```

The `white-space: pre-line` handles multi-line messages like the sludge gun's examinemsg which contains `\n`.

**Step 1: Find and add CSS**

Search for `.tip-name` or `.tip-row` CSS to locate tooltip styles, then add `.tip-flavor` rule.

**Step 2: Commit**

```bash
git add <css-file>
git commit -m "style: italic flavor text in item tooltips"
```

---

### Task 5: Run lint and full test suite

**Step 1: Run lint**

```bash
cd client && npm run lint && npm run format:check
```

Fix any issues.

**Step 2: Run full test suite**

```bash
npm test
```

Expected: ALL PASS

**Step 3: Final commit (if lint fixes needed)**

```bash
git add -p
git commit -m "fix: lint/format fixes for flavor text"
```
