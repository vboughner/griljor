#!/usr/bin/env bash
#
# scripts/sync-repo-config.sh
#
# Reads the currently running PM2 map processes and syncs the repo config files
# to match, so they reflect reality after an add-map or remove-map operation.
#
# Files updated:
#   server/ecosystem.config.js  — rebuilds the map app entries
#   server/nginx-example.conf   — rebuilds the map $uri $ws_port block
#
# Usage:  bash ~/griljor/scripts/sync-repo-config.sh
#
# Run this after add-map.sh or remove-map.sh, then commit the changes.
#

set -e

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"

python3 - "$REPO_DIR" <<'PYEOF'
import sys, json, subprocess, re

repo_dir = sys.argv[1]

# ---------------------------------------------------------------------------
# 1. Read live PM2 state
# ---------------------------------------------------------------------------
result = subprocess.run(['pm2', 'jlist'], capture_output=True, text=True)
if result.returncode != 0:
    print(f'Error: pm2 jlist failed:\n{result.stderr}')
    sys.exit(1)

processes = json.loads(result.stdout)

def get_env(proc, key):
    """Read an env var from either pm2_env directly or pm2_env.env (ecosystem file style)."""
    pm2_env = proc.get('pm2_env', {})
    if key in pm2_env:
        return pm2_env[key]
    return pm2_env.get('env', {}).get(key)

# Collect game server processes: those with a MAP env var, sorted by port
maps = []
for proc in processes:
    map_name = get_env(proc, 'MAP')
    port = get_env(proc, 'PORT')
    if map_name and port:
        maps.append({'name': map_name, 'port': int(port)})

maps.sort(key=lambda m: m['port'])

if not maps:
    print('No running map processes found in PM2 (no process has a MAP env var).')
    print('Nothing to sync.')
    sys.exit(0)

print(f'Found {len(maps)} running map(s):')
for m in maps:
    print(f"  {m['name']:20s} port {m['port']}")
print()

# ---------------------------------------------------------------------------
# 2. Update server/ecosystem.config.js
# ---------------------------------------------------------------------------
eco_path = f'{repo_dir}/server/ecosystem.config.js'
with open(eco_path) as f:
    eco = f.read()

# Generate the map entries block
map_entries = ''
for m in maps:
    name = m['name']
    port = m['port']
    map_entries += (
        f"    {{\n"
        f"      name:   '{name}',\n"
        f"      script: 'dist/main.js',\n"
        f"      env:    {{ MAP: '{name}', PORT: {port}, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${{DOMAIN}}/ws/{name}` }},\n"
        f"    }},\n"
    )

# Replace everything between the lobby entry closing },  and the // Add more maps comment
new_eco = re.sub(
    r'(    \},\n)(.*?)(    // Add more maps here)',
    lambda m: m.group(1) + map_entries + m.group(3),
    eco,
    count=1,
    flags=re.DOTALL,
)

if new_eco == eco:
    print('ecosystem.config.js: no changes needed.')
else:
    with open(eco_path, 'w') as f:
        f.write(new_eco)
    print('ecosystem.config.js: updated.')

# ---------------------------------------------------------------------------
# 3. Update nginx-example.conf
# ---------------------------------------------------------------------------
nginx_path = f'{repo_dir}/server/nginx-example.conf'
with open(nginx_path) as f:
    nginx = f.read()

# Generate the map block entries (align port numbers at column 20 for readability)
nginx_entries = ''
for m in maps:
    entry = f'    ~^/ws/{m["name"]}'
    nginx_entries += f'{entry:<24}{m["port"]};\n'

# Replace the contents of the map $uri $ws_port { ... } block
new_nginx = re.sub(
    r'(map \$uri \$ws_port \{)[^}]*(\})',
    lambda m: m.group(1) + '\n' + nginx_entries + m.group(2),
    nginx,
    count=1,
)

if new_nginx == nginx:
    print('server/nginx-example.conf: no changes needed.')
else:
    with open(nginx_path, 'w') as f:
        f.write(new_nginx)
    print('server/nginx-example.conf: updated.')

print()
print('Done. Review the changes with:')
print('  git diff server/ecosystem.config.js server/nginx-example.conf')
print()
print('Then commit:')
print('  git add server/ecosystem.config.js server/nginx-example.conf')
print('  git commit -m "Update active map list"')
PYEOF
