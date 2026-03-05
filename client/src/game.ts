import { MapFile, ObjectFile, ObjDef, RoomData, InventoryItem } from './types';
import { loadMaskedSprite, loadSprite, setColorMode, ColorMode } from './assets';
import { preloadRoomSprites, buildRoomBackground, renderFrame, OtherPlayer } from './renderer';
import { GameNetwork } from './network';

const GRID = 20;

interface ExitTile { destRoom: number; landX: number; landY: number; }

/** Returns true if the tile at (x, y) cannot be entered. */
function isTileBlocked(x: number, y: number, room: RoomData, objects: ObjDef[]): boolean {
  const cell = room.spot?.[x]?.[y];
  if (!cell) return false;
  const [flId, wlId] = cell;
  if (wlId > 0 && !objects[wlId]?.permeable) return true;
  if (flId > 0 && !objects[flId]?.permeable) return true;
  return false;
}

/** Build a map from "x,y" → exit destination for all exit objects in the room. */
function buildExitMap(room: RoomData, objects: ObjDef[]): Map<string, ExitTile> {
  const map = new Map<string, ExitTile>();
  for (const ro of room.recorded_objects ?? []) {
    if (ro.detail < 0) continue;
    const obj = objects[ro.type];
    if (!obj?.exit) continue;
    const landX = ro.infox >= 0 ? ro.infox : ro.x;
    const landY = ro.infoy >= 0 ? ro.infoy : ro.y;
    map.set(`${ro.x},${ro.y}`, { destRoom: ro.detail, landX, landY });
  }
  return map;
}

interface MissileAnim {
  x: number;
  y: number;
  dx: number;
  dy: number;
  objType: number;
  timer: ReturnType<typeof setTimeout> | null;
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

  // floor items: per-room map of "x,y" → item
  private floorItems = new Map<number, Map<string, InventoryItem>>();

  // active missile animations
  private missiles = new Map<number, MissileAnim>();
  // cache: objType → ImageBitmap for missile sprites
  private missileSpriteCache = new Map<number, ImageBitmap | null>();

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

    // Initialize floor items from map data
    this.initFloorItems();

    navBtns['north'].onclick = () => this.move(0, -1);
    navBtns['south'].onclick = () => this.move(0,  1);
    navBtns['east'].onclick  = () => this.move( 1, 0);
    navBtns['west'].onclick  = () => this.move(-1, 0);
    for (const btn of Object.values(navBtns)) btn.disabled = false;

    this.onKeyDown = (e: KeyboardEvent) => {
      // Don't intercept keys while typing in an input
      const tag = (document.activeElement as HTMLElement)?.tagName;
      if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;

      const keyDirs: Record<string, [number, number]> = {
        // Arrow keys
        ArrowUp: [0, -1], ArrowDown: [0, 1], ArrowRight: [1, 0], ArrowLeft: [-1, 0],
        // QWERTY 8-directional (original layout)
        q: [-1, -1], w: [0, -1], e: [1, -1],
        a: [-1,  0],             d: [1,  0],
        z: [-1,  1], x: [0,  1], c: [1,  1],
      };
      const codeDirs: Record<string, [number, number]> = {
        // Numpad (use e.code to avoid NumLock interference)
        Numpad7: [-1, -1], Numpad8: [0, -1], Numpad9: [1, -1],
        Numpad4: [-1,  0],                   Numpad6: [1,  0],
        Numpad1: [-1,  1], Numpad2: [0,  1], Numpad3: [1,  1],
      };

      const d = keyDirs[e.key] ?? codeDirs[e.code];
      if (d) { e.preventDefault(); this.move(d[0], d[1]); return; }

      // Item actions
      if (e.key === 's') { e.preventDefault(); this.network?.sendPickup(this.px, this.py, 'left'); return; }
      if (e.key === 'S') { e.preventDefault(); this.network?.sendPickup(this.px, this.py, 'right'); return; }
      if (e.key === 'Z') { e.preventDefault(); this.network?.sendDrop('left'); return; }
      if (e.key === 'X') { e.preventDefault(); this.network?.sendDrop('right'); return; }
    };
    window.addEventListener('keydown', this.onKeyDown);

    this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
    this.canvas.addEventListener('mousedown', (e) => {
      e.preventDefault();
      const rect = this.canvas.getBoundingClientRect();
      const tx = Math.floor((e.clientX - rect.left) / 32);
      const ty = Math.floor((e.clientY - rect.top)  / 32);

      if (e.button === 0 || e.button === 1) {
        // Left-click → right hand, middle-click → left hand
        const hand: 'left' | 'right' = e.button === 1 ? 'left' : 'right';
        const key = `${tx},${ty}`;
        if (this.floorItems.get(this.currentRoom)?.has(key)) {
          // Pick up floor item
          this.network?.sendPickup(tx, ty, hand);
        } else if (tx !== this.px || ty !== this.py) {
          // Fire weapon toward clicked tile
          this.network?.sendFireWeapon(hand, tx, ty);
        }
        return;
      }

      // Right-click: move toward tile
      if (e.button === 2) {
        const dx = Math.sign(tx - this.px);
        const dy = Math.sign(ty - this.py);
        if (dx !== 0 || dy !== 0) void this.move(dx, dy);
      }
    });

    if (this.network) this.wireNetwork(this.network);
  }

  private initFloorItems(): void {
    for (let roomIdx = 0; roomIdx < this.mapData.rooms.length; roomIdx++) {
      const room = this.mapData.rooms[roomIdx];
      const itemMap = new Map<string, InventoryItem>();
      for (const ro of room.recorded_objects ?? []) {
        if (ro.type <= 0) continue;
        const obj = this.objects[ro.type];
        if (!obj?.takeable) continue;
        const quantity = ro.detail > 0 ? ro.detail : 1;
        itemMap.set(`${ro.x},${ro.y}`, { type: ro.type, quantity });
      }
      this.floorItems.set(roomIdx, itemMap);
    }
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

    net.onItemsSync = async (msg) => {
      // Replace local floor items entirely with server state
      this.floorItems.clear();
      for (const entry of msg.items) {
        let roomMap = this.floorItems.get(entry.room);
        if (!roomMap) {
          roomMap = new Map<string, InventoryItem>();
          this.floorItems.set(entry.room, roomMap);
        }
        roomMap.set(`${entry.x},${entry.y}`, entry.item);
      }
      this.roomBg = null; // force rebuild (background exclusion is correct already)
      await this.render();
    };

    net.onItemRemoved = async (msg) => {
      const roomMap = this.floorItems.get(msg.room);
      if (roomMap) {
        roomMap.delete(`${msg.x},${msg.y}`);
        if (msg.room === this.currentRoom) await this.render();
      }
    };

    net.onItemAdded = async (msg) => {
      let roomMap = this.floorItems.get(msg.room);
      if (!roomMap) {
        roomMap = new Map<string, InventoryItem>();
        this.floorItems.set(msg.room, roomMap);
      }
      roomMap.set(`${msg.x},${msg.y}`, msg.item);
      if (msg.room === this.currentRoom) await this.render();
    };

    net.onMissileStart = (msg) => {
      if (msg.room !== this.currentRoom || msg.path.length === 0) return;
      const anim: MissileAnim = {
        x: msg.path[0].x, y: msg.path[0].y,
        dx: msg.dx, dy: msg.dy,
        objType: msg.objType, timer: null,
      };
      this.missiles.set(msg.id, anim);
      void this.render();

      let step = 0;
      const advance = () => {
        const current = this.missiles.get(msg.id);
        if (!current) return; // cancelled by MISSILE_END
        step++;
        if (step < msg.path.length) {
          current.x = msg.path[step].x;
          current.y = msg.path[step].y;
          void this.render();
          current.timer = setTimeout(advance, msg.msPerStep);
        } else {
          // Animation finished — server will send MISSILE_END to finalize
          // but remove locally after one extra step so sprite doesn't linger
          current.timer = setTimeout(() => {
            this.missiles.delete(msg.id);
            void this.render();
          }, msg.msPerStep);
        }
      };
      anim.timer = setTimeout(advance, msg.msPerStep);
    };

    net.onMissileEnd = (msg) => {
      const anim = this.missiles.get(msg.id);
      if (anim?.timer) clearTimeout(anim.timer);
      this.missiles.delete(msg.id);
      void this.render();
    };

    net.onYouDied = async (msg) => {
      await this.goToRoom(msg.respawnRoom, msg.respawnX, msg.respawnY);
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

    const floorItems = this.floorItems.get(this.currentRoom) ?? new Map<string, InventoryItem>();

    await renderFrame(
      this.canvas, this.roomBg, this.playerSprite, this.px, this.py,
      others, floorItems, this.objects, this.objset
    );
    await this.drawMissiles();
    this.roomInfo.textContent =
      `Room ${this.currentRoom}: "${room.name}" — (${this.px}, ${this.py})`;
  }

  private async drawMissiles(): Promise<void> {
    if (this.missiles.size === 0) return;
    const TILE = 32;
    const ctx = this.canvas.getContext('2d')!;
    const base = `/data/objects/bitmaps/${this.objset}`;

    for (const anim of this.missiles.values()) {
      // Resolve sprite, using a local cache to avoid repeated createImageBitmap calls
      let bm: ImageBitmap | null;
      if (this.missileSpriteCache.has(anim.objType)) {
        bm = this.missileSpriteCache.get(anim.objType)!;
      } else {
        const obj = this.objects[anim.objType];
        let imgData: ImageData | null = null;
        if (obj?.bitmap) {
          if (obj.masked && obj.mask) {
            imgData = await loadMaskedSprite(`${base}/${obj.bitmap}`, `${base}/${obj.mask}`);
          } else {
            imgData = await loadSprite(`${base}/${obj.bitmap}`);
          }
        }
        bm = imgData ? await createImageBitmap(imgData) : null;
        this.missileSpriteCache.set(anim.objType, bm);
      }

      const obj = this.objects[anim.objType];
      const cx = anim.x * TILE + TILE / 2;
      const cy = anim.y * TILE + TILE / 2;
      if (bm) {
        if (obj?.directional) {
          // Base bitmap faces UP. Rotate clockwise by atan2(dx, -dy).
          const angle = Math.atan2(anim.dx, -anim.dy);
          ctx.save();
          ctx.translate(cx, cy);
          ctx.rotate(angle);
          ctx.drawImage(bm, -TILE / 2, -TILE / 2, TILE, TILE);
          ctx.restore();
        } else {
          ctx.drawImage(bm, anim.x * TILE, anim.y * TILE, TILE, TILE);
        }
      } else {
        // Fallback: yellow square
        ctx.fillStyle = '#ffff00';
        ctx.fillRect(anim.x * TILE + 12, anim.y * TILE + 12, 8, 8);
      }
    }
  }
}
