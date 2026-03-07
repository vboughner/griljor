# Griljor: The War of Griljor

A multiplayer real-time action game originally written in 1989 for Sun workstations running X11. Players navigate 20×20 tile rooms, pick up weapons and items, and battle each other across interconnected maps.

---

## Active Development: Modern Web Rewrite

The game is being rewritten as a browser-playable web application. The goal is to preserve the original gameplay faithfully while making it accessible to anyone with a modern browser — no installation required.

**Why**: The original runs only on 1989-era Sun/X11 systems. The binary format, hardcoded paths, and X11 dependency make it effectively unplayable today. The rewrite extracts all original assets (bitmaps, maps, object definitions) through an automated pipeline and reimplements the game logic in TypeScript.

**How**:
- A Python pipeline (`pipeline/`) parses the original binary `.map` and `.obj` files into JSON and converts raw 1-bit XBM bitmaps to PNGs — a one-time extraction of everything the original game shipped with.
- A Node.js/TypeScript WebSocket server (`server/`) runs the authoritative game logic: movement, combat, inventory, doors, room state.
- A Vite/TypeScript browser client (`client/`) renders the 20×20 tile grid on HTML5 Canvas using the original bitmaps, and communicates with the server over WebSockets.
- A lobby server lists running game instances so players can browse and join from a title screen.

**What's implemented**: asset pipeline, full room rendering with original sprites, multiplayer sync, inventory system (35 slots + hand slots), combat with projectiles and damage, XP/leveling, doors and keys, consumables, map state reset, lobby with live player display.

### Key documents

| Document | Contents |
|----------|----------|
| [`web-rewrite.md`](web-rewrite.md) | Architecture decisions, porting challenges, and feature plan |
| [`implementation-notes.md`](implementation-notes.md) | Detailed record of what was built, phase by phase, with technical decisions |
| [`todo.md`](todo.md) | Remaining work |
| [`deployment-plan.md`](deployment-plan.md) | Hosting plan (Hetzner VM, nginx, PM2, HTTPS) |

### Running locally

```sh
# Terminal 1 — game server (default map: battle, port 3001)
cd server && npm run dev

# Terminal 2 — Vite dev server (client)
cd client && npm run dev
```

The lobby server (`server/src/lobby.ts`) runs on port 3000. The client connects to it automatically at `localhost:3000`.

---

## Legacy Codebase (Reference)

The original C/X11 source is preserved in `src/` for reference. It is not the active development target.

### Building the legacy code

```sh
cd src
make two       # builds griljor (client) + grildriver (server)
make all       # also builds editmap, obtor, editpass
```

Requires `OPENWINHOME` set (for X11 headers) and paths in `src/config.h` updated to the local system. See `CLAUDE.md` for full build details.

### Original install instructions

The file you're reading replaces the original `README.md`, which contained 1989-era Sun workstation install instructions (untarring a tarfile, editing hardcoded paths, running `make two`, contacting `vanb@soda.berkeley.edu`). Those instructions are preserved in git history.
