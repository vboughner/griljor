import * as http from 'http';
import * as net from 'net';
import { WebSocketServer } from 'ws';
import { loadWorld } from './world';
import { GameSession } from './session';

const mapName = process.env.MAP ?? process.argv[2] ?? 'battle';
const LOBBY_URL = process.env.LOBBY_URL ?? 'http://localhost:3000';

function postJson(url: string, body: unknown): void {
  const data = JSON.stringify(body);
  const opts = new URL(url);
  const req = http.request({
    hostname: opts.hostname,
    port: opts.port,
    path: opts.pathname,
    method: 'POST',
    headers: { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(data) },
  });
  req.on('error', () => {}); // lobby being down is non-fatal
  req.write(data);
  req.end();
}

function findFreePort(startPort: number): Promise<number> {
  return new Promise((resolve, reject) => {
    const probe = net.createServer();
    probe.once('error', (err: NodeJS.ErrnoException) => {
      if (err.code === 'EADDRINUSE') {
        resolve(findFreePort(startPort + 1));
      } else {
        reject(err);
      }
    });
    probe.listen(startPort, () => {
      probe.close(() => resolve(startPort));
    });
  });
}

async function main(): Promise<void> {
  const world = await loadWorld(mapName);

  const server = http.createServer((_req, res) => {
    res.writeHead(200, {
      'Content-Type': 'text/plain',
      'Access-Control-Allow-Origin': '*',
    });
    res.end('Griljor server OK\n');
  });

  const wss = new WebSocketServer({ server, path: '/ws' });

  const startPort = parseInt(process.env.PORT ?? process.argv[3] ?? '3001', 10);
  const PORT = await findFreePort(startPort);
  await new Promise<void>((resolve) => server.listen(PORT, resolve));

  const wsUrl = process.env.PUBLIC_WS_URL ?? `ws://localhost:${PORT}/ws`;

  function sendHeartbeat(): void {
    postJson(`${LOBBY_URL}/heartbeat`, {
      wsUrl,
      players: game.playerCount,
      avatars: game.playerAvatars,
    });
  }

  const game = new GameSession(world, { onPlayerCountChange: sendHeartbeat });

  wss.on('connection', (ws) => {
    game.handleConnection(ws);
  });

  if (PORT !== startPort) {
    console.log(`Port ${startPort} in use, using :${PORT}`);
  }
  console.log(`Griljor server on :${PORT}, map: ${world.mapName} (${world.roomCount} rooms)`);

  // Register with lobby
  postJson(`${LOBBY_URL}/register`, {
    mapName,
    title: world.title,
    teams: world.teams,
    rooms: world.roomCount,
    wsUrl,
    maxPlayers: world.maxPlayers,
  });

  // Heartbeat every 5s (safety net; immediate heartbeats are sent on join/leave)
  setInterval(sendHeartbeat, 5_000);

  // Graceful shutdown
  const shutdown = () => {
    postJson(`${LOBBY_URL}/unregister`, { wsUrl });
    setTimeout(() => process.exit(0), 200);
  };
  process.on('SIGINT', shutdown);
  process.on('SIGTERM', shutdown);
}

main().catch((err) => {
  console.error('Server error:', err);
  process.exit(1);
});
