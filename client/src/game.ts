import { MapFile, ObjectFile, ObjDef, RoomData, InventoryItem } from './types';
import { loadMaskedSprite, loadSprite, setColorMode, getColorMode, ColorMode } from './assets';
import {
  preloadRoomSprites,
  buildRoomBackground,
  renderFrame,
  getBitmap,
  OtherPlayer,
  TILE,
  BORDER,
} from './renderer';
import { GameNetwork } from './network';
import { showTooltip, hideTooltip, moveTooltip } from './tooltip';
import { stepDelay, applyHpPenalty } from './utils';
import { isTileBlocked, computeBfsPathToNearest, buildExitMap, ExitTile } from './game-utils';
import { tileIsVisible, tileViewBlocked } from './los';

const GRID = 20;
const TOMBSTONE_BIT = '/sprites/bitmaps/tombbit.png';
const TOMBSTONE_MASK = '/sprites/bitmaps/tombmask.png';

interface MissileAnim {
  x: number;
  y: number;
  dx: number;
  dy: number;
  objType: number;
  timer: ReturnType<typeof setTimeout> | null;
}

interface HitMarker {
  x: number;
  y: number;
  damage: number;
  startTime: number;
  isHeal: boolean;
}

const HIT_MARKER_DURATION = 600; // ms

interface RemotePlayer {
  id: number;
  name: string;
  avatar: string;
  room: number;
  x: number;
  y: number;
  sprite: ImageData | null;
  dead: boolean;
  team: number;
}

interface RemoteMonster {
  id: number;
  name: string;
  avatar: string;
  room: number;
  x: number;
  y: number;
  sprite: ImageData | null;
  dead: boolean;
  team: number;
  monsterId: string;
}

export class Game {
  private mapData: MapFile;
  private objects: ObjDef[];
  private objset: string;
  private currentRoom: number = 0;
  private px: number = 10;
  private py: number = 10;
  private exitMap: Map<string, ExitTile> = new Map();
  private exitKeys: Set<string> = new Set();
  private roomBg: OffscreenCanvas | null = null;
  private playerSprite: ImageData | null = null;
  private tombstoneSprite: ImageData | null = null;
  private isDead = false;
  private avatarName: string = 'crom';
  private canvas: HTMLCanvasElement;
  private roomInfo: HTMLElement;
  private status: HTMLElement;
  private onKeyDown!: (e: KeyboardEvent) => void;

  private network: GameNetwork | null = null;
  private myId: number | null = null;
  private otherPlayers = new Map<number, RemotePlayer>();
  private monsters = new Map<number, RemoteMonster>();

  // floor items: per-room map of "x,y" → item
  private floorItems = new Map<number, Map<string, InventoryItem>>();

  // active missile animations
  private missiles = new Map<number, MissileAnim>();

  // floating damage number markers
  private hitMarkers: HitMarker[] = [];
  // punch hit bitmap overlays
  private punchMarkers: Array<{ x: number; y: number; dx: number; dy: number; until: number }> = [];
  // screen flash end time (ms) when local player takes damage
  private screenFlashUntil = 0;

  // click-to-move path: walk toward this tile step by step
  private moveTarget: { x: number; y: number } | null = null;
  private movePath: Array<{ x: number; y: number }> = [];
  private moveTimer: ReturnType<typeof setTimeout> | null = null;
  // rate-limit: keyboard cannot move faster than click-to-move
  private moveReadyAt = 0;

  // hand items (kept in sync via setHands, used for click routing)
  private leftHand: InventoryItem | null = null;
  // full inventory for light radius calculation in dark rooms
  private inventory: Array<InventoryItem | null> = [];

  // local player HP for speed penalty
  private myHp = 100;
  private myMaxHp = 100;

  private myTeam = 0;
  private boxOtherPlayers = true;

  // tile hover debug mode (toggled by ?)
  private hoverMode = false;

  // pickup highlight overlay (toggled by p, on by default)
  private showPickupHighlights = true;
  private currentWeight = 0;
  private maxWeight = 150;
  private invFull = false;

  // fog-of-war visibility overlay (toggled by v, on by default)
  private fogEnabled = true;
  // per-tile current fog alpha (0 = fully lit, 0.2 = fully dimmed); starts fully fogged
  private fogAlpha: number[][] = Array.from({ length: GRID }, () => new Array(GRID).fill(0.2));
  private fogAnimFrame: number | null = null;

  private destroyed = false;

  // Render concurrency guard: at most one render in flight; queue one more if needed
  private renderInProgress = false;
  private renderQueued = false;
  private visibilityGrid: boolean[][] = [];

  constructor(
    mapData: MapFile,
    objFile: ObjectFile,
    canvas: HTMLCanvasElement,
    roomInfo: HTMLElement,
    status: HTMLElement,
    navBtns: Record<string, HTMLButtonElement>,
    network?: GameNetwork,
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
    navBtns['south'].onclick = () => this.move(0, 1);
    navBtns['east'].onclick = () => this.move(1, 0);
    navBtns['west'].onclick = () => this.move(-1, 0);
    for (const btn of Object.values(navBtns)) btn.disabled = false;

    this.onKeyDown = (e: KeyboardEvent) => {
      // Don't intercept keys while typing in an input
      const tag = (document.activeElement as HTMLElement)?.tagName;
      if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;

      const keyDirs: Record<string, [number, number]> = {
        // Arrow keys
        ArrowUp: [0, -1],
        ArrowDown: [0, 1],
        ArrowRight: [1, 0],
        ArrowLeft: [-1, 0],
        // QWERTY 8-directional (original layout)
        q: [-1, -1],
        w: [0, -1],
        e: [1, -1],
        a: [-1, 0],
        s: [0, 1],
        d: [1, 0],
        z: [-1, 1],
        x: [0, 1],
        c: [1, 1],
      };
      const codeDirs: Record<string, [number, number]> = {
        // Numpad (use e.code to avoid NumLock interference)
        Numpad7: [-1, -1],
        Numpad8: [0, -1],
        Numpad9: [1, -1],
        Numpad4: [-1, 0],
        Numpad6: [1, 0],
        Numpad1: [-1, 1],
        Numpad2: [0, 1],
        Numpad3: [1, 1],
      };

      const d = keyDirs[e.key] ?? codeDirs[e.code];
      if (d) {
        e.preventDefault();
        this.stopMoving();
        if (Date.now() < this.moveReadyAt) return;
        this.moveReadyAt = Infinity; // lock until move resolves
        void this.move(d[0], d[1]).then(() => {
          // Sample delay from the tile we just arrived on (same as click-to-move)
          this.moveReadyAt = Date.now() + this.getMoveDelay();
        });
        return;
      }

      // Toggle fog-of-war visibility overlay
      if (e.key === 'v') {
        e.preventDefault();
        this.fogEnabled = !this.fogEnabled;
        if (this.fogEnabled) this.computeVisibility();
        else void this.render();
        return;
      }

      // Toggle pickup highlight overlay
      if (e.key === 'p') {
        e.preventDefault();
        this.showPickupHighlights = !this.showPickupHighlights;
        void this.render();
        return;
      }

      // Toggle player indicator boxes
      if (e.key === 'o') {
        e.preventDefault();
        this.boxOtherPlayers = !this.boxOtherPlayers;
        void this.render();
        return;
      }

      // Toggle tile hover debug mode
      if (e.key === 'i') {
        e.preventDefault();
        this.hoverMode = !this.hoverMode;
        this.canvas.style.cursor = this.hoverMode ? 'crosshair' : '';
        this.status.textContent = this.hoverMode ? '[Hover mode ON — move mouse over tiles]' : '';
        if (!this.hoverMode) hideTooltip();
        return;
      }

      // Item actions
      if (e.key === 'f') {
        e.preventDefault();
        if (!this.isDead) this.network?.sendPickup(this.px, this.py);
        return;
      }
      if (e.key === 'g') {
        e.preventDefault();
        if (!this.isDead) this.network?.sendDrop('active');
        return;
      }
    };
    window.addEventListener('keydown', this.onKeyDown);

    this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
    this.canvas.addEventListener('mousedown', (e) => {
      e.preventDefault();
      const rect = this.canvas.getBoundingClientRect();
      // Subtract BORDER so tile (0,0) starts at the inner map area
      const tx = Math.floor((e.clientX - rect.left) / TILE) - 1;
      const ty = Math.floor((e.clientY - rect.top) / TILE) - 1;

      // Border click: right-click walks toward exit; left fires into next room
      if (tx < 0 || tx >= GRID || ty < 0 || ty >= GRID) {
        if (this.isDead) return;
        if (e.button === 2) {
          this.startMovingTo(tx, ty);
        } else if (e.button === 0) {
          this.network?.sendFireWeapon(tx, ty);
        }
        return;
      }

      if (e.button === 0) {
        // Left-click: pick up, use opener, or fire weapon
        if (this.isDead) return;
        const handObj = this.leftHand ? this.objects[this.leftHand.type] : null;
        const key = `${tx},${ty}`;
        const tileOccupied = [...this.otherPlayers.values()].some(
          (p) => p.room === this.currentRoom && p.x === tx && p.y === ty,
        );
        const room = this.mapData.rooms[this.currentRoom];
        const dist = Math.max(Math.abs(tx - this.px), Math.abs(ty - this.py));
        const hasDoorAtTile =
          dist === 1 &&
          room?.recorded_objects?.some(
            (ro) => ro.x === tx && ro.y === ty && (this.objects[ro.type]?.swings ?? false),
          );
        if (handObj?.opens && hasDoorAtTile) {
          // Key in hand, adjacent door — use key even if a floor item is also on this tile
          this.network?.sendUseItem(tx, ty);
        } else if (!tileOccupied && this.floorItems.get(this.currentRoom)?.has(key)) {
          this.network?.sendPickup(tx, ty);
        } else if ((handObj?.health ?? 0) < 0) {
          // Consumable: use on self regardless of where the player clicked
          this.network?.sendUseItem(this.px, this.py);
        } else if (handObj?.opens && dist === 1) {
          // Holding an opener adjacent to target tile — use it (open/close door)
          this.network?.sendUseItem(tx, ty);
        } else if (tx !== this.px || ty !== this.py) {
          this.network?.sendFireWeapon(tx, ty);
        }
        return;
      }

      // Right-click: walk toward clicked tile
      if (e.button === 2) {
        if (this.isDead) return;
        this.startMovingTo(tx, ty);
      }
    });

    this.canvas.addEventListener('mousemove', (e) => {
      if (!this.hoverMode) {
        moveTooltip(e.clientX, e.clientY);
        return;
      }
      const rect = this.canvas.getBoundingClientRect();
      const tx = Math.floor((e.clientX - rect.left) / TILE) - 1;
      const ty = Math.floor((e.clientY - rect.top) / TILE) - 1;
      if (tx < 0 || tx >= GRID || ty < 0 || ty >= GRID) {
        hideTooltip();
        return;
      }
      showTooltip(this.buildTileHtml(tx, ty), e.clientX, e.clientY);
    });
    this.canvas.addEventListener('mouseleave', () => {
      if (this.hoverMode) hideTooltip();
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
      if (msg.id === this.myId) return; // don't store self in otherPlayers
      const sprite = await this.loadAvatarSprite(msg.avatar);
      this.otherPlayers.set(msg.id, {
        id: msg.id,
        name: msg.name,
        avatar: msg.avatar,
        room: msg.room,
        x: msg.x,
        y: msg.y,
        sprite,
        dead: msg.dead,
        team: msg.team,
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
        x: msg.path[0].x,
        y: msg.path[0].y,
        dx: msg.dx,
        dy: msg.dy,
        objType: msg.objType,
        timer: null,
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

    net.onPlayerHit = (msg) => {
      if (msg.room !== this.currentRoom) return;
      this.addHitMarker(msg.x, msg.y, msg.damage, false);
      if (msg.victimId === this.myId) {
        this.screenFlashUntil = Date.now() + 200;
      }
    };

    net.onPlayerHeal = (msg) => {
      if (msg.room !== this.currentRoom) return;
      this.addHitMarker(msg.x, msg.y, msg.amount, true);
    };

    net.onRoomObjectChanged = async (msg) => {
      const room = this.mapData.rooms[msg.room];
      if (!room) return;

      if (msg.layer && room.spot) {
        // Spot-layer change (destruction / flammable chain reaction)
        const layerIdx = msg.layer === 'floor' ? 0 : 1;
        if (room.spot[msg.x]?.[msg.y]) {
          room.spot[msg.x][msg.y][layerIdx] = msg.newType;
        }
      } else {
        // Existing behavior: recorded_objects change (door toggles, destruction)
        for (const ro of room.recorded_objects ?? []) {
          if (ro.x === msg.x && ro.y === msg.y) {
            ro.type = msg.newType;
            break;
          }
        }
      }

      if (msg.room === this.currentRoom) {
        this.roomBg = null; // rebuild background
        this.computeVisibility();
        await this.render();
      }
    };

    net.onPlayerHidden = async (msg) => {
      this.otherPlayers.delete(msg.id);
      await this.render();
    };

    net.onPunch = (msg) => {
      if (msg.room !== this.currentRoom) return;
      this.punchMarkers.push({
        x: msg.x,
        y: msg.y,
        dx: msg.dx,
        dy: msg.dy,
        until: Date.now() + 350,
      });
      void this.render();
      setTimeout(() => void this.render(), 360);
    };

    // ── Monster handlers ──────────────────────────────────────────────────

    net.onMonsterInfo = async (msg) => {
      const sprite = await this.loadAvatarSprite(msg.avatar);
      this.monsters.set(msg.id, {
        id: msg.id,
        name: msg.name,
        avatar: msg.avatar,
        room: msg.room,
        x: msg.x,
        y: msg.y,
        sprite,
        dead: msg.dead,
        team: msg.team,
        monsterId: msg.monsterId,
      });
      await this.render();
    };

    net.onMonsterLocation = async (msg) => {
      const m = this.monsters.get(msg.id);
      if (m) {
        m.room = msg.room;
        m.x = msg.x;
        m.y = msg.y;
        await this.render();
      }
    };

    net.onMonsterHidden = async (msg) => {
      this.monsters.delete(msg.id);
      await this.render();
    };
  }

  /** Sync active hand for click routing (opens, health, weapon checks). */
  setActiveHand(item: InventoryItem | null): void {
    this.leftHand = item;
  }

  /** Sync full inventory for dark room light radius calculation. */
  setInventory(inv: Array<InventoryItem | null>): void {
    if (!this.isRoomDark()) {
      this.inventory = inv;
      return;
    }
    const prev = this.getEffectiveLightRadius();
    this.inventory = inv;
    if (this.getEffectiveLightRadius() !== prev) this.computeVisibility();
  }

  /** Update local player HP for movement speed penalty calculation. */
  setMyHp(hp: number, maxHp: number): void {
    this.myHp = hp;
    this.myMaxHp = maxHp;
  }

  /** Update carry weight and inventory fullness so the renderer can tint unpickable items. */
  setWeight(current: number, max: number, inventoryFull: boolean): void {
    this.currentWeight = current;
    this.maxWeight = max;
    this.invFull = inventoryFull;
    void this.render();
  }

  private addHitMarker(x: number, y: number, damage: number, isHeal: boolean): void {
    this.hitMarkers.push({ x, y, damage, startTime: Date.now(), isHeal });
    setTimeout(() => {
      const now = Date.now();
      this.hitMarkers = this.hitMarkers.filter((m) => now - m.startTime < HIT_MARKER_DURATION);
      void this.render();
    }, HIT_MARKER_DURATION);
    void this.render();
  }

  destroy(): void {
    this.destroyed = true;
    if (this.fogAnimFrame !== null) cancelAnimationFrame(this.fogAnimFrame);
    window.removeEventListener('keydown', this.onKeyDown);
    this.stopMoving();
    for (const anim of this.missiles.values()) {
      if (anim.timer) clearTimeout(anim.timer);
    }
    this.missiles.clear();
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
      `/sprites/facebits/${name}mask.png`,
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

  setMyTeam(team: number): void {
    this.myTeam = team;
  }

  public notifyDied(): void {
    this.isDead = true;
    this.stopMoving();
    void this.render();
  }

  public async notifyRespawned(room: number, x: number, y: number): Promise<void> {
    this.isDead = false;
    await this.goToRoom(room, x, y);
  }

  async goToRoom(index: number, px = 10, py = 10): Promise<void> {
    if (index < 0 || index >= this.mapData.rooms.length) return;
    this.stopMoving();
    const prevRoom = this.currentRoom;
    this.currentRoom = index;
    // Clear players from the old room — the server will re-announce anyone
    // visible via PLAYER_INFO after it processes our location update.
    if (prevRoom !== index) {
      for (const [id, p] of this.otherPlayers) {
        if (p.room === prevRoom) this.otherPlayers.delete(id);
      }
    }
    this.px = px;
    this.py = py;
    // Reset fog: fully black for dark rooms, light dim for lit rooms
    const fogInit = this.isRoomDark() ? 1.0 : 0.2;
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        this.fogAlpha[x][y] = fogInit;
      }
    }
    this.computeVisibility();
    this.roomBg = null;
    this.exitMap = buildExitMap(this.mapData.rooms[index], this.objects);
    this.exitKeys = new Set(this.exitMap.keys());
    this.tombstoneSprite = await loadMaskedSprite(TOMBSTONE_BIT, TOMBSTONE_MASK);
    this.network?.sendLocation(this.currentRoom, this.px, this.py);
    await this.render();
  }

  private async move(dx: number, dy: number): Promise<void> {
    if (this.isDead) return;
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
    this.computeVisibility();
    this.network?.sendLocation(this.currentRoom, this.px, this.py);

    await this.render();
  }

  // ── Click-to-move ─────────────────────────────────────────────────────────

  private startMovingTo(x: number, y: number): void {
    this.stopMoving();
    this.moveTarget = { x, y };
    const room = this.mapData.rooms[this.currentRoom];
    if (x >= 0 && x < GRID && y >= 0 && y < GRID) {
      this.movePath = computeBfsPathToNearest(
        this.px,
        this.py,
        x,
        y,
        room,
        this.objects,
        this.exitKeys,
      );
    } else {
      // Off-grid border exit: BFS to nearest edge tile, then step off
      const ex = Math.max(0, Math.min(GRID - 1, x));
      const ey = Math.max(0, Math.min(GRID - 1, y));
      this.movePath =
        this.px === ex && this.py === ey
          ? []
          : computeBfsPathToNearest(this.px, this.py, ex, ey, room, this.objects, this.exitKeys);
    }
    this.scheduleMoveStep();
  }

  private stopMoving(): void {
    if (this.moveTimer !== null) {
      clearTimeout(this.moveTimer);
      this.moveTimer = null;
    }
    this.moveTarget = null;
    this.movePath = [];
  }

  private scheduleMoveStep(): void {
    const delay = this.getMoveDelay();
    this.moveTimer = setTimeout(() => {
      this.moveTimer = null;
      void this.doMoveStep();
    }, delay);
  }

  private async doMoveStep(): Promise<void> {
    if (this.isDead) {
      this.stopMoving();
      return;
    }
    if (!this.moveTarget) return;
    const target = this.moveTarget;

    let dx: number, dy: number;
    const offGrid = target.x < 0 || target.x >= GRID || target.y < 0 || target.y >= GRID;

    if (offGrid) {
      if (this.movePath.length > 0) {
        // Still navigating to the edge tile
        const next = this.movePath[0];
        dx = next.x - this.px;
        dy = next.y - this.py;
      } else {
        // At the edge tile — step off the grid to trigger room transition
        dx = Math.sign(target.x - this.px);
        dy = Math.sign(target.y - this.py);
      }
    } else {
      // In-room: follow pre-computed BFS path
      if (this.movePath.length === 0) {
        this.stopMoving();
        return;
      }
      const next = this.movePath[0];
      dx = next.x - this.px;
      dy = next.y - this.py;
    }

    if (dx === 0 && dy === 0) {
      this.stopMoving();
      return;
    }

    const prevRoom = this.currentRoom;
    const prevX = this.px;
    const prevY = this.py;

    await this.move(dx, dy);

    // Room changed: arrived in new room, stop
    if (this.currentRoom !== prevRoom) {
      this.stopMoving();
      return;
    }

    // Blocked by a dynamic obstacle (e.g. another player on that tile): stop
    if (this.px === prevX && this.py === prevY) {
      this.stopMoving();
      return;
    }

    // Consume the step we just completed
    if (
      this.movePath.length > 0 &&
      this.movePath[0].x === this.px &&
      this.movePath[0].y === this.py
    ) {
      this.movePath.shift();
    }

    // Still have a target — keep going
    if (this.moveTarget) this.scheduleMoveStep();
  }

  /** Delay in ms for one step. movement field is a speed 1-9 (9=fastest).
   *  Absent movement means blocked (0); open tiles with no object default to 9.
   *  When hurt, movement slows proportionally (at 50% HP → ~1.41× delay). */
  private getMoveDelay(): number {
    const room = this.mapData.rooms[this.currentRoom];
    const flId = room.spot?.[this.px]?.[this.py]?.[0] ?? 0;
    const spd = flId > 0 ? (this.objects[flId]?.movement ?? 0) : 9;
    const base = stepDelay(spd);
    return applyHpPenalty(base, this.myHp, this.myMaxHp);
  }

  // ── Tile hover debug ──────────────────────────────────────────────────────

  private buildTileHtml(tx: number, ty: number): string {
    const room = this.mapData.rooms[this.currentRoom];
    const rows: string[] = [
      `<div class="tip-name">Room ${this.currentRoom}</div>`,
      `<div class="tip-name">Tile (${tx}, ${ty})</div>`,
    ];

    const [flId, wlId] = room.spot?.[tx]?.[ty] ?? [0, 0];
    const flObj = flId > 0 ? this.objects[flId] : null;
    const wlObj = wlId > 0 ? this.objects[wlId] : null;

    if (flObj) {
      const blocked = flObj.permeable === false ? ' <span class="tip-lbl">[blocks]</span>' : '';
      rows.push(
        `<div class="tip-row"><span class="tip-lbl">floor</span> ${flObj.name ?? `#${flId}`}${blocked}</div>`,
      );
      const spd = flObj.movement ?? 0;
      const delay = spd > 0 ? stepDelay(spd) : null;
      const speedLabel = delay === null ? 'blocked' : `${delay} ms/step`;
      rows.push(`<div class="tip-row"><span class="tip-lbl">speed</span> ${speedLabel}</div>`);
    }
    if (wlObj) {
      const passable =
        (wlObj.movement ?? 0) > 0
          ? ' <span class="tip-lbl">[passable]</span>'
          : ' <span class="tip-lbl">[blocks]</span>';
      rows.push(
        `<div class="tip-row"><span class="tip-lbl">wall&nbsp;</span> ${wlObj.name ?? `#${wlId}`}${passable}</div>`,
      );
    }

    const recHere = (room.recorded_objects ?? []).filter(
      (ro) => ro.x === tx && ro.y === ty && ro.type > 0,
    );
    for (const ro of recHere) {
      const obj = this.objects[ro.type];
      const nm = obj?.name ?? `#${ro.type}`;
      const tag = obj?.takeable ? ' <span class="tip-lbl">[spawn]</span>' : '';
      rows.push(`<div class="tip-row">• ${nm}${tag}</div>`);
    }

    const droppedItem = this.floorItems.get(this.currentRoom)?.get(`${tx},${ty}`);
    if (droppedItem) {
      const obj = this.objects[droppedItem.type];
      const nm = obj?.name ?? `#${droppedItem.type}`;
      rows.push(`<div class="tip-row">• ${nm} <span class="tip-lbl">[dropped]</span></div>`);
    }

    if (!flObj && !wlObj && recHere.length === 0 && !droppedItem) {
      rows.push(`<div class="tip-row"><span class="tip-lbl">floor</span> empty</div>`);
      rows.push(
        `<div class="tip-row"><span class="tip-lbl">speed</span> ${stepDelay(9)} ms/step</div>`,
      );
    }
    return rows.join('');
  }

  // ── Dark room helpers ────────────────────────────────────────────────────

  /** Is the current room dark? */
  private isRoomDark(): boolean {
    const room = this.mapData.rooms[this.currentRoom];
    return (room?.dark ?? 0) === 1;
  }

  /** Effective light radius from best flashlight in hand + inventory. */
  private getEffectiveLightRadius(): number {
    const BASE_DARK_RADIUS = 2;
    if (!this.isRoomDark()) return Infinity;
    let best = 0;
    // Check hand
    if (this.leftHand) {
      const fl = this.objects[this.leftHand.type]?.flashlight ?? 0;
      if (fl > best) best = fl;
    }
    // Check all inventory slots
    for (const item of this.inventory) {
      if (!item) continue;
      const fl = this.objects[item.type]?.flashlight ?? 0;
      if (fl > best) best = fl;
    }
    return best > 0 ? best : BASE_DARK_RADIUS;
  }

  // ── Rendering ──────────────────────────────────────────────────────────────

  private computeVisibility(): void {
    if (!this.fogEnabled) return;
    const room = this.mapData.rooms[this.currentRoom];
    if (!room) {
      this.visibilityGrid = [];
      return;
    }
    const isDark = this.isRoomDark();
    const lightRadius = isDark ? this.getEffectiveLightRadius() : Infinity;

    // Pre-build glow lookup set for dark rooms (avoids O(recorded_objects) per tile)
    let glowSet: Set<string> | null = null;
    if (isDark) {
      glowSet = new Set<string>();
      // Check spot array for glowing floor/wall tiles
      if (room.spot) {
        for (let x = 0; x < GRID; x++) {
          for (let y = 0; y < GRID; y++) {
            const cell = room.spot[x]?.[y];
            if (!cell) continue;
            if (
              (cell[0] > 0 && this.objects[cell[0]]?.glows) ||
              (cell[1] > 0 && this.objects[cell[1]]?.glows)
            ) {
              glowSet.add(`${x},${y}`);
            }
          }
        }
      }
      for (const ro of room.recorded_objects ?? []) {
        if (this.objects[ro.type]?.glows) glowSet.add(`${ro.x},${ro.y}`);
      }
      const items = this.floorItems.get(this.currentRoom);
      if (items) {
        for (const [key, item] of items) {
          if (this.objects[item.type]?.glows) glowSet.add(key);
        }
      }
    }

    const grid: boolean[][] = [];
    for (let x = 0; x < GRID; x++) {
      grid[x] = [];
      for (let y = 0; y < GRID; y++) {
        const hasLos = tileIsVisible(room, this.objects, this.px, this.py, x, y);
        if (!hasLos) {
          grid[x][y] = false;
        } else if (!isDark) {
          grid[x][y] = true;
        } else {
          // Dark room: visible if within light radius or tile glows
          const dist = Math.max(Math.abs(x - this.px), Math.abs(y - this.py));
          grid[x][y] = dist <= lightRadius || glowSet!.has(`${x},${y}`);
        }
      }
    }
    // Second pass: reveal blocking tiles (walls) adjacent to visible open tiles.
    // A wall face is visible if you can see the floor beside it, even when the
    // diagonal path to the corner is blocked by an adjacent wall.
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        if (grid[x][y] || !isTileBlocked(x, y, room, this.objects)) continue;
        for (const [dx, dy] of [
          [0, 1],
          [0, -1],
          [1, 0],
          [-1, 0],
          [1, 1],
          [1, -1],
          [-1, 1],
          [-1, -1],
        ]) {
          const nx = x + dx;
          const ny = y + dy;
          if (nx < 0 || nx >= GRID || ny < 0 || ny >= GRID) continue;
          if (grid[nx][ny] && !tileViewBlocked(room, this.objects, nx, ny)) {
            grid[x][y] = true;
            break;
          }
        }
      }
    }

    this.visibilityGrid = grid;
    if (this.fogEnabled) this.startFogAnimation();
  }

  private startFogAnimation(): void {
    if (this.fogAnimFrame !== null) return;
    this.fogAnimFrame = requestAnimationFrame(() => this.tickFogAnimation());
  }

  private tickFogAnimation(): void {
    this.fogAnimFrame = null;
    if (!this.fogEnabled) return;
    const isDark = this.isRoomDark();
    const fogMax = isDark ? 1.0 : 0.2; // full black in dark rooms, dim in lit rooms
    // ~300ms transition: fogMax target alpha / 18 frames at 60fps
    const STEP = fogMax / 18;
    let anyChanging = false;
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        const target = this.visibilityGrid[x]?.[y] ? 0 : fogMax;
        const current = this.fogAlpha[x][y];
        if (Math.abs(current - target) <= STEP) {
          this.fogAlpha[x][y] = target;
        } else {
          this.fogAlpha[x][y] += current < target ? STEP : -STEP;
          anyChanging = true;
        }
      }
    }
    void this.render();
    if (anyChanging && !this.destroyed) {
      this.fogAnimFrame = requestAnimationFrame(() => this.tickFogAnimation());
    }
  }

  private drawFogOverlay(): void {
    const ctx = this.canvas.getContext('2d')!;
    ctx.save();
    ctx.fillStyle = '#000';
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        const alpha = this.fogAlpha[x][y];
        if (alpha > 0) {
          ctx.globalAlpha = alpha;
          ctx.fillRect(BORDER + x * TILE, BORDER + y * TILE, TILE, TILE);
        }
      }
    }
    ctx.restore();
  }

  private async render(): Promise<void> {
    if (this.destroyed) return;
    if (this.renderInProgress) {
      this.renderQueued = true;
      return;
    }
    this.renderInProgress = true;
    try {
      do {
        this.renderQueued = false;
        await this.doRender();
      } while (this.renderQueued && !this.destroyed);
    } finally {
      this.renderInProgress = false;
    }
  }

  private async doRender(): Promise<void> {
    if (this.destroyed) return;
    const room = this.mapData.rooms[this.currentRoom];

    if (!this.roomBg) {
      this.status.textContent = 'Loading room…';
      await preloadRoomSprites(room, this.objects, this.objset);
      this.roomBg = await buildRoomBackground(room, this.objects, this.objset);
      this.status.textContent = '';
    }

    // Collect other players and monsters in the same room
    const others: OtherPlayer[] = [];
    for (const p of this.otherPlayers.values()) {
      if (p.room === this.currentRoom) {
        others.push({ px: p.x, py: p.y, sprite: p.sprite, dead: p.dead, team: p.team });
      }
    }
    for (const m of this.monsters.values()) {
      if (m.room === this.currentRoom) {
        others.push({
          px: m.x,
          py: m.y,
          sprite: m.sprite,
          dead: m.dead,
          team: m.team,
          isMonster: true,
        });
      }
    }

    const floorItems = this.floorItems.get(this.currentRoom) ?? new Map<string, InventoryItem>();

    await renderFrame(
      this.canvas,
      this.roomBg,
      this.isDead ? this.tombstoneSprite : this.playerSprite,
      this.px,
      this.py,
      others,
      floorItems,
      this.objects,
      this.objset,
      this.tombstoneSprite,
      this.myTeam,
      this.boxOtherPlayers,
      this.mapData.map.teams_supported > 1,
      this.isDead,
      room,
      this.showPickupHighlights,
      this.currentWeight,
      this.maxWeight,
      this.invFull,
    );
    if (this.fogEnabled) this.drawFogOverlay();
    this.drawBorderIndicators(room);
    await this.drawMissiles();
    this.drawHitMarkers();
    await this.drawPunchMarkers();
    this.drawScreenFlash();
    this.roomInfo.textContent = room.name && room.name !== 'no name' ? room.name : '';
  }

  private drawBorderIndicators(room: RoomData): void {
    const ctx = this.canvas.getContext('2d')!;
    ctx.fillStyle = getColorMode() === 'dark' ? '#666' : '#888';

    const mapPx = GRID * TILE; // 640 — width/height of inner map in pixels
    const halfBorder = BORDER / 2;
    const mapMid = BORDER + mapPx / 2; // center of map horizontally and vertically

    const drawArrow = (cx: number, cy: number, dx: number, dy: number): void => {
      const S = 9;
      const px = -dy,
        py = dx; // perpendicular
      ctx.beginPath();
      ctx.moveTo(cx + dx * S, cy + dy * S);
      ctx.lineTo(cx + px * S * 0.7 - dx * S * 0.7, cy + py * S * 0.7 - dy * S * 0.7);
      ctx.lineTo(cx - px * S * 0.7 - dx * S * 0.7, cy - py * S * 0.7 - dy * S * 0.7);
      ctx.closePath();
      ctx.fill();
    };

    if (room.exit_north >= 0) drawArrow(mapMid, halfBorder, 0, -1);
    if (room.exit_south >= 0) drawArrow(mapMid, BORDER + mapPx + halfBorder, 0, 1);
    if (room.exit_west >= 0) drawArrow(halfBorder, mapMid, -1, 0);
    if (room.exit_east >= 0) drawArrow(BORDER + mapPx + halfBorder, mapMid, 1, 0);
  }

  private drawHitMarkers(): void {
    if (this.hitMarkers.length === 0) return;
    const ctx = this.canvas.getContext('2d')!;
    const now = Date.now();
    ctx.save();
    ctx.font = 'bold 13px monospace';
    ctx.textAlign = 'center';
    for (const m of this.hitMarkers) {
      const age = now - m.startTime;
      if (age >= HIT_MARKER_DURATION) continue;
      const t = age / HIT_MARKER_DURATION; // 0→1
      const alpha = 1 - t;
      const rise = t * 16; // float upward by up to 16px
      const cx = BORDER + m.x * TILE + TILE / 2;
      const cy = BORDER + m.y * TILE - rise;
      ctx.globalAlpha = alpha;
      ctx.fillStyle = m.isHeal ? '#44ff44' : '#ff4444';
      const label = m.isHeal ? `+${m.damage}` : `-${m.damage}`;
      ctx.fillText(label, cx, cy);
    }
    ctx.globalAlpha = 1;
    ctx.restore();
  }

  private async drawPunchMarkers(): Promise<void> {
    const now = Date.now();
    this.punchMarkers = this.punchMarkers.filter((m) => now < m.until);
    if (this.punchMarkers.length === 0) return;

    const sprite = await loadMaskedSprite(
      '/data/objects/bitmaps/standard/001_bitmap.png',
      '/data/objects/bitmaps/standard/001_mask.png',
    );
    if (!sprite) return;

    const bm = await getBitmap(sprite);
    const ctx = this.canvas.getContext('2d')!;
    for (const m of this.punchMarkers) {
      const cx = BORDER + m.x * TILE + TILE / 2;
      const cy = BORDER + m.y * TILE + TILE / 2;
      const angle = Math.atan2(m.dy, m.dx) + Math.PI / 2;
      ctx.save();
      ctx.translate(cx, cy);
      ctx.rotate(angle);
      ctx.drawImage(bm, -TILE / 2, -TILE / 2);
      ctx.restore();
    }
  }

  private drawScreenFlash(): void {
    const now = Date.now();
    if (now >= this.screenFlashUntil) return;
    const ctx = this.canvas.getContext('2d')!;
    const t = (this.screenFlashUntil - now) / 200; // 1→0
    ctx.save();
    ctx.globalAlpha = t * 0.2;
    ctx.fillStyle = '#ff0000';
    const w = this.canvas.width;
    const h = this.canvas.height;
    const thickness = 12;
    ctx.fillRect(0, 0, w, thickness);
    ctx.fillRect(0, h - thickness, w, thickness);
    ctx.fillRect(0, thickness, thickness, h - thickness * 2);
    ctx.fillRect(w - thickness, thickness, thickness, h - thickness * 2);
    ctx.globalAlpha = 1;
    ctx.restore();
  }

  private async drawMissiles(): Promise<void> {
    if (this.missiles.size === 0) return;
    const ctx = this.canvas.getContext('2d')!;
    const base = `/data/objects/bitmaps/${this.objset}`;

    for (const anim of this.missiles.values()) {
      const obj = this.objects[anim.objType];
      let bm: ImageBitmap | null = null;
      if (obj?.bitmap) {
        let imgData: ImageData | null;
        if (obj.masked && obj.mask) {
          imgData = await loadMaskedSprite(`${base}/${obj.bitmap}`, `${base}/${obj.mask}`);
        } else {
          imgData = await loadSprite(`${base}/${obj.bitmap}`);
        }
        bm = imgData ? await getBitmap(imgData) : null;
      }

      const ox = BORDER + anim.x * TILE;
      const oy = BORDER + anim.y * TILE;
      const cx = ox + TILE / 2;
      const cy = oy + TILE / 2;
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
          ctx.drawImage(bm, ox, oy, TILE, TILE);
        }
      } else {
        // Fallback: yellow square
        ctx.fillStyle = '#ffff00';
        ctx.fillRect(ox + 12, oy + 12, 8, 8);
      }
    }
  }
}
