# Lint + Prettier Design

**Date:** 2026-03-13
**Branch:** add-lint-prettier
**Scope:** `client/` and `server/` only — legacy and pipeline folders are untouched.

## Goals

- Add ESLint (TypeScript-aware) and Prettier to both packages
- Provide `npm run lint`, `npm run format`, and `npm run format:check` scripts
- Add a GitHub Actions workflow that runs lint and format checks on PRs

## Approach

- **Separate configs per package** — each of `client/` and `server/` gets its own ESLint and Prettier config
- **ESLint v9 flat config** (`eslint.config.js`) — the current standard
- **No pre-commit hooks** — developers run scripts manually; CI enforces on PRs

## Dependencies (added to each package)

| Package | Role |
|---|---|
| `eslint` | Linter runner |
| `@eslint/js` | ESLint built-in JS rules |
| `typescript-eslint` | TypeScript parser + recommended rules |
| `prettier` | Code formatter |
| `eslint-config-prettier` | Disables ESLint rules that conflict with Prettier |

## Prettier Config (identical in both packages)

```json
{
  "semi": true,
  "singleQuote": true,
  "printWidth": 100,
  "tabWidth": 2
}
```

## ESLint Config

Both packages use `typescript-eslint` recommended rules + `eslint-config-prettier` to disable formatting rules. The server config targets the Node/CommonJS environment; the client config targets browser/ESNext.

## Scripts (added to each `package.json`)

```json
"lint": "eslint src",
"format": "prettier --write src",
"format:check": "prettier --check src"
```

## GitHub Actions

**`.github/workflows/lint.yml`** — triggers on pull requests and pushes to `main`.

Two parallel jobs:
- `lint-client`: `npm ci` → `npm run lint` → `npm run format:check` in `client/`
- `lint-server`: same in `server/`

Uses `ubuntu-latest`, Node 22.

## Files Created/Modified

### `client/`
- `package.json` — add devDependencies + scripts
- `eslint.config.js` — new
- `.prettierrc` — new
- `.prettierignore` — new

### `server/`
- `package.json` — add devDependencies + scripts
- `eslint.config.js` — new
- `.prettierrc` — new
- `.prettierignore` — new

### Root
- `.github/workflows/lint.yml` — new
