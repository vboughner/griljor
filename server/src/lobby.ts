import * as http from 'http';
import { WebSocketServer, WebSocket } from 'ws';

interface GameEntry {
  mapName: string;
  title: string;
  teams: number;
  rooms: number;
  host: string;
  port: number;
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

function key(host: string, port: number): string {
  return `${host}:${port}`;
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

function isRegisterBody(b: unknown): b is { mapName: string; title?: string; teams?: number; rooms?: number; host: string; port: number; maxPlayers?: number } {
  return typeof b === 'object' && b !== null &&
    typeof (b as Record<string, unknown>).mapName === 'string' &&
    typeof (b as Record<string, unknown>).host === 'string' &&
    typeof (b as Record<string, unknown>).port === 'number';
}

function isHeartbeatBody(b: unknown): b is { host: string; port: number; players: number; avatars?: Array<{ avatar: string; name: string }> } {
  return typeof b === 'object' && b !== null &&
    typeof (b as Record<string, unknown>).host === 'string' &&
    typeof (b as Record<string, unknown>).port === 'number' &&
    typeof (b as Record<string, unknown>).players === 'number';
}

function isUnregisterBody(b: unknown): b is { host: string; port: number } {
  return typeof b === 'object' && b !== null &&
    typeof (b as Record<string, unknown>).host === 'string' &&
    typeof (b as Record<string, unknown>).port === 'number';
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
      const k = key(body.host, body.port);
      games.set(k, {
        mapName: body.mapName,
        title: body.title ?? body.mapName,
        teams: body.teams ?? 0,
        rooms: body.rooms ?? 0,
        host: body.host,
        port: body.port,
        players: 0,
        maxPlayers: body.maxPlayers ?? 16,
        avatars: [],
        lastSeen: Date.now(),
      });
      console.log(`[lobby] registered ${k} (${body.mapName})`);
      broadcast();
      send(200, { ok: true });
    } catch { send(400, { error: 'Bad request' }); }
    return;
  }

  if (method === 'POST' && url === '/heartbeat') {
    try {
      const body = await readBody(req);
      if (!isHeartbeatBody(body)) { send(400, { error: 'Bad body' }); return; }
      const k = key(body.host, body.port);
      const entry = games.get(k);
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
      const k = key(body.host, body.port);
      games.delete(k);
      console.log(`[lobby] unregistered ${k}`);
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

server.listen(3000, () => {
  console.log('Griljor lobby on :3000');
});
