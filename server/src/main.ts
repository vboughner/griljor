import * as http from 'http';
import { WebSocketServer } from 'ws';
import { loadWorld } from './world';
import { GameSession } from './session';

const PORT = 3001;
const mapName = process.argv[2] ?? 'battle';
const LOBBY_URL = 'http://localhost:3000';
const LOBBY_HOST = 'localhost';

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
  const game = new GameSession(world);

  wss.on('connection', (ws) => {
    game.handleConnection(ws);
  });

  server.listen(PORT, () => {
    console.log(`Griljor server on :${PORT}, map: ${world.mapName} (${world.roomCount} rooms)`);

    // Register with lobby
    postJson(`${LOBBY_URL}/register`, { mapName, host: LOBBY_HOST, port: PORT, maxPlayers: 16 });

    // Heartbeat every 5s
    setInterval(() => {
      postJson(`${LOBBY_URL}/heartbeat`, { host: LOBBY_HOST, port: PORT, players: game.playerCount });
    }, 5_000);
  });

  // Graceful shutdown
  const shutdown = () => {
    postJson(`${LOBBY_URL}/unregister`, { host: LOBBY_HOST, port: PORT });
    setTimeout(() => process.exit(0), 200);
  };
  process.on('SIGINT', shutdown);
  process.on('SIGTERM', shutdown);
}

main().catch((err) => {
  console.error('Server error:', err);
  process.exit(1);
});
