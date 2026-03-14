#!/usr/bin/env bash
set -euo pipefail

# ---------------------------------------------------------------------------
# dev.sh — start tmux dev session for a Griljor worktree
#
# Usage:
#   ./dev.sh [map1] [map2] ...   # default: battle
#   ./dev.sh stop                # kill the tmux session
#
# Port formula (N = worktree index parsed from dirname):
#   Lobby:    3000 + N*100
#   Game M:   3001 + N*100 + M   (M = 0-based map index)
#   Client:   5073 + N*100
# ---------------------------------------------------------------------------

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIRNAME="$(basename "$REPO_DIR")"

# Parse N from dirname
if [[ "$DIRNAME" == "griljor" ]]; then
  N=0
elif [[ "$DIRNAME" =~ ^griljor-([0-9]+)-.+ ]]; then
  N="${BASH_REMATCH[1]}"
else
  echo "Error: cannot determine worktree index from directory name '$DIRNAME'."
  echo "Expected 'griljor' or 'griljor-<N>-<feature>'."
  exit 1
fi

SESSION="$DIRNAME"

# Handle stop
if [[ "${1:-}" == "stop" ]]; then
  if tmux has-session -t "$SESSION" 2>/dev/null; then
    tmux kill-session -t "$SESSION"
    echo "Stopped tmux session: $SESSION"
  else
    echo "No tmux session named '$SESSION' is running."
  fi
  exit 0
fi

# Collect maps (default: battle)
MAPS=("${@:-battle}")

# Calculate ports
LOBBY_PORT=$((3000 + N * 100))
CLIENT_PORT=$((5073 + N * 100))

# If session already exists, attach
if tmux has-session -t "$SESSION" 2>/dev/null; then
  echo "Session '$SESSION' already running — attaching."
  exec tmux attach-session -t "$SESSION"
fi

# Build the lobby command
LOBBY_CMD="cd '$REPO_DIR/server' && PORT=$LOBBY_PORT npm run lobby"

# Create session with lobby pane
tmux new-session -d -s "$SESSION" -n dev bash -c "$LOBBY_CMD; exec bash"

# Add a pane for each map
for i in "${!MAPS[@]}"; do
  MAP="${MAPS[$i]}"
  GAME_PORT=$((3001 + N * 100 + i))
  GAME_CMD="cd '$REPO_DIR/server' && MAP=$MAP PORT=$GAME_PORT LOBBY_URL=http://localhost:$LOBBY_PORT npm run dev; exec bash"
  tmux split-window -t "$SESSION:dev" -v bash -c "$GAME_CMD"
  tmux select-layout -t "$SESSION:dev" tiled
done

# Add client pane
CLIENT_CMD="cd '$REPO_DIR/client' && VITE_LOBBY_URL=http://localhost:$LOBBY_PORT npm run dev -- --port $CLIENT_PORT; exec bash"
tmux split-window -t "$SESSION:dev" -v bash -c "$CLIENT_CMD"
tmux select-layout -t "$SESSION:dev" tiled

echo "Open: http://localhost:$CLIENT_PORT"
exec tmux attach-session -t "$SESSION"
