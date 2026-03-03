import { MapFile, ObjectFile, ObjDef, RoomData } from './types';
import { loadMaskedSprite } from './assets';
import { preloadRoomSprites, renderRoom } from './renderer';

const GRID = 20;

interface ExitTile { destRoom: number; landX: number; landY: number; }

/** Returns true if the tile at (x, y) cannot be entered. */
function isTileBlocked(x: number, y: number, room: RoomData, objects: ObjDef[]): boolean {
  const [flId, wlId] = room.spot[x][y];
  if (wlId > 0 && !objects[wlId]?.permeable) return true;
  if (flId > 0 && !objects[flId]?.permeable) return true;
  return false;
}

/** Build a map from "x,y" → exit destination for all exit objects in the room. */
function buildExitMap(room: RoomData, objects: ObjDef[]): Map<string, ExitTile> {
  const map = new Map<string, ExitTile>();
  for (const ro of room.recorded_objects) {
    if (ro.detail < 0) continue;
    const obj = objects[ro.type];
    if (!obj?.exit) continue;
    const landX = ro.infox >= 0 ? ro.infox : ro.x;
    const landY = ro.infoy >= 0 ? ro.infoy : ro.y;
    map.set(`${ro.x},${ro.y}`, { destRoom: ro.detail, landX, landY });
  }
  return map;
}

export class Game {
  private mapData: MapFile;
  private objects: ObjDef[];
  private objset: string;
  private currentRoom: number = 0;
  private px: number = 10;
  private py: number = 10;
  private exitMap: Map<string, ExitTile> = new Map();
  private playerSprite: ImageData | null = null;
  private canvas: HTMLCanvasElement;
  private roomInfo: HTMLElement;
  private status: HTMLElement;

  constructor(
    mapData: MapFile,
    objFile: ObjectFile,
    canvas: HTMLCanvasElement,
    roomInfo: HTMLElement,
    status: HTMLElement,
    navBtns: Record<string, HTMLButtonElement>
  ) {
    this.mapData = mapData;
    this.objects = objFile.objects;
    this.objset = mapData.map.objfilename.replace(/\.obj$/, '');
    this.canvas = canvas;
    this.roomInfo = roomInfo;
    this.status = status;

    navBtns['north'].onclick = () => this.move(0, -1);
    navBtns['south'].onclick = () => this.move(0,  1);
    navBtns['east'].onclick  = () => this.move( 1, 0);
    navBtns['west'].onclick  = () => this.move(-1, 0);
    for (const btn of Object.values(navBtns)) btn.disabled = false;

    window.addEventListener('keydown', (e) => {
      const dirs: Record<string, [number, number]> = {
        ArrowUp: [0, -1], ArrowDown: [0, 1], ArrowRight: [1, 0], ArrowLeft: [-1, 0],
      };
      const d = dirs[e.key];
      if (d) { e.preventDefault(); this.move(d[0], d[1]); }
    });
  }

  async loadPlayerSprite(): Promise<void> {
    this.playerSprite = await loadMaskedSprite(
      '/sprites/facebits/crombit.png',
      '/sprites/facebits/crommask.png'
    );
  }

  async goToRoom(index: number, px = 10, py = 10): Promise<void> {
    if (index < 0 || index >= this.mapData.rooms.length) return;
    this.currentRoom = index;
    this.px = px;
    this.py = py;
    this.exitMap = buildExitMap(this.mapData.rooms[index], this.objects);
    await this.render();
  }

  private async move(dx: number, dy: number): Promise<void> {
    const room = this.mapData.rooms[this.currentRoom];
    let nx = this.px + dx;
    let ny = this.py + dy;

    // Room transitions at edges
    if (ny < 0 && room.exit_north >= 0) {
      await this.goToRoom(room.exit_north, nx, GRID - 1);
      return;
    }
    if (ny >= GRID && room.exit_south >= 0) {
      await this.goToRoom(room.exit_south, nx, 0);
      return;
    }
    if (nx >= GRID && room.exit_east >= 0) {
      await this.goToRoom(room.exit_east, 0, ny);
      return;
    }
    if (nx < 0 && room.exit_west >= 0) {
      await this.goToRoom(room.exit_west, GRID - 1, ny);
      return;
    }

    // Clamp to grid bounds
    nx = Math.max(0, Math.min(GRID - 1, nx));
    ny = Math.max(0, Math.min(GRID - 1, ny));

    // Exits take priority over collision — check before blocking
    const exit = this.exitMap.get(`${nx},${ny}`);
    if (exit) {
      await this.goToRoom(exit.destRoom, exit.landX, exit.landY);
      return;
    }

    // Block movement into walls
    if (isTileBlocked(nx, ny, room, this.objects)) return;

    this.px = nx;
    this.py = ny;

    await this.render();
  }

  private async render(): Promise<void> {
    const room = this.mapData.rooms[this.currentRoom];
    this.status.textContent = 'Loading room sprites…';
    await preloadRoomSprites(room, this.objects, this.objset);
    this.status.textContent = 'Rendering…';
    await renderRoom(this.canvas, room, this.objects, this.objset, this.playerSprite, this.px, this.py);
    this.roomInfo.textContent =
      `Room ${this.currentRoom}: "${room.name}" — (${this.px}, ${this.py})`;
    this.status.textContent = '';
  }
}
