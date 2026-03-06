import { readFile } from 'fs/promises';
import { join } from 'path';

export interface ObjDef {
  _index: number;
  name?: string;
  type?: number;        // bitmask for opens-matching (0 if absent = matches all openers)
  takeable?: boolean;
  weight?: number;
  numbered?: boolean;
  permeable?: boolean;
  weapon?: boolean;
  damage?: number;
  range?: number;
  movingobj?: number;
  speed?: number;
  opens?: number;       // non-zero: item can open swinging objects (bitmask)
  swings?: boolean;     // true: this object can be toggled open/closed
  alternate?: number;   // object type this becomes when toggled
}

export interface RecObj {
  x: number;
  y: number;
  type: number;
  detail: number;
}

export interface RoomData {
  name: string;
  recorded_objects: RecObj[];
  spot?: number[][][]; // [x][y][2]; absent in diag-format maps
}

export interface World {
  mapName: string;
  title: string;
  teams: number;
  roomCount: number;
  rooms: RoomData[];
  objects: Array<ObjDef | null>;
  resetOnEmpty: boolean;
  resetAfterSeconds: number;
  maxPlayers: number;
}

export async function loadWorld(mapName: string): Promise<World> {
  // __dirname is server/src (ts-node) or server/dist (compiled); go up 2 levels to griljor root
  const mapPath = join(__dirname, '..', '..', 'pipeline', 'out', 'data', 'maps', `${mapName}.json`);
  const raw = await readFile(mapPath, 'utf-8');
  const data = JSON.parse(raw) as {
    map: { name?: string; objfilename: string; teams_supported?: number; resetOnEmpty?: boolean; resetAfterSeconds?: number; maxPlayers?: number };
    rooms: Array<{ recorded_objects?: RecObj[]; spot: number[][][] }>;
  };

  const objName = data.map.objfilename.replace(/\.obj$/, '');
  const objPath = join(__dirname, '..', '..', 'pipeline', 'out', 'data', 'objects', `${objName}.json`);
  const objRaw = await readFile(objPath, 'utf-8');
  const objData = JSON.parse(objRaw) as { objects: Array<ObjDef | null> };

  const rooms: RoomData[] = data.rooms.map((r) => ({
    name: (r as { name?: string }).name ?? '',
    recorded_objects: r.recorded_objects ?? [],
    spot: r.spot,
  }));

  return {
    mapName,
    title: data.map.name ?? mapName,
    teams: data.map.teams_supported ?? 0,
    roomCount: rooms.length,
    rooms,
    objects: objData.objects,
    resetOnEmpty: data.map.resetOnEmpty ?? false,
    resetAfterSeconds: data.map.resetAfterSeconds ?? 30,
    maxPlayers: data.map.maxPlayers ?? 16,
  };
}
