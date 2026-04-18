import { readFile } from 'fs/promises';
import { join } from 'path';

export interface ObjDef {
  _index: number;
  name?: string;
  type?: number; // bitmask for opens-matching (0 if absent = matches all openers)
  takeable?: boolean;
  weight?: number;
  numbered?: boolean;
  permeable?: boolean;
  movement?: number; // player walkability: absent/0 = blocked, 1–9 = walkable (9=fastest)
  weapon?: boolean;
  refire?: number; // fire rate modifier: -5 (slow) to 5 (fast); 0 = default 850ms cooldown
  damage?: number;
  range?: number;
  movingobj?: number;
  speed?: number;
  opens?: number; // non-zero: item can open swinging objects (bitmask)
  swings?: boolean; // true: this object can be toggled open/closed
  alternate?: number; // object type this becomes when toggled
  health?: number; // negative = restores HP on use
  lost?: boolean; // consumed on use (remove from inventory)
  charges?: number; // bitmask: this ammo item reloads weapons where weapon.type & charges != 0
  capacity?: number; // max charges a numbered weapon can hold
  stop?: boolean; // projectile stops on impact and lands on the floor
  explodes?: number; // non-zero: projectile explodes on impact (does not land as item)
  boombit?: number; // object type the weapon explodes into
  piercing?: number; // non-zero: explosion missiles pass through walls
  spread?: number; // number of evenly-spaced directions; 0 or absent = default 8
  directional?: boolean; // true: sprite direction matches missile travel direction
  transparent?: boolean; // if absent/false, tile blocks line of sight
}

export interface RecObj {
  x: number;
  y: number;
  type: number;
  detail: number;
}

export interface RoomData {
  name: string;
  floor: number; // default floor tile ID; 0 = void-floor map (battle-style)
  team: number; // 0 = neutral, 1+ = team-owned
  recorded_objects: RecObj[];
  spot?: number[][][]; // [x][y][2]; absent in diag-format maps
  exitNorth: number; // adjacent room index, -1 = no exit
  exitEast: number;
  exitSouth: number;
  exitWest: number;
}

export interface PlacementRule {
  mode: 't' | 'r';
  objType: number;
  quantity: number;
  target: number;
}

export interface PlacementConfig {
  intervalSeconds: number;
  rules: PlacementRule[];
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
  placement: PlacementConfig | null;
}

export async function loadWorld(mapName: string): Promise<World> {
  // __dirname is server/src (ts-node) or server/dist (compiled); go up 2 levels to griljor root
  const mapPath = join(__dirname, '..', '..', 'pipeline', 'out', 'data', 'maps', `${mapName}.json`);
  const raw = await readFile(mapPath, 'utf-8');
  const data = JSON.parse(raw) as {
    map: {
      name?: string;
      objfilename: string;
      teams_supported?: number;
      resetOnEmpty?: boolean;
      resetAfterSeconds?: number;
      maxPlayers?: number;
    };
    rooms: Array<{
      name?: string;
      floor?: number;
      team?: number;
      recorded_objects?: RecObj[];
      spot: number[][][];
      exit_north?: number;
      exit_east?: number;
      exit_south?: number;
      exit_west?: number;
    }>;
    placement?: {
      intervalSeconds: number;
      rules: Array<{ mode: string; objType: number; quantity: number; target: number }>;
    };
  };

  const objName = data.map.objfilename.replace(/\.obj$/, '');
  const objPath = join(
    __dirname,
    '..',
    '..',
    'pipeline',
    'out',
    'data',
    'objects',
    `${objName}.json`,
  );
  const objRaw = await readFile(objPath, 'utf-8');
  const objData = JSON.parse(objRaw) as { objects: Array<ObjDef | null> };

  const rooms: RoomData[] = data.rooms.map((r) => ({
    name: r.name ?? '',
    floor: r.floor ?? 0,
    team: r.team ?? 0,
    recorded_objects: r.recorded_objects ?? [],
    spot: r.spot,
    exitNorth: r.exit_north ?? -1,
    exitEast: r.exit_east ?? -1,
    exitSouth: r.exit_south ?? -1,
    exitWest: r.exit_west ?? -1,
  }));

  let placement: PlacementConfig | null = null;
  if (data.placement && data.placement.rules.length > 0) {
    placement = {
      intervalSeconds: data.placement.intervalSeconds,
      rules: data.placement.rules
        .filter((r) => r.mode === 't' || r.mode === 'r')
        .map((r) => ({
          mode: r.mode as 't' | 'r',
          objType: r.objType,
          quantity: r.quantity,
          target: r.target,
        }))
        .filter((r) => {
          const obj = objData.objects[r.objType];
          if (!obj) {
            console.warn(
              `[${mapName}] placement rule references unknown object ${r.objType} — skipped`,
            );
            return false;
          }
          if (!obj.takeable) {
            console.warn(
              `[${mapName}] placement rule references non-takeable object ${r.objType} (${obj.name}) — skipped`,
            );
            return false;
          }
          return true;
        }),
    };
    if (placement.rules.length === 0) placement = null;
  }

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
    placement,
  };
}
