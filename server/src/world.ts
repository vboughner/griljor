import { readFile } from 'fs/promises';
import { join } from 'path';

export interface ObjDef {
  _index: number;
  name?: string;
  takeable?: boolean;
  weight?: number;
  numbered?: boolean;
  permeable?: boolean;
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
  spot: number[][][]; // [x][y][2]
}

export interface World {
  mapName: string;
  roomCount: number;
  rooms: RoomData[];
  objects: Array<ObjDef | null>;
}

export async function loadWorld(mapName: string): Promise<World> {
  // __dirname is server/src (ts-node) or server/dist (compiled); go up 2 levels to griljor root
  const mapPath = join(__dirname, '..', '..', 'pipeline', 'out', 'data', 'maps', `${mapName}.json`);
  const raw = await readFile(mapPath, 'utf-8');
  const data = JSON.parse(raw) as {
    map: { objfilename: string };
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
    roomCount: rooms.length,
    rooms,
    objects: objData.objects,
  };
}
