# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Primary Focus: Modern Web Rewrite

**The active development work in this repository is a modern web rewrite of the original game.** The rewrite lives in the `server/` and `client/` directories and is implemented as a Node.js/TypeScript WebSocket server with a Vite/TypeScript browser client.

- **Overall plan and feature roadmap**: see [`modern-rewrite-plan.md`](modern-rewrite-plan.md)
- **What has already been implemented**: see [`implementation-notes.md`](implementation-notes.md)
- **What still needs to be done**: see [`todo.md`](todo.md)
- **Deployment and hosting plan**: see [`deployment-plan.md`](deployment-plan.md)

When asked to work on the game, assume the target is the modern web rewrite unless the user explicitly asks about the legacy C codebase.

---

## Legacy Game (Reference Only)

**Griljor: The War of Griljor** is a 1989-era multiplayer real-time game written in C with an X11 GUI. It has a client-server architecture where multiple players connect to a game driver (server) over TCP sockets. The legacy source lives in `legacy/src/` and is kept for reference.

### Build System

All source files live in `legacy/src/`. Build from the `legacy/src/` directory using `make`.

```sh
cd legacy/src

#### Build main client + server only
make two

#### Build everything (client, server, map editor, object editor, password editor)
make all

#### Build individual targets
make griljor      # player client
make grildriver   # game server
make editmap      # map editor
make obtor        # object definition editor
make editpass     # password file editor

#### Generate ctags for all source files
make tags

#### Remove executables (preserves .o files)
make clean
```

The Makefile uses `CC = cc` and requires `OPENWINHOME` to be set (for X11 headers via `-I$(OPENWINHOME)/include`). Link flags include `-lX11 -lsocket -lnsl -lm`.

### Configuration

**`legacy/src/config.h`** is the central configuration file. All paths are hardcoded to the original Sun workstation at `/net/rootbeer.Eng/export/home/vbo/games/src/griljor/2.0/`. Before building on a new system, update all paths in `config.h` to point to the actual `legacy/lib/` directory location.

Key path defines: `OBJ_LIB_DIR`, `MAP_LIB_DIR`, `HELP_LIB_DIR`, `DFLT_PASS_FILE`, `DFLT_GAME_FILE`, `DFLT_MAP_FILE`.

Network ports: `BOSS_PORT 1137` (game listing), `BOSS_REC_PORT 3323` (player connections).

### Architecture

#### Executables and Their Source Groupings

The Makefile defines clear source groups (see `legacy/src/Makefile`):

| Executable | Purpose | Key sources |
|------------|---------|-------------|
| `griljor` | Player client | `player.c`, `playerX11.c`, `movement.c`, `missile.c`, `roomdraw.c`, `title.c` + shared + window + vline |
| `grildriver` | Game server | `driver.c`, `accept.c`, `password.c`, `timecheck.c` + shared + burt AI + vline |
| `editmap` | Map editor | `editmap.c`, `editorX11.c`, `emap.c`, `ranroom.c` + shared subset + window + vline |
| `obtor` | Object definition editor | `obtorX11.c`, `editobj.c`, `pixmapedit.c`, etc. + shared subset + window + vline |
| `editpass` | Password file editor | `editpass.c`, `xx.c`, `processorx.c`, `bitmapx.c`, etc. |

#### Shared Module Groups

- **Shared core** (`SRCS`/`OBJS`): `map.c`, `objects.c`, `socket.c`, `person.c`, `lib.c`, `gametime.c`, `message.c`, `notify.c`, `mapfunc.c`, `mapstore.c`, `connect.c`, etc. — used by both client and server.
- **Window layer** (`WINDOW_SRCS`): `windowsX11.c`, `inputX11.c`, `outputX11.c`, `menusX11.c` — shared by client and all editors.
- **Vline graphics** (`VSRCS`): `vsys.c`, `vline.c` — custom 2D drawing library used everywhere.
- **Burt AI** (`BURT_SRCS`): `legacy/burt/io.c`, `legacy/burt/load.c`, `legacy/burt/main.c`, `legacy/burt/person.c`, `legacy/burt/response.c` — Eliza-style NPC AI, compiled into `grildriver` only.

#### Game Data (`legacy/lib/`)

- `legacy/lib/map/` — Map files (`.map`) with player placement metadata (`.pla`)
- `legacy/lib/obj/` — Object definition files (`.obj`, `.bin`)
- `legacy/lib/` — Help files, news, player password store, game list, monster variables, GM personality script (`gm.text`)

#### Bitmap Resources

Bitmaps are stored as raw binary files (no extension) and `#include`d directly into C source:
- `legacy/bitmaps/` — Terrain, environment, and title screen bitmaps
- `legacy/bit/` — UI elements and object bitmaps
- `legacy/facebits/` — Player avatar bitmaps (paired `*bit`/`*mask` files)

#### Utilities (`legacy/util/`)

Standalone tools with their own Makefiles:
- `legacy/util/bmconvert/` — Bitmap format conversion
- `legacy/util/mask/` — Auto-generate masks from bitmaps (`make mkmask`)
- `legacy/util/rotate/` — Bitmap rotation
- `legacy/util/textbit/` — Render text as bitmap

#### Burt AI Testing

```sh
cd legacy/burt
make testburt
./testburt
```

### Key Header Files

- `config.h` — All compile-time configuration (paths, ports, game constants, sizes)
- `def.h` — Common type/macro definitions
- `extern.h` / `extern.c` — Global variable declarations/definitions
- `socket.h` — Network packet structures and protocol
- `map.h` / `objects.h` / `person.h` — Core game data structures
- `windowsX11.h` — X11 window management interface
- `volvox.h` / `vsys.h` / `vline.h` — Vline graphics library headers
