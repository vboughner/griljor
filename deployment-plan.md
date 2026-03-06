# Hosting Plan: Griljor Web Rewrite

## Context

The modern rewrite runs a Vite/TypeScript frontend, a single lobby HTTP server (port 3000), and multiple game server processes (WebSocket, ports 3001+). Currently everything is hardcoded to `localhost`. The goal is to host this publicly for free so friends can play, with 1 lobby instance and 5–10 game servers each running a different map.

## Platform: Oracle Cloud Always Free Tier

- Truly free forever: up to 4 ARM cores + 24 GB RAM (Ampere A1)
- Full Linux VM → nginx + Node + PM2
- Single VM hosts: lobby + 10 game servers + static frontend
- HTTPS via Let's Encrypt (certbot) + user's own domain
- Frontend served by nginx from `client/dist/` (same VM, one domain)

---

## Architecture on the VM

```
Internet (port 80/443 via nginx)
    │
    ├── /              → client/dist/ (static Vite build)
    ├── /games         → lobby:3000 (HTTP proxy)
    └── /ws/<mapname>  → game server:<port> (WebSocket proxy)
```

nginx routes all traffic through standard ports. Each game server registers its public WebSocket path (e.g. `wss://griljor.example.com/ws/battle`) with the lobby instead of a raw host:port.

---

## Code Changes Required

### 1. Make lobby URL configurable (server + client)

**`server/src/main.ts` line 8** — replace hardcoded `http://localhost:3000`:
```ts
const LOBBY_URL = process.env.LOBBY_URL ?? 'http://localhost:3000';
```

**`client/src/lobby.ts` line 9** — replace hardcoded `http://localhost:3000`:
```ts
const LOBBY_URL = import.meta.env.VITE_LOBBY_URL ?? 'http://localhost:3000';
```

### 2. Make game server public address configurable

**`server/src/main.ts`** — when registering with lobby, send the public WebSocket URL instead of `host:port`:
```ts
const PUBLIC_WS_URL = process.env.PUBLIC_WS_URL;  // e.g. wss://griljor.example.com/ws/battle
// Registration payload becomes { mapName, wsUrl, maxPlayers }
```

Lobby `GET /games` response changes `host`/`port` to `wsUrl` so client connects to it directly.

### 3. Client WebSocket connection

**`client/src/network.ts`** — connection uses the `wsUrl` from the game list directly (already a full ws:// URL in production), falls back to constructing `ws://localhost:{port}/ws` in dev.

### 4. Vite client build with env var

Add `client/.env.production` (gitignored):
```
VITE_LOBBY_URL=https://griljor.example.com/games
```

---

## Server Infrastructure Files to Create

### `server/ecosystem.config.js` (PM2 process file)
```js
module.exports = {
  apps: [
    { name: 'lobby',   script: 'dist/lobby.js', env: { PORT: 3000 } },
    { name: 'battle',  script: 'dist/main.js', env: { MAP: 'battle',  PORT: 3001, PUBLIC_WS_URL: 'wss://griljor.example.com/ws/battle',  LOBBY_URL: 'http://localhost:3000' } },
    { name: 'castle',  script: 'dist/main.js', env: { MAP: 'castle',  PORT: 3002, PUBLIC_WS_URL: 'wss://griljor.example.com/ws/castle',  LOBBY_URL: 'http://localhost:3000' } },
    // ... up to 10 maps
  ]
}
```

### `nginx-example.conf`
```nginx
server {
    server_name griljor.example.com;

    location / {
        root /home/ubuntu/griljor/client/dist;
        try_files $uri $uri/ /index.html;
    }

    location /games {
        proxy_pass http://localhost:3000/games;
        proxy_set_header Host $host;
    }

    # Map map name to port
    map $1 $ws_port {
        battle  3001;
        castle  3002;
        # add more maps here
    }

    location ~ ^/ws/(.+)$ {
        proxy_pass http://localhost:$ws_port;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
    }

    listen 443 ssl;
    ssl_certificate     /etc/letsencrypt/live/griljor.example.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/griljor.example.com/privkey.pem;
}

server {
    listen 80;
    server_name griljor.example.com;
    return 301 https://$host$request_uri;
}
```

---

## Asset Serving

The `pipeline/out/` assets (maps JSON, sprites PNG, objects JSON) are currently served by Vite's `publicDir`. In production:
- Include `pipeline/out/` in the Vite build's `publicDir` (already configured)
- Assets land in `client/dist/` after `npm run build`
- nginx serves them as static files alongside the HTML/JS

No CDN needed unless assets get large enough to matter.

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

## Deployment Steps (once VM is set up)

1. Provision Oracle Cloud Always Free ARM VM (Ubuntu 22.04)
2. Install Node.js 20+, nginx, certbot, PM2
3. `git clone` repo onto VM
4. `cd server && npm install && npm run build`
5. `cd client && npm run build` (with `client/.env.production` containing correct domain)
6. `pm2 start server/ecosystem.config.js`
7. Configure nginx using `nginx-example.conf` as template (replace `griljor.example.com` with real domain)
8. `certbot --nginx -d griljor.example.com` (Let's Encrypt HTTPS)
9. `pm2 save && pm2 startup` (auto-restart on VM reboot)

---

## Oracle Cloud VM Provisioning

### Create the VM

1. Sign in to [cloud.oracle.com](https://cloud.oracle.com)
2. Go to **Compute → Instances → Create Instance**
3. **Name**: anything (e.g. `griljor`)
4. **Image**: click "Change image" → **Canonical Ubuntu** → **22.04** → confirm
5. **Shape**: click "Change shape" → **Ampere** tab → **VM.Standard.A1.Flex**
   - Set OCPUs to **4** and RAM to **24 GB** (the full free allowance)
6. **Networking**: leave defaults (creates a new VCN automatically)
7. **SSH keys**: paste your existing public key, or let Oracle generate one and download it
8. Click **Create**

Wait ~2 minutes for it to reach "Running" state. Note the **Public IP address**.

### Open ports 80 and 443

Oracle's default firewall blocks everything except port 22. Two things to fix:

**A. Security List (Oracle's firewall)**
1. Go to **Networking → Virtual Cloud Networks** → click your VCN → **Security Lists** → **Default Security List**
2. Click **Add Ingress Rules** and add two rules:
   - Source: `0.0.0.0/0`, Protocol: TCP, Destination port: **80**
   - Source: `0.0.0.0/0`, Protocol: TCP, Destination port: **443**

**B. iptables on the VM (Ubuntu's OS firewall)**

SSH in first:
```sh
ssh -i ~/your-key.pem ubuntu@<PUBLIC_IP>
```

Then open the ports:
```sh
sudo iptables -I INPUT -p tcp --dport 80  -j ACCEPT
sudo iptables -I INPUT -p tcp --dport 443 -j ACCEPT
sudo netfilter-persistent save
```

### Point your domain at the VM

In your DNS provider, add an **A record**:
```
yourdomain.com  →  <PUBLIC_IP>
```

Check propagation with:
```sh
dig yourdomain.com +short
```

---

## VM Setup Instructions

These steps assume Ubuntu 22.04 (Oracle Cloud Ampere A1 or any VPS). Run everything as a non-root user with sudo access.

### 1. Provision the VM

On Oracle Cloud Always Free: create an **Ampere A1** instance (ARM64, Ubuntu 22.04). Open ports 80 and 443 in the security list / firewall rules.

### 2. Install dependencies

```sh
# Node.js 20
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs nginx certbot python3-certbot-nginx git

# PM2
sudo npm install -g pm2
```

### 3. Clone and build

```sh
git clone https://github.com/youruser/griljor.git ~/griljor
cd ~/griljor

# Build server
cd server && npm install && npm run build && cd ..

# Build client with production lobby URL baked in
cd client
echo "VITE_LOBBY_URL=https://yourdomain.com/games" > .env.production
npm install && npm run build && cd ..
```

### 4. Edit the PM2 config

In `server/ecosystem.config.js`, replace `griljor.example.com` with your domain and add your maps:

```js
const DOMAIN = 'yourdomain.com';

apps: [
  { name: 'lobby',   script: 'dist/lobby.js', env: { PORT: 3000 } },
  { name: 'battle',  script: 'dist/main.js', env: { MAP: 'battle',  PORT: 3001, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/battle`  } },
  { name: 'castle',  script: 'dist/main.js', env: { MAP: 'castle',  PORT: 3002, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/castle`  } },
  { name: 'forest',  script: 'dist/main.js', env: { MAP: 'forest',  PORT: 3003, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/forest`  } },
  { name: 'dungeon', script: 'dist/main.js', env: { MAP: 'dungeon', PORT: 3004, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/dungeon` } },
  { name: 'arena',   script: 'dist/main.js', env: { MAP: 'arena',   PORT: 3005, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/arena`   } },
]
```

Also add the corresponding entries in `nginx-example.conf`'s `map` block:

```nginx
map $1 $ws_port {
    battle  3001;
    castle  3002;
    forest  3003;
    dungeon 3004;
    arena   3005;
}
```

### 5. Configure nginx

```sh
sudo cp ~/griljor/nginx-example.conf /etc/nginx/sites-available/griljor
sudo sed -i 's/griljor.example.com/yourdomain.com/g' /etc/nginx/sites-available/griljor
sudo ln -s /etc/nginx/sites-available/griljor /etc/nginx/sites-enabled/griljor
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t && sudo systemctl reload nginx
```

### 6. HTTPS via Let's Encrypt

```sh
sudo certbot --nginx -d yourdomain.com
```

This edits the nginx config in-place to add SSL certs and auto-redirect HTTP→HTTPS.

### 7. Start everything with PM2

```sh
cd ~/griljor/server
pm2 start ecosystem.config.js
pm2 save
pm2 startup   # prints a command — copy/paste it to enable auto-start on reboot
```

### 8. Verify

```sh
pm2 status          # all 6 processes should show "online"
pm2 logs            # watch heartbeat messages
curl https://yourdomain.com/games   # should return JSON game list
```

Then open `https://yourdomain.com` in a browser — title screen should appear and the lobby should list all maps.

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
- [ ] `https://griljor.example.com` serves title screen
- [ ] Game list shows 5–10 maps from lobby
- [ ] Join a game, play with a friend from a different network
- [ ] `pm2 logs` shows heartbeats every 5s from each game server
- [ ] VM reboot → all processes restart automatically via `pm2 startup`
