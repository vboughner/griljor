# Worktree Dev Workflow

Work on multiple features simultaneously using git worktrees, with each worktree running its own isolated set of dev servers managed by tmux.

## Prerequisites

```sh
brew install tmux
```

## Creating a Worktree

```sh
./new-worktree.sh <N> <feature> [branch-name]
```

- **N** — Unique positive integer for this worktree (determines ports, must not conflict with another active worktree)
- **feature** — Short name used in the directory and branch name
- **branch-name** — Optional; defaults to `<feature>`

Examples:

```sh
./new-worktree.sh 1 teams
./new-worktree.sh 2 combat fix/combat-damage
./new-worktree.sh 3 lighting
```

This creates `../griljor-<N>-<feature>/`, checks out or creates the branch, and runs `npm install` in `server/` and `client/`.

## Starting Dev Servers

From any worktree directory (or the main repo):

```sh
./dev.sh                        # lobby + 1 game (battle map) + client
./dev.sh battle dungeon         # lobby + 2 games + client
./dev.sh battle dungeon sewers  # lobby + 3 games + client
```

This opens a tmux session with one pane per process. All panes are tiled automatically.

To stop:

```sh
./dev.sh stop
```

## Port Reference

| Worktree              | N | Lobby | Game 0 | Game 1 | Client                         |
|-----------------------|---|-------|--------|--------|--------------------------------|
| `griljor/`            | 0 | 3000  | 3001   | 3002   | http://localhost:5073          |
| `griljor-1-teams/`    | 1 | 3100  | 3101   | 3102   | http://localhost:5173          |
| `griljor-2-combat/`   | 2 | 3200  | 3201   | 3202   | http://localhost:5273          |
| `griljor-3-lighting/` | 3 | 3300  | 3301   | 3302   | http://localhost:5373          |

Formula for worktree N, map index M (0-based):
- Lobby: `3000 + N*100`
- Game M: `3001 + N*100 + M`
- Client: `5073 + N*100`

## tmux Tips

| Action | Keys |
|--------|------|
| Switch pane | `Ctrl+b` then arrow key |
| Scroll in pane | `Ctrl+b [` then arrow keys; `q` to exit |
| Detach session | `Ctrl+b d` |
| Re-attach | `tmux attach-session -t <session-name>` |
| List sessions | `tmux ls` |

## Removing a Worktree

```sh
# Stop the dev session first:
cd ../griljor-1-teams && ./dev.sh stop

# Remove the worktree:
git worktree remove ../griljor-1-teams

# Optionally delete the branch:
git branch -d teams
```

## Notes

- Each worktree shares the same git object store but has its own working tree and index.
- `node_modules/` is not shared — each worktree has its own after `new-worktree.sh` runs `npm install`.
- The session name equals the directory basename (e.g. `griljor-1-teams`), so re-running `./dev.sh` from the same worktree attaches to the existing session.
