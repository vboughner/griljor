import * as http from 'http';
import { WebSocketServer } from 'ws';
import { loadWorld } from './world';
import { GameSession } from './session';

const PORT = 3001;
const mapName = process.argv[2] ?? 'battle';

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
  });
}

main().catch((err) => {
  console.error('Server error:', err);
  process.exit(1);
});
