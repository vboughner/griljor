import * as http from 'http';
import { WebSocketServer, WebSocket } from 'ws';

interface GameEntry {
  mapName: string;
  title: string;
  teams: number;
  rooms: number;
  wsUrl: string;
  players: number;
  maxPlayers: number;
  avatars: Array<{ avatar: string; name: string }>;
  lastSeen: number;
}

const TTL_MS = 15_000;
const games = new Map<string, GameEntry>();
const watchers = new Set<WebSocket>();

function broadcast(): void {
  purgeStale();
  const payload = JSON.stringify([...games.values()]);
  for (const ws of watchers) {
    if (ws.readyState === WebSocket.OPEN) ws.send(payload);
  }
}

function purgeStale(): void {
  const now = Date.now();
  for (const [k, entry] of games) {
    if (now - entry.lastSeen > TTL_MS) games.delete(k);
  }
}

async function readBody(req: http.IncomingMessage): Promise<unknown> {
  return new Promise((resolve, reject) => {
    let raw = '';
    req.on('data', (chunk) => { raw += chunk; });
    req.on('end', () => {
      try { resolve(JSON.parse(raw)); }
      catch { reject(new Error('Invalid JSON')); }
    });
    req.on('error', reject);
  });
}

function isRegisterBody(b: unknown): b is { mapName: string; title?: string; teams?: number; rooms?: number; wsUrl: string; maxPlayers?: number } {
  return typeof b === 'object' && b !== null &&
    typeof (b as Record<string, unknown>).mapName === 'string' &&
    typeof (b as Record<string, unknown>).wsUrl === 'string';
}

function isHeartbeatBody(b: unknown): b is { wsUrl: string; players: number; avatars?: Array<{ avatar: string; name: string }> } {
  return typeof b === 'object' && b !== null &&
    typeof (b as Record<string, unknown>).wsUrl === 'string' &&
    typeof (b as Record<string, unknown>).players === 'number';
}

function isUnregisterBody(b: unknown): b is { wsUrl: string } {
  return typeof b === 'object' && b !== null &&
    typeof (b as Record<string, unknown>).wsUrl === 'string';
}

const CORS = {
  'Access-Control-Allow-Origin': '*',
  'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
  'Access-Control-Allow-Headers': 'Content-Type',
};

const server = http.createServer(async (req, res) => {
  const method = req.method ?? 'GET';
  const url = req.url ?? '/';

  // Preflight
  if (method === 'OPTIONS') {
    res.writeHead(204, CORS);
    res.end();
    return;
  }

  const send = (status: number, body: unknown) => {
    res.writeHead(status, { 'Content-Type': 'application/json', ...CORS });
    res.end(JSON.stringify(body));
  };

  if (method === 'POST' && url === '/register') {
    try {
      const body = await readBody(req);
      if (!isRegisterBody(body)) { send(400, { error: 'Bad body' }); return; }
      games.set(body.wsUrl, {
        mapName: body.mapName,
        title: body.title ?? body.mapName,
        teams: body.teams ?? 0,
        rooms: body.rooms ?? 0,
        wsUrl: body.wsUrl,
        players: 0,
        maxPlayers: body.maxPlayers ?? 16,
        avatars: [],
        lastSeen: Date.now(),
      });
      console.log(`[lobby] registered ${body.wsUrl} (${body.mapName})`);
      broadcast();
      send(200, { ok: true });
    } catch { send(400, { error: 'Bad request' }); }
    return;
  }

  if (method === 'POST' && url === '/heartbeat') {
    try {
      const body = await readBody(req);
      if (!isHeartbeatBody(body)) { send(400, { error: 'Bad body' }); return; }
      const entry = games.get(body.wsUrl);
      if (entry) {
        entry.players = body.players;
        entry.avatars = body.avatars ?? [];
        entry.lastSeen = Date.now();
        broadcast();
      }
      send(200, { ok: true });
    } catch { send(400, { error: 'Bad request' }); }
    return;
  }

  if (method === 'POST' && url === '/unregister') {
    try {
      const body = await readBody(req);
      if (!isUnregisterBody(body)) { send(400, { error: 'Bad body' }); return; }
      games.delete(body.wsUrl);
      console.log(`[lobby] unregistered ${body.wsUrl}`);
      broadcast();
      send(200, { ok: true });
    } catch { send(400, { error: 'Bad request' }); }
    return;
  }

  if (method === 'GET' && url === '/games') {
    purgeStale();
    send(200, [...games.values()]);
    return;
  }

  send(404, { error: 'Not found' });
});

const wss = new WebSocketServer({ server, path: '/watch' });

wss.on('connection', (ws) => {
  watchers.add(ws);
  // Send current list immediately on connect
  purgeStale();
  ws.send(JSON.stringify([...games.values()]));
  ws.on('close', () => watchers.delete(ws));
  ws.on('error', () => watchers.delete(ws));
});

const PORT = parseInt(process.env.PORT ?? '3000', 10);
server.listen(PORT, () => {
  console.log(`Griljor lobby on :${PORT}`);
});
