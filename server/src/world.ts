import { readFile } from 'fs/promises';
import { join } from 'path';

export interface World {
  mapName: string;
  roomCount: number;
}

export async function loadWorld(mapName: string): Promise<World> {
  // __dirname is server/src (ts-node) or server/dist (compiled); go up 2 levels to griljor root
  const mapPath = join(__dirname, '..', '..', 'pipeline', 'out', 'data', 'maps', `${mapName}.json`);
  const raw = await readFile(mapPath, 'utf-8');
  const data = JSON.parse(raw) as { rooms: unknown[] };
  return { mapName, roomCount: data.rooms.length };
}
