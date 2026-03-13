# Hosting Plan: Griljor Web Rewrite

## Context

The modern rewrite runs a Vite/TypeScript frontend, a single lobby HTTP server (port 3000), and multiple game server processes (WebSocket, ports 3001+). The goal is to host this publicly so friends can play, with 1 lobby instance and up to 10 game servers each running a different map.

## Platform: Hetzner Cloud

- CX22: 2 vCPU, 4 GB RAM, ~€3.29/month — plenty for lobby + 5 game servers
- Full Linux VM → nginx + Node + PM2
- Single VM hosts: lobby + game servers + static frontend
- HTTPS via Let's Encrypt (certbot) + own domain
- Frontend served by nginx from `client/dist/`

---

## Architecture on the VM

```
Internet (port 80/443 via nginx)
    │
    ├── /              → client/dist/ (static Vite build)
    ├── /games         → lobby:3000 (HTTP proxy)
    ├── /watch         → lobby:3000/watch (WebSocket, live game list)
    └── /ws/<mapname>  → game server:<port>/ws (WebSocket proxy)
```

nginx routes all traffic through standard ports. Each game server registers its public WebSocket path (e.g. `wss://griljor.com/ws/castle`) with the lobby instead of a raw host:port.

---

## Key nginx Notes (lessons learned)

- The `map` block must use `$uri` (not `$1`) as the key, with regex entries like `~^/ws/castle`
- `proxy_pass` for game servers must use `127.0.0.1` (not `localhost`) and must include the `/ws` path: `http://127.0.0.1:$ws_port/ws`
- Do NOT copy `nginx-example.conf` over the live config after certbot has run — certbot adds SSL lines that would be wiped. Use `sed` to edit the live config in place instead.
- Run certbot with an HTTP-only config first; certbot adds the SSL lines itself.
- After rebuilding `client/dist/`, re-run the chmod commands — the dist folder is recreated fresh.

---

## Code Changes Required

### 1. Make lobby URL configurable (server + client)

**`server/src/main.ts`** — replace hardcoded `http://localhost:3000`:
```ts
const LOBBY_URL = process.env.LOBBY_URL ?? 'http://localhost:3000';
```

**`client/src/lobby.ts`** — `VITE_LOBBY_URL` should be the origin only (no `/games` path):
```ts
const LOBBY_HTTP = import.meta.env.VITE_LOBBY_URL ?? 'http://localhost:3000';
const LOBBY_WS = LOBBY_HTTP.replace(/^http/, 'ws') + '/watch';
```

### 2. Make game server public address configurable

**`server/src/main.ts`** — registers with lobby using `wsUrl`:
```ts
const PUBLIC_WS_URL = process.env.PUBLIC_WS_URL;  // e.g. wss://griljor.com/ws/castle
```

### 3. Vite client build with env var

`client/.env.production` (gitignored) — use the bare origin, no path:
```
VITE_LOBBY_URL=https://griljor.com
```

---

## Server Infrastructure Files

### `server/ecosystem.config.js` (PM2 process file)

The active maps are configured here. See the file in the repo for the current list. Each map needs:
- A unique `PORT`
- A matching entry in `nginx-example.conf`'s `map` block

### `nginx-example.conf`

See the file in the repo root. This is the template — the live config on the server diverges after certbot adds SSL lines.

---

## Files to Create/Modify

| File | Change |
|------|--------|
| `server/src/main.ts` | Use `LOBBY_URL` + `PUBLIC_WS_URL` + `PORT` + `MAP` env vars |
| `server/src/lobby.ts` | Accept `wsUrl` field instead of `host`/`port` |
| `client/src/lobby.ts` | Use `VITE_LOBBY_URL` env var (origin only); derive WS URL |
| `client/src/network.ts` | Accept full WebSocket URL string (already does) |
| `server/ecosystem.config.js` | PM2 config for all processes |
| `client/.env.production` | Production lobby URL — origin only, no `/games` (gitignored) |
| `nginx-example.conf` | nginx config template |

---

## Hetzner Cloud VM Provisioning

Hetzner is the recommended host — cheap, reliable, and straightforward. The CX22 (2 vCPU, 4 GB RAM) is plenty for the lobby + 5 game servers.

### Create a Server

1. Sign in and click **"Add Server"** from the dashboard
2. **Location** — pick a region close to your players (Ashburn/Hillsboro for US, Helsinki/Nuremberg/Falkenstein for EU)
3. **Image** — select **Ubuntu 24.04**
4. **Type** — click **"Shared vCPU"** → **x86** → **CX22** (~€3.29/month)
5. **Networking** — leave defaults (public IPv4 included)
6. **SSH keys** — click "Add SSH key" and paste your public key (`~/.ssh/id_ed25519.pub` or `~/.ssh/id_rsa.pub`). If you don't have one, run `ssh-keygen` on your Mac first.
7. **Name** — e.g. `griljor`
8. Click **"Create & Buy Now"**

Takes ~30 seconds. Note the public IP address shown on the dashboard.

### SSH in

```sh
ssh -i ~/.ssh/id_ed25519 root@<IP_ADDRESS>
```

### Create a non-root user

```sh
adduser griljor
usermod -aG sudo griljor
rsync --archive --chown=griljor:griljor ~/.ssh /home/griljor
su - griljor
sudo whoami   # should print "root"
```

Use the `griljor` user for all subsequent steps.

---

## DNS Setup via Cloudflare

GreenGeeks (and many registrars) don't accept third-party nameservers directly. Cloudflare acts as a free DNS middleman.

1. Sign up at cloudflare.com (free plan)
2. Click **"Add a site"** → enter your domain (e.g. `griljor.com`)
3. Choose the **Free plan**
4. Cloudflare scans existing DNS, then gives you two nameservers like:
   ```
   aria.ns.cloudflare.com
   bob.ns.cloudflare.com
   ```
5. In your registrar (GreenGeeks), go to **Nameservers → Custom** and enter those two Cloudflare nameservers
6. In Cloudflare's DNS dashboard, add an A record:
   - Type: **A**, Name: **@**, Value: `<IP_ADDRESS>`, Proxy: **DNS only** (grey cloud — NOT orange)

> Keep the proxy set to **DNS only** (grey). The orange proxy intercepts WebSocket traffic and will break the game servers.

Check propagation:
```sh
dig griljor.com +short
```

---

## VM Setup Instructions

Run everything as the `griljor` user (with sudo access).

### 1. Install dependencies

```sh
# Node.js 20
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs nginx certbot python3-certbot-nginx git

# PM2
sudo npm install -g pm2
```

### 2. Clone and build

```sh
git clone -b modern-rewrite https://github.com/vboughner/griljor.git ~/griljor
cd ~/griljor

# Build server
cd server && npm install && npm run build && cd ..

# Build client with production lobby URL baked in
cd client
echo "VITE_LOBBY_URL=https://griljor.com" > .env.production
npm install && npm run build && cd ..
```

### 3. Configure nginx

Copy the template and fix the home directory path (the template uses `/home/ubuntu/` as a placeholder):

```sh
sudo cp ~/griljor/nginx-example.conf /etc/nginx/sites-available/griljor
sudo sed -i 's|/home/ubuntu/|/home/griljor/|g' /etc/nginx/sites-available/griljor
sudo ln -s /etc/nginx/sites-available/griljor /etc/nginx/sites-enabled/griljor
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t && sudo systemctl reload nginx
```

nginx runs as `www-data` and needs read access to the client build:

```sh
chmod o+x /home/griljor
chmod o+x /home/griljor/griljor
chmod o+x /home/griljor/griljor/client
chmod -R o+r /home/griljor/griljor/client/dist
```

### 4. HTTPS via Let's Encrypt

```sh
sudo certbot --nginx -d griljor.com
```

Certbot edits the live nginx config in-place to add SSL and the HTTP→HTTPS redirect. Certificates renew automatically via a systemd timer — no manual action needed.

> **Important**: after this point, never overwrite `/etc/nginx/sites-available/griljor` by copying from the repo. The live file has certbot SSL lines that the repo file does not. Use `sed` to make targeted edits to the live file instead.

### 5. Start everything with PM2

```sh
cd ~/griljor/server
pm2 start ecosystem.config.js
pm2 save
pm2 startup   # prints a command — copy/paste it to enable auto-start on reboot
```

### 6. Verify

```sh
pm2 status                       # all processes should show "online"
pm2 logs                         # watch for heartbeat messages
curl https://griljor.com/games   # should return JSON game list
```

Then open `https://griljor.com` in a browser — title screen should appear and the lobby should list all maps.

---

## Managing Maps

Use the scripts in `scripts/` to add or remove maps without manually editing configs.

### Add a map

```sh
bash ~/griljor/scripts/add-map.sh <mapname> <port>
# Example:
bash ~/griljor/scripts/add-map.sh battle 3005
```

Available map names: see `pipeline/out/data/maps/` (use the filename without `.json`).
Ports already in use: 3000 (lobby), 3002 (castle), 3003 (paradise), 3004 (flag).

The script starts the PM2 process, adds the nginx route, reloads nginx, and saves the PM2 process list.

### Remove a map

```sh
bash ~/griljor/scripts/remove-map.sh <mapname>
# Example:
bash ~/griljor/scripts/remove-map.sh battle
```

The script stops and deletes the PM2 process, removes the nginx route, reloads nginx, and saves the PM2 process list.

### Sync repo config files after adding or removing a map

After adding or removing maps, run this to update `server/ecosystem.config.js` and `nginx-example.conf` to match the live PM2 state:

```sh
bash ~/griljor/scripts/sync-repo-config.sh
git diff server/ecosystem.config.js nginx-example.conf
git add server/ecosystem.config.js nginx-example.conf
git commit -m "Update active map list"
```

### If a map server fails to appear in the lobby

It may have started before the lobby was ready. Restart it:
```sh
pm2 restart <mapname>
```

---

## Updating the Game

```sh
cd ~/griljor && git pull
bash ~/griljor/scripts/rebuild-restart-production.sh
```

---

## Troubleshooting

| Symptom | Check |
|---------|-------|
| 500 on the site | `sudo tail -20 /var/log/nginx/error.log` — likely a path or permission issue |
| `/games/games` 404 | `VITE_LOBBY_URL` has `/games` appended — it should be the bare origin only |
| WebSocket connection refused | nginx not routing — check the `map` block uses `$uri` with regex entries |
| Game server shows online but not in lobby | `pm2 restart <mapname>` — it started before the lobby was ready |
| nginx test fails after editing | Never copy repo config over live config after certbot has run |

---

## Verification Checklist

- [ ] Local dev still works: `npm run dev` in both `server/` and `client/` with no env vars set
- [ ] `pm2 status` shows all processes green
- [ ] `https://griljor.com` serves title screen
- [ ] Game list shows maps from lobby
- [ ] Join a game, play with a friend from a different network
- [ ] `pm2 logs` shows heartbeats every 5s from each game server
- [ ] VM reboot → all processes restart automatically via `pm2 startup`
