#!/usr/bin/env bash
set -euo pipefail

# ---------------------------------------------------------------------------
# new-worktree.sh — create a new git worktree for parallel feature work
#
# Usage:
#   ./new-worktree.sh <N> <feature> [branch-name]
#
# Examples:
#   ./new-worktree.sh 1 teams
#   ./new-worktree.sh 2 combat fix/combat-damage
# ---------------------------------------------------------------------------

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <N> <feature> [branch-name]"
  echo ""
  echo "  N            Positive integer (unique worktree index, e.g. 1, 2, 3)"
  echo "  feature      Short feature name (e.g. teams, combat, lighting)"
  echo "  branch-name  Git branch name (default: <feature>)"
  exit 1
fi

N="$1"
FEATURE="$2"
BRANCH="${3:-$FEATURE}"

# Validate N
if ! [[ "$N" =~ ^[1-9][0-9]*$ ]]; then
  echo "Error: N must be a positive integer (got '$N')."
  exit 1
fi

# Validate feature
if [[ -z "$FEATURE" ]]; then
  echo "Error: feature name must not be empty."
  exit 1
fi

TARGET_DIR="$(dirname "$REPO_DIR")/griljor-${N}-${FEATURE}"

echo "Creating worktree: $TARGET_DIR"
echo "Branch: $BRANCH"
echo ""

# Add worktree (create branch if it doesn't exist)
if git -C "$REPO_DIR" show-ref --verify --quiet "refs/heads/$BRANCH"; then
  echo "Branch '$BRANCH' exists — checking it out."
  git -C "$REPO_DIR" worktree add -q "$TARGET_DIR" "$BRANCH"
else
  echo "Branch '$BRANCH' does not exist — creating it."
  git -C "$REPO_DIR" worktree add -q "$TARGET_DIR" -b "$BRANCH"
fi

echo ""
echo "Installing server dependencies..."
(cd "$TARGET_DIR/server" && npm install --silent)

echo ""
echo "Installing client dependencies..."
(cd "$TARGET_DIR/client" && npm install --silent)

echo ""
echo "Done! Worktree ready at: $TARGET_DIR"
echo ""
echo "To start dev servers:"
echo "  cd $TARGET_DIR"
echo "  ./dev.sh              # battle map"
echo "  ./dev.sh battle dungeon  # multiple maps"
echo ""
echo "Ports for N=$N:"
echo "  Lobby:  $((3000 + N * 100))"
echo "  Game 0: $((3001 + N * 100))"
echo "  Client: http://localhost:$((5073 + N * 100))"
echo ""
echo "To remove this worktree later:"
echo "  git worktree remove $TARGET_DIR"
