#!/usr/bin/env bash
#
# scripts/rebuild-restart-production.sh
#
# Run this on the production server to rebuild and redeploy the game.
#
# BEFORE running this script:
#   1. Pull the latest code manually:
#        cd ~/griljor && git pull
#   2. Then run this script:
#        bash ~/griljor/scripts/rebuild-restart-production.sh
#
# IMPORTANT NOTES:
#
#   - Do NOT touch the nginx config. The live file at
#     /etc/nginx/sites-available/griljor has certbot SSL lines that are
#     not in the repo. Never copy nginx-example.conf over it.
#
#   - PM2 handles process management. "pm2 restart all" gracefully
#     restarts the lobby + all game servers.
#
#   - After restart, nginx needs read access to the newly rebuilt client/dist/.
#     The chmod step below re-grants that access (the dist folder is recreated
#     fresh on each build, losing previous permissions).
#
#   - If a game server isn't showing in the lobby after restart, it may have
#     started before the lobby was ready. Fix with: pm2 restart <mapname>
#

set -e

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"

echo "==> Building server..."
cd "$REPO_DIR/server"
npm run build

echo "==> Building client..."
cd "$REPO_DIR/client"
npm run build

echo "==> Fixing permissions on client/dist (required for nginx/www-data)..."
chmod -R o+r "$REPO_DIR/client/dist"

echo "==> Restarting all PM2 processes..."
pm2 restart all

echo ""
echo "Done. Verify with:"
echo "  pm2 status          # all processes should show 'online'"
echo "  pm2 logs            # watch for heartbeats every 5s from each game server"
echo "  curl https://griljor.com/games  # should return JSON game list"
