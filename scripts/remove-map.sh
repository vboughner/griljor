#!/usr/bin/env bash
#
# scripts/remove-map.sh
#
# Remove a map from the production server.
#
# Usage:  bash ~/griljor/scripts/remove-map.sh <mapname>
# Example: bash ~/griljor/scripts/remove-map.sh battle
#
# This script:
#   1. Stops and deletes the PM2 process for the map
#   2. Removes the WebSocket route from the live nginx config
#   3. Saves the PM2 process list so the change survives reboots
#

set -e

MAPNAME="${1:-}"

if [[ -z "$MAPNAME" ]]; then
  echo "Usage: $0 <mapname>"
  echo "Example: $0 battle"
  echo ""
  echo "Running map processes:"
  pm2 list --no-color | grep -v lobby | grep online || echo "  (none)"
  exit 1
fi

NGINX_CONF="/etc/nginx/sites-available/griljor"

# Confirm PM2 knows about this process
if ! pm2 list --no-color | grep -qw "$MAPNAME"; then
  echo "Warning: no PM2 process named '$MAPNAME' found."
  read -p "Continue with nginx cleanup anyway? [y/N] " CONFIRM
  [[ "$CONFIRM" =~ ^[Yy]$ ]] || exit 1
else
  echo "==> Stopping and deleting PM2 process '$MAPNAME'..."
  pm2 delete "$MAPNAME"

  echo "==> Saving PM2 process list..."
  pm2 save
fi

echo "==> Removing /ws/${MAPNAME} from nginx config..."
if sudo grep -q "~\^/ws/${MAPNAME}" "$NGINX_CONF"; then
  sudo sed -i "/~\^\/ws\/${MAPNAME}/d" "$NGINX_CONF"
  echo "  Removed ~^/ws/${MAPNAME} from nginx map block."
else
  echo "  No nginx entry found for /ws/${MAPNAME} — skipping."
fi

echo "==> Testing and reloading nginx..."
sudo nginx -t && sudo systemctl reload nginx

echo ""
echo "Done. Map '$MAPNAME' has been removed."
echo ""
echo "If this map was in the repo config, also update:"
echo "  server/ecosystem.config.js   — remove the '$MAPNAME' app entry"
echo "  nginx-example.conf           — remove the ~^/ws/${MAPNAME} line from the map block"
