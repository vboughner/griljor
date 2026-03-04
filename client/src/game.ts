import { MapFile, ObjectFile, ObjDef, RoomData } from './types';
import { loadMaskedSprite, setColorMode, ColorMode } from './assets';
import { preloadRoomSprites, buildRoomBackground, renderFrame, OtherPlayer } from './renderer';
import { GameNetwork } from './network';

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

interface RemotePlayer {
  id: number;
  name: string;
  avatar: string;
  room: number;
  x: number;
  y: number;
  sprite: ImageData | null;
}

export class Game {
  private mapData: MapFile;
  private objects: ObjDef[];
  private objset: string;
  private currentRoom: number = 0;
  private px: number = 10;
  private py: number = 10;
  private exitMap: Map<string, ExitTile> = new Map();
  private roomBg: OffscreenCanvas | null = null;
  private playerSprite: ImageData | null = null;
  private avatarName: string = 'crom';
  private canvas: HTMLCanvasElement;
  private roomInfo: HTMLElement;
  private status: HTMLElement;
  private onKeyDown!: (e: KeyboardEvent) => void;

  private network: GameNetwork | null = null;
  private myId: number | null = null;
  private otherPlayers = new Map<number, RemotePlayer>();

  constructor(
    mapData: MapFile,
    objFile: ObjectFile,
    canvas: HTMLCanvasElement,
    roomInfo: HTMLElement,
    status: HTMLElement,
    navBtns: Record<string, HTMLButtonElement>,
    network?: GameNetwork
  ) {
    this.mapData = mapData;
    this.objects = objFile.objects;
    this.objset = mapData.map.objfilename.replace(/\.obj$/, '');
    this.canvas = canvas;
    this.roomInfo = roomInfo;
    this.status = status;
    this.network = network ?? null;

    navBtns['north'].onclick = () => this.move(0, -1);
    navBtns['south'].onclick = () => this.move(0,  1);
    navBtns['east'].onclick  = () => this.move( 1, 0);
    navBtns['west'].onclick  = () => this.move(-1, 0);
    for (const btn of Object.values(navBtns)) btn.disabled = false;

    this.onKeyDown = (e: KeyboardEvent) => {
      const dirs: Record<string, [number, number]> = {
        ArrowUp: [0, -1], ArrowDown: [0, 1], ArrowRight: [1, 0], ArrowLeft: [-1, 0],
      };
      const d = dirs[e.key];
      if (d) { e.preventDefault(); this.move(d[0], d[1]); }
    };
    window.addEventListener('keydown', this.onKeyDown);

    this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
    this.canvas.addEventListener('mousedown', (e) => {
      e.preventDefault();
      if (e.button === 2) {
        const rect = this.canvas.getBoundingClientRect();
        const tx = Math.floor((e.clientX - rect.left) / 32);
        const ty = Math.floor((e.clientY - rect.top)  / 32);
        const dx = Math.sign(tx - this.x);
        const dy = Math.sign(ty - this.y);
        if (dx !== 0 || dy !== 0) this.move(dx, dy);
      }
    });

    if (this.network) this.wireNetwork(this.network);
  }

  private wireNetwork(net: GameNetwork): void {
    net.onPlayerInfo = async (msg) => {
      const sprite = await this.loadAvatarSprite(msg.avatar);
      this.otherPlayers.set(msg.id, {
        id: msg.id, name: msg.name, avatar: msg.avatar,
        room: msg.room, x: msg.x, y: msg.y, sprite,
      });
      await this.render();
    };

    net.onLocation = async (msg) => {
      if (msg.id === this.myId) return; // ignore echo of our own position
      const p = this.otherPlayers.get(msg.id);
      if (p) {
        p.room = msg.room;
        p.x = msg.x;
        p.y = msg.y;
        await this.render();
      }
    };

    net.onLeave = async (msg) => {
      this.otherPlayers.delete(msg.id);
      await this.render();
    };
  }

  destroy(): void {
    window.removeEventListener('keydown', this.onKeyDown);
    this.network?.sendLeave();
  }

  async setMode(mode: ColorMode): Promise<void> {
    setColorMode(mode);
    this.roomBg = null;
    await this.loadPlayerSprite();
    await this.render();
  }

  async loadPlayerSprite(): Promise<void> {
    this.playerSprite = await this.loadAvatarSprite(this.avatarName);
  }

  private async loadAvatarSprite(name: string): Promise<ImageData | null> {
    return loadMaskedSprite(
      `/sprites/facebits/${name}bit.png`,
      `/sprites/facebits/${name}mask.png`
    );
  }

  async setAvatar(name: string): Promise<void> {
    this.avatarName = name;
    await this.loadPlayerSprite();
    await this.render();
  }

  setMyId(id: number): void {
    this.myId = id;
  }

  async goToRoom(index: number, px = 10, py = 10): Promise<void> {
    if (index < 0 || index >= this.mapData.rooms.length) return;
    this.currentRoom = index;
    this.px = px;
    this.py = py;
    this.roomBg = null;
    this.exitMap = buildExitMap(this.mapData.rooms[index], this.objects);
    this.network?.sendLocation(this.currentRoom, this.px, this.py);
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

    // Exits take priority over collision
    const exit = this.exitMap.get(`${nx},${ny}`);
    if (exit) {
      await this.goToRoom(exit.destRoom, exit.landX, exit.landY);
      return;
    }

    // Block movement into walls
    if (isTileBlocked(nx, ny, room, this.objects)) return;

    // Block movement into a tile occupied by another player in this room
    for (const p of this.otherPlayers.values()) {
      if (p.room === this.currentRoom && p.x === nx && p.y === ny) return;
    }

    this.px = nx;
    this.py = ny;
    this.network?.sendLocation(this.currentRoom, this.px, this.py);

    await this.render();
  }

  private async render(): Promise<void> {
    const room = this.mapData.rooms[this.currentRoom];

    if (!this.roomBg) {
      this.status.textContent = 'Loading room…';
      await preloadRoomSprites(room, this.objects, this.objset);
      this.roomBg = await buildRoomBackground(room, this.objects, this.objset);
      this.status.textContent = '';
    }

    // Collect other players in the same room
    const others: OtherPlayer[] = [];
    for (const p of this.otherPlayers.values()) {
      if (p.room === this.currentRoom) {
        others.push({ px: p.x, py: p.y, sprite: p.sprite });
      }
    }

    await renderFrame(this.canvas, this.roomBg, this.playerSprite, this.px, this.py, others);
    this.roomInfo.textContent =
      `Room ${this.currentRoom}: "${room.name}" — (${this.px}, ${this.py})`;
  }
}
