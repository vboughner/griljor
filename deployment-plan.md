# Hosting Plan: Griljor Web Rewrite

## Context

The modern rewrite runs a Vite/TypeScript frontend, a single lobby HTTP server (port 3000), and multiple game server processes (WebSocket, ports 3001+). The goal is to host this publicly so friends can play, with 1 lobby instance and 5–10 game servers each running a different map.

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
    └── /ws/<mapname>  → game server:<port> (WebSocket proxy)
```

nginx routes all traffic through standard ports. Each game server registers its public WebSocket path (e.g. `wss://griljor.com/ws/battle`) with the lobby instead of a raw host:port.

---

## Code Changes Required

### 1. Make lobby URL configurable (server + client)

**`server/src/main.ts`** — replace hardcoded `http://localhost:3000`:
```ts
const LOBBY_URL = process.env.LOBBY_URL ?? 'http://localhost:3000';
```

**`client/src/lobby.ts`** — replace hardcoded `http://localhost:3000`:
```ts
const LOBBY_URL = import.meta.env.VITE_LOBBY_URL ?? 'http://localhost:3000';
```

### 2. Make game server public address configurable

**`server/src/main.ts`** — when registering with lobby, send the public WebSocket URL instead of `host:port`:
```ts
const PUBLIC_WS_URL = process.env.PUBLIC_WS_URL;  // e.g. wss://griljor.com/ws/battle
// Registration payload becomes { mapName, wsUrl, maxPlayers }
```

Lobby `GET /games` response uses `wsUrl` so client connects to it directly.

### 3. Client WebSocket connection

**`client/src/network.ts`** — connection uses the `wsUrl` from the game list directly (already a full ws:// URL in production), falls back to `ws://localhost:{port}/ws` in dev.

### 4. Vite client build with env var

Add `client/.env.production` (gitignored):
```
VITE_LOBBY_URL=https://griljor.com/games
```

---

## Server Infrastructure Files

### `server/ecosystem.config.js` (PM2 process file)
```js
module.exports = {
  apps: [
    { name: 'lobby',   script: 'dist/lobby.js', env: { PORT: 3000 } },
    { name: 'battle',  script: 'dist/main.js', env: { MAP: 'battle',  PORT: 3001, PUBLIC_WS_URL: 'wss://griljor.com/ws/battle',  LOBBY_URL: 'http://localhost:3000' } },
    { name: 'castle',  script: 'dist/main.js', env: { MAP: 'castle',  PORT: 3002, PUBLIC_WS_URL: 'wss://griljor.com/ws/castle',  LOBBY_URL: 'http://localhost:3000' } },
    // ... up to 10 maps
  ]
}
```

### `nginx-example.conf`
See the file in the repo root. Key sections: static frontend, `/games` HTTP proxy, `/watch` WebSocket proxy for lobby, `/ws/<mapname>` WebSocket proxy per game server.

---

## Asset Serving

The `pipeline/out/` assets (maps JSON, sprites PNG, objects JSON) are served by Vite's `publicDir`. In production:
- Assets land in `client/dist/` after `npm run build`
- nginx serves them as static files alongside the HTML/JS

---

## Files to Create/Modify

| File | Change |
|------|--------|
| `server/src/main.ts` | Use `LOBBY_URL` + `PUBLIC_WS_URL` env vars |
| `server/src/lobby.ts` | Accept `wsUrl` field instead of `host`/`port` |
| `client/src/lobby.ts` | Use `VITE_LOBBY_URL` env var; connect using `wsUrl` directly |
| `client/src/network.ts` | Accept full WebSocket URL string (already does) |
| `server/ecosystem.config.js` | New: PM2 config for all processes |
| `client/.env.production` | New: production lobby URL (gitignored) |
| `nginx-example.conf` | New: nginx config example |

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
git clone https://github.com/vboughner/griljor.git ~/griljor
cd ~/griljor

# Build server
cd server && npm install && npm run build && cd ..

# Build client with production lobby URL baked in
cd client
echo "VITE_LOBBY_URL=https://griljor.com/games" > .env.production
npm install && npm run build && cd ..
```

### 3. Edit the PM2 config

In `server/ecosystem.config.js`, replace `griljor.example.com` with your domain and add your maps:

```js
const DOMAIN = 'griljor.com';

  apps: [
    {
      name:   'lobby',
      script: 'dist/lobby.js',
      env:    { PORT: 3000 },
    },
    {
      name:   'battle',
      script: 'dist/main.js',
      env:    { MAP: 'battle', PORT: 3001, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/battle` },
    },
    {
      name:   'castle',
      script: 'dist/main.js',
      env:    { MAP: 'castle', PORT: 3002, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/castle` },
    },
    {
      name:   'paradise',
      script: 'dist/main.js',
      env:    { MAP: 'paradise', PORT: 3003, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/paradise` },
    },
    {
      name:   'flag',
      script: 'dist/main.js',
      env:    { MAP: 'flag', PORT: 3004, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/flag` },
    },
    {
      name:   'hometown',
      script: 'dist/main.js',
      env:    { MAP: 'hometown', PORT: 3005, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/hometown` },
    },
    {
      name:   'sword',
      script: 'dist/main.js',
      env:    { MAP: 'sword', PORT: 3006, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/sword` },
    },
    {
      name:   'twoperson',
      script: 'dist/main.js',
      env:    { MAP: 'twoperson', PORT: 3007, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/twoperson` },
    },
    {
      name:   'trek',
      script: 'dist/main.js',
      env:    { MAP: 'trek', PORT: 3008, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/trek` },
    },
    {
      name:   'title',
      script: 'dist/main.js',
      env:    { MAP: 'title', PORT: 3009, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/title` },
    },
    // Add more maps here following the same pattern.
    // Each map needs a unique PORT and a matching /ws/<mapname> block in nginx-example.conf.
  ],
```

Also add the corresponding entries in `nginx-example.conf`'s `map` block:

```nginx
map $1 $ws_port {
    battle  3001;
    castle  3002;
    paradise  3003;
    flag  3004;
    hometown  3005;
    sword  3006;
    twoperson  3007;
    trek  3008;
    title  3009;
}
```

### 4. Configure nginx

```sh
sudo cp ~/griljor/nginx-example.conf /etc/nginx/sites-available/griljor
sudo sed -i 's|/home/ubuntu/|/home/griljor/|g' /etc/nginx/sites-available/griljor
sudo ln -s /etc/nginx/sites-available/griljor /etc/nginx/sites-enabled/griljor
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t && sudo systemctl reload nginx
```

nginx runs as `www-data` and needs read access to the client build. Grant it:

```sh
chmod o+x /home/griljor
chmod o+x /home/griljor/griljor
chmod o+x /home/griljor/griljor/client
chmod -R o+r /home/griljor/griljor/client/dist
```

### 5. HTTPS via Let's Encrypt

```sh
sudo certbot --nginx -d griljor.com
```

This edits the nginx config in-place to add SSL certs and auto-redirect HTTP→HTTPS.

### 6. Start everything with PM2

```sh
cd ~/griljor/server
pm2 start ecosystem.config.js
pm2 save
pm2 startup   # prints a command — copy/paste it to enable auto-start on reboot
```

### 7. Verify

```sh
pm2 status          # all 6 processes should show "online"
pm2 logs            # watch heartbeat messages
curl https://griljor.com/games   # should return JSON game list
```

Then open `https://griljor.com` in a browser — title screen should appear and the lobby should list all maps.

### Updating later

```sh
cd ~/griljor
git pull
cd server && npm run build && cd ..
cd client && npm run build && cd ..
pm2 restart all
```

---

## Verification Checklist

- [ ] Local dev still works: `npm run dev` in both `server/` and `client/` with no env vars set
- [ ] `pm2 status` shows all processes green
- [ ] `https://griljor.com` serves title screen
- [ ] Game list shows maps from lobby
- [ ] Join a game, play with a friend from a different network
- [ ] `pm2 logs` shows heartbeats every 5s from each game server
- [ ] VM reboot → all processes restart automatically via `pm2 startup`
