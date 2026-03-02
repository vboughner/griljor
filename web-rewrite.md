# Griljor Web Rewrite — Plan

A working document for discussing how to port Griljor to run in a web browser.

---

## What We're Actually Porting

Before picking a stack, it helps to be precise about what each component does
and how hard it is to carry forward:

| Component | Original technology | Web equivalent |
|-----------|--------------------|----|
| Player rendering | X11 bitmaps, custom Vline 2D lib | HTML5 Canvas 2D or WebGL |
| Raw 32×32 bitmaps | Binary files `#include`d into C | Convert to PNGs / sprite sheet |
| In-game messaging | UDP datagrams (custom binary packets) | WebSockets |
| Connection handshake | TCP stream socket (DriverPack/AcceptancePack structs) | WebSocket handshake / HTTP REST |
| Game server logic | `grildriver` C process | Rewritten or wrapped server |
| Burt AI (GM NPC) | Eliza-style pattern match, loaded from `gm.text` | Trivially portable to any language |
| Map / object data | Binary `.map` / `.obj` files on disk | Served as static JSON or binary assets |
| Player password store | Custom binary file, `editpass` GUI editor | Simple DB or JSON file |
| Editors (editmap, obtor) | Full X11 GUI apps | Web-based level editor (optional / stretch goal) |

### The Hard Parts

1. **UDP → WebSockets**: The original uses UDP datagrams for all in-game packets
   (movement, missiles, messages). Browsers cannot send raw UDP. WebSockets
   (TCP-based) are the only option. The game was already designed around a
   packet-with-serial-number model, which actually maps cleanly — but we lose
   fire-and-forget semantics. For a game at this speed (500ms per move) that's
   fine.

2. **Binary bitmaps**: Every terrain tile, UI element, and player avatar is a
   raw 32×32 1-bit binary file `#include`d directly into C source. There are
   ~186 of them. They all need to be converted to PNGs (or a sprite sheet)
   before a browser can draw them. This is a one-time asset pipeline step, not
   an ongoing problem.

3. **Game state is peer-to-peer, not server-authoritative**: Looking at the
   packet types (`MY_LOCATION`, `WEAPON_FIRED`, `YOU_KILLED_ME`), the original
   design has players broadcasting their own state to each other; the driver is
   more of a coordinator than a single source of truth. A web port would
   probably want to centralize this logic in the server, which is a mild
   architectural shift.

4. **Timing model**: The game runs on `CLICKS_PER_SEC = 1000` (1 click = 1ms).
   A web game loop via `requestAnimationFrame` + `setTimeout`/`setInterval` can
   replicate this without difficulty, but it needs to be designed deliberately.

---

## Approach Options

### Option A — Full Clean Rewrite

Rewrite everything from scratch in modern web technologies. Faithfully reproduce
the gameplay, but re-architect for the web from the ground up.

- **Frontend**: TypeScript + HTML5 Canvas 2D
- **Backend**: TypeScript/Node.js or Go, with WebSockets
- **Protocol**: JSON over WebSockets (or MessagePack for compactness)
- **Asset pipeline**: One-time C utility (or script) to convert raw bitmaps → PNGs

**Pros**:
- Clean, maintainable result
- Can be hosted anywhere (Vercel, Fly.io, a VPS)
- Full control over modernizing the architecture
- Shared language (TypeScript) between client and server makes protocol types trivial to share

**Cons**:
- Most work — essentially a ground-up reimplementation
- Risk of gameplay feel diverging from the original

---

### Option B — C Server + WebSocket Bridge

Keep the `grildriver` C server mostly intact, but add a WebSocket-to-UDP
proxy layer. Rewrite only the frontend in TypeScript + Canvas.

```
Browser ←— WebSocket ——→ Bridge process ←— UDP/TCP ——→ grildriver (C)
```

**Pros**:
- Game logic is preserved exactly (the C server is the authority)
- Frontend rewrite is scoped and focused
- Good incremental path: get one piece working at a time

**Cons**:
- The bridge adds complexity and a new failure point
- Still requires significant C modernization work (the hardcoded paths in
  `config.h`, Sun-specific socket flags like `-lsocket -lnsl`, `SUN3`
  typedef guards in `socket.h`)
- The C server would still need to run on a Linux host somewhere
- The peer-to-peer packet model (`TO_ALL` broadcasts) is awkward to proxy

---

### Option C — Emscripten / WebAssembly

Compile the C client to WebAssembly using Emscripten, run it in the browser.

**Pros**:
- Potentially the highest fidelity to the original game logic and feel
- Less reimplementation work per line of original C

**Cons**:
- X11 is not supported by Emscripten — all X11 calls must be replaced anyway
  (which is most of the display code), so the savings are less than they appear
- Build complexity is high (Emscripten toolchain, shim layers)
- Debugging WASM is harder than TypeScript
- Result is harder to maintain or extend

This option is probably not worth it given how much of the code is X11.

---

## Selected Approach

**Option A — Full clean rewrite** in TypeScript throughout:

- **Frontend**: TypeScript + HTML5 Canvas 2D
- **Backend**: Node.js + TypeScript (WebSockets via `ws` library)
- **Protocol**: JSON over WebSockets (shared type definitions between client and server)
- **Fidelity**: Faithful to the original gameplay and feel, with room to tune balance and speed

The original codebase is ~35K lines of C, but a lot of that is X11 boilerplate.
The actual game logic — movement, combat, map rendering, Burt AI, socket protocol
— is well-structured and well-commented, making it a good reference for a port
rather than something to translate line by line.

---

## Proposed Architecture (if we do Option A)

```
┌─────────────────────────────────────┐
│           Browser Client            │
│                                     │
│  ┌──────────┐   ┌─────────────────┐ │
│  │ Canvas   │   │  Input Handler  │ │
│  │ Renderer │   │ (keyboard/mouse)│ │
│  └────┬─────┘   └───────┬─────────┘ │
│       │                 │           │
│  ┌────▼─────────────────▼─────────┐ │
│  │         Game Client State      │ │
│  │  (local copy of world state)   │ │
│  └────────────────┬───────────────┘ │
│                   │ WebSocket       │
└───────────────────┼─────────────────┘
                    │
┌───────────────────▼─────────────────┐
│            Game Server              │
│                                     │
│  ┌──────────────────────────────┐   │
│  │   Connection Manager         │   │
│  │   (WebSocket sessions)       │   │
│  └──────────┬───────────────────┘   │
│             │                       │
│  ┌──────────▼───────────────────┐   │
│  │   Game Engine                │   │
│  │   - Map / room state         │   │
│  │   - Player positions         │   │
│  │   - Missile simulation       │   │
│  │   - Collision / hit logic    │   │
│  │   - Experience / levels      │   │
│  └──────────┬───────────────────┘   │
│             │                       │
│  ┌──────────▼───────────────────┐   │
│  │   Burt AI  (GM NPC)          │   │
│  │   (port of burt/*.c logic)   │   │
│  └──────────────────────────────┘   │
│                                     │
│  ┌──────────────────────────────┐   │
│  │   Persistence                │   │
│  │   - Player profiles (JSON/DB)│   │
│  │   - Map files (JSON)         │   │
│  │   - Object definitions (JSON)│   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

---

## Phased Approach

If we go with Option A, a reasonable sequence would be:

**Phase 1 — Asset pipeline**
- Write a script to convert raw binary bitmaps (`bit/`, `bitmaps/`, `facebits/`)
  to PNGs
- Convert `.map` and `.obj` binary files to JSON

**Phase 2 — Static renderer**
- Scaffold the frontend project (TypeScript + Canvas)
- Load a single map and render it in the browser (no networking yet)
- Render player avatar and basic terrain tiles

**Phase 3 — Server scaffold + WebSocket connection**
- Scaffold the server (Node or Go)
- Define the WebSocket message protocol (mirror the original packet types)
- Get a single client connecting and appearing on the server

**Phase 4 — Core game loop**
- Player movement
- Missile firing and travel
- Hit detection
- Experience / leveling

**Phase 5 — Multiplayer**
- Multiple clients in the same game
- Server-authoritative state broadcast
- Game lifecycle (start, end, score)

**Phase 6 — Persistence + auth**
- Player profiles and passwords
- Save/load stats between games

**Phase 7 — Burt AI + polish**
- Port Burt from C to the server language
- Title screen, game listing, team selection

**Stretch: web-based editors**
- editmap as a browser-based map editor
- obtor as a browser-based object definition editor

---

## Decisions Made

| Question | Decision |
|----------|----------|
| Approach | Option A — full clean rewrite |
| Backend language | TypeScript / Node.js |
| Fidelity | Faithful to original, with room to tune |
| Editors | Port eventually (Phase 7+), not blocking the game port |
| Hosting | TBD — build locally first |
| Persistence | Flat JSON files (mirrors the original binary password store) |
| Server authority | Server-authoritative (cleaner for web, cheat-resistant) |
| Protocol format | JSON over WebSockets (shared TS types between client and server) |

## Open Questions

- **Server authority detail**: The original has some peer-to-peer elements
  (e.g., `YOU_KILLED_ME` is sent from the shooter to the victim). In a
  server-authoritative model the server computes hit detection itself. Worth
  noting as a known divergence from the original.

- **Monorepo vs. separate repos**: Keep client and server in one repo
  (e.g., `client/` and `server/` subdirectories) to share type definitions,
  or separate repos?
