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

## Verification Checklist

- [ ] Local dev still works: `npm run dev` in both `server/` and `client/` with no env vars set
- [ ] `pm2 status` shows all processes green
- [ ] `https://griljor.example.com` serves title screen
- [ ] Game list shows 5–10 maps from lobby
- [ ] Join a game, play with a friend from a different network
- [ ] `pm2 logs` shows heartbeats every 5s from each game server
- [ ] VM reboot → all processes restart automatically via `pm2 startup`
