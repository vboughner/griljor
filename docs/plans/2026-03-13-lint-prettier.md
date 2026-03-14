# Lint + Prettier Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add ESLint v9 + Prettier to `client/` and `server/`, with a GitHub Actions workflow that enforces them on PRs.

**Architecture:** Separate configs per package (no shared root config). ESLint v9 flat config (`eslint.config.mjs`) in each package. Prettier config identical in both. CI runs two parallel jobs — one per package.

**Tech Stack:** ESLint v9, `typescript-eslint` (unified v8+ package), Prettier, `eslint-config-prettier`, GitHub Actions.

---

### Task 1: Set up linting and formatting in `client/`

**Files:**
- Modify: `client/package.json`
- Create: `client/eslint.config.mjs`
- Create: `client/.prettierrc`
- Create: `client/.prettierignore`

---

**Step 1: Install dependencies**

```bash
cd client
npm install --save-dev eslint @eslint/js typescript-eslint prettier eslint-config-prettier
```

Expected: packages added to `devDependencies`, `package-lock.json` updated.

---

**Step 2: Create `client/eslint.config.mjs`**

```js
import eslint from '@eslint/js';
import tseslint from 'typescript-eslint';
import prettierConfig from 'eslint-config-prettier';

export default tseslint.config(
  eslint.configs.recommended,
  tseslint.configs.recommended,
  prettierConfig,
);
```

Note: `.mjs` extension forces ESM syntax regardless of the package's `"type"` field.

---

**Step 3: Create `client/.prettierrc`**

```json
{
  "semi": true,
  "singleQuote": true,
  "printWidth": 100,
  "tabWidth": 2
}
```

---

**Step 4: Create `client/.prettierignore`**

```
dist/
node_modules/
```

---

**Step 5: Add scripts to `client/package.json`**

Add to the `"scripts"` block:

```json
"lint": "eslint src",
"format": "prettier --write src",
"format:check": "prettier --check src"
```

---

**Step 6: Format existing source files**

```bash
cd client
npm run format
```

Expected: Prettier rewrites files in `src/` to match config. Many files will be modified.

---

**Step 7: Lint and auto-fix**

```bash
cd client
npm run lint -- --fix
```

Expected: ESLint fixes auto-fixable issues. Review any remaining errors printed to stdout and fix them manually. Common issues: unused variables (may need to prefix with `_`), `any` types flagged by `@typescript-eslint/no-explicit-any`.

If there are `@typescript-eslint/no-explicit-any` errors on legitimate uses (e.g. WebSocket message data), suppress them with `// eslint-disable-next-line @typescript-eslint/no-explicit-any` on the line above, or consider adding a rule override to the config:

```js
// add inside tseslint.config(...) as a final object:
{
  rules: {
    '@typescript-eslint/no-explicit-any': 'warn',
  },
}
```

---

**Step 8: Verify both scripts pass cleanly**

```bash
cd client
npm run lint
npm run format:check
```

Expected: both exit with code 0, no output (or "All matched files use Prettier code style!").

---

**Step 9: Commit**

```bash
git add client/package.json client/package-lock.json client/eslint.config.mjs client/.prettierrc client/.prettierignore client/src/
git commit -m "feat: add ESLint + Prettier to client"
```

---

### Task 2: Set up linting and formatting in `server/`

**Files:**
- Modify: `server/package.json`
- Create: `server/eslint.config.mjs`
- Create: `server/.prettierrc`
- Create: `server/.prettierignore`

---

**Step 1: Install dependencies**

```bash
cd server
npm install --save-dev eslint @eslint/js typescript-eslint prettier eslint-config-prettier
```

---

**Step 2: Create `server/eslint.config.mjs`**

```js
import eslint from '@eslint/js';
import tseslint from 'typescript-eslint';
import prettierConfig from 'eslint-config-prettier';

export default tseslint.config(
  eslint.configs.recommended,
  tseslint.configs.recommended,
  prettierConfig,
);
```

---

**Step 3: Create `server/.prettierrc`**

```json
{
  "semi": true,
  "singleQuote": true,
  "printWidth": 100,
  "tabWidth": 2
}
```

---

**Step 4: Create `server/.prettierignore`**

```
dist/
node_modules/
```

---

**Step 5: Add scripts to `server/package.json`**

Add to the `"scripts"` block:

```json
"lint": "eslint src",
"format": "prettier --write src",
"format:check": "prettier --check src"
```

---

**Step 6: Format existing source files**

```bash
cd server
npm run format
```

---

**Step 7: Lint and auto-fix**

```bash
cd server
npm run lint -- --fix
```

Review and manually fix any remaining errors. Apply the same `@typescript-eslint/no-explicit-any` strategy as in Task 1 if needed.

---

**Step 8: Verify both scripts pass cleanly**

```bash
cd server
npm run lint
npm run format:check
```

Expected: both exit with code 0.

---

**Step 9: Commit**

```bash
git add server/package.json server/package-lock.json server/eslint.config.mjs server/.prettierrc server/.prettierignore server/src/
git commit -m "feat: add ESLint + Prettier to server"
```

---

### Task 3: Add GitHub Actions CI workflow

**Files:**
- Create: `.github/workflows/lint.yml`

---

**Step 1: Create `.github/workflows/lint.yml`**

```yaml
name: Lint

on:
  push:
    branches: [main]
  pull_request:

jobs:
  lint-client:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-node@v4
        with:
          node-version: 22
          cache: npm
          cache-dependency-path: client/package-lock.json
      - run: npm ci
        working-directory: client
      - run: npm run lint
        working-directory: client
      - run: npm run format:check
        working-directory: client

  lint-server:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-node@v4
        with:
          node-version: 22
          cache: npm
          cache-dependency-path: server/package-lock.json
      - run: npm ci
        working-directory: server
      - run: npm run lint
        working-directory: server
      - run: npm run format:check
        working-directory: server
```

---

**Step 2: Verify the file looks correct, then commit**

```bash
git add .github/workflows/lint.yml
git commit -m "feat: add GitHub Actions lint workflow"
```

---
