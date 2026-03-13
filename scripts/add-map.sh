#!/usr/bin/env bash
#
# scripts/add-map.sh
#
# Add a new map to the production server.
#
# Usage:  bash ~/griljor/scripts/add-map.sh <mapname> <port>
# Example: bash ~/griljor/scripts/add-map.sh battle 3005
#
# Available maps: see pipeline/out/data/maps/ for valid map names.
# Ports already in use: 3000 (lobby), 3002 (castle), 3003 (paradise), 3004 (flag).
# Use a new port that isn't already taken.
#
# This script:
#   1. Starts the game server as a PM2 process
#   2. Adds the WebSocket route to the live nginx config
#   3. Saves the PM2 process list so the map survives reboots
#
# To also commit the map to the repo config, update:
#   server/ecosystem.config.js  — add the app entry
#   nginx-example.conf          — add the ~^/ws/<mapname> line to the map block
#

set -e

MAPNAME="${1:-}"
PORT="${2:-}"

if [[ -z "$MAPNAME" || -z "$PORT" ]]; then
  echo "Usage: $0 <mapname> <port>"
  echo "Example: $0 battle 3005"
  echo ""
  echo "Available maps:"
  ls "$(cd "$(dirname "$0")/.." && pwd)/pipeline/out/data/maps/" | sed 's/\.json$//'
  exit 1
fi

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DOMAIN="griljor.com"
LOBBY="http://localhost:3000"
NGINX_CONF="/etc/nginx/sites-available/griljor"

# Confirm the map data file exists
MAP_FILE="$REPO_DIR/pipeline/out/data/maps/${MAPNAME}.json"
if [[ ! -f "$MAP_FILE" ]]; then
  echo "Error: no map data file found at: $MAP_FILE"
  echo ""
  echo "Available maps:"
  ls "$REPO_DIR/pipeline/out/data/maps/" | sed 's/\.json$//'
  exit 1
fi

# Confirm the server build is present
SERVER_MAIN="$REPO_DIR/server/dist/main.js"
if [[ ! -f "$SERVER_MAIN" ]]; then
  echo "Error: server build not found at $SERVER_MAIN"
  echo "Run: cd ~/griljor/server && npm run build"
  exit 1
fi

# Confirm PM2 doesn't already have this name
if pm2 list --no-color | grep -qw "$MAPNAME"; then
  echo "Error: PM2 already has a process named '$MAPNAME'."
  echo "To replace it, first run: bash ~/griljor/scripts/remove-map.sh $MAPNAME"
  exit 1
fi

# Confirm the port isn't already in use by PM2
if pm2 list --no-color | grep -q ":${PORT}"; then
  echo "Warning: port $PORT may already be in use. Check 'pm2 list'."
  read -p "Continue anyway? [y/N] " CONFIRM
  [[ "$CONFIRM" =~ ^[Yy]$ ]] || exit 1
fi

echo "==> Starting PM2 process '$MAPNAME' on port $PORT..."
MAP="$MAPNAME" PORT="$PORT" LOBBY_URL="$LOBBY" PUBLIC_WS_URL="wss://${DOMAIN}/ws/${MAPNAME}" \
  pm2 start "$SERVER_MAIN" --name "$MAPNAME"

echo "==> Adding /ws/${MAPNAME} → ${PORT} to nginx config..."
sudo python3 - "$MAPNAME" "$PORT" "$NGINX_CONF" <<'PYEOF'
import sys, re

mapname, port, conf_path = sys.argv[1], sys.argv[2], sys.argv[3]

with open(conf_path) as f:
    conf = f.read()

if f'~^/ws/{mapname}' in conf:
    print(f'  Entry for /ws/{mapname} already exists in nginx config — skipping.')
else:
    new_line = f'    ~^/ws/{mapname}   {port};\n'
    # Insert the new entry at the end of the map $uri $ws_port { ... } block
    conf = re.sub(
        r'(map \$uri \$ws_port \{[^}]*)',
        lambda m: m.group(0) + new_line,
        conf, count=1
    )
    with open(conf_path, 'w') as f:
        f.write(conf)
    print(f'  Added ~^/ws/{mapname}   {port};')
PYEOF

echo "==> Testing and reloading nginx..."
sudo nginx -t && sudo systemctl reload nginx

echo "==> Saving PM2 process list..."
pm2 save

echo ""
echo "Done. Map '$MAPNAME' is now live at wss://${DOMAIN}/ws/${MAPNAME}"
echo ""
echo "Verify with:"
echo "  pm2 status"
echo "  curl https://${DOMAIN}/games   # should list $MAPNAME"
echo ""
echo "If '$MAPNAME' doesn't appear in the lobby after a few seconds, run:"
echo "  pm2 restart $MAPNAME"
echo ""
echo "To also commit this map to the repo config, update:"
echo "  server/ecosystem.config.js   — add the '$MAPNAME' app entry"
echo "  nginx-example.conf           — add the ~^/ws/${MAPNAME} line to the map block"
