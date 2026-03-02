export interface RecObj {
  x: number;
  y: number;
  type: number;
  detail: number;
  infox: number;
  infoy: number;
  zinger: number;
  extra: number[];
}

export interface RoomData {
  name: string;
  floor: number;
  team: number;
  exit_north: number;
  exit_east: number;
  exit_south: number;
  exit_west: number;
  appearance: number;
  dark: number;
  spot: number[][][]; // [x][y][2] — [floor_id, wall_id]
  recorded_objects: RecObj[];
}

export interface MapMeta {
  name: string;
  objfilename: string;
  teams_supported: number;
  team_name_1: string;
  team_name_2: string;
}

export interface MapFile {
  source: string;
  format: string;
  map: MapMeta;
  rooms: RoomData[];
  room_count: number;
}

export interface ObjDef {
  _index: number;
  name?: string;
  bitmap?: string;
  mask?: string;
  masked?: boolean;
  transparent?: boolean;
  permeable?: boolean;
  movement?: number;
}

export interface ObjectFile {
  source: string;
  count: number;
  objects: ObjDef[];
}
