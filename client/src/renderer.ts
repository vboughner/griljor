import { RoomData, ObjDef, InventoryItem } from './types';
import { loadMaskedSprite, loadSprite, loadOpaqueTile, getColorMode } from './assets';

export const TILE = 32;
const GRID = 20;
export const BORDER = TILE; // 1-tile blank border around the map
export const CANVAS_SIZE = TILE * (GRID + 2); // 704 (20 tiles + 1 border each side)

function bitmapUrl(objset: string, filename: string): string {
  return `/data/objects/bitmaps/${objset}/${filename}`;
}

async function spriteForObj(
  obj: ObjDef,
  objset: string,
  opaque = false,
): Promise<ImageData | null> {
  if (!obj.bitmap) return null;
  const burl = bitmapUrl(objset, obj.bitmap);
  if (obj.masked && obj.mask) {
    return loadMaskedSprite(burl, bitmapUrl(objset, obj.mask));
  }
  return opaque ? loadOpaqueTile(burl) : loadSprite(burl);
}

/** Preload all sprites referenced by tiles in the given room. */
export async function preloadRoomSprites(
  room: RoomData,
  objects: ObjDef[],
  objset: string,
): Promise<void> {
  const needed = new Set<number>();
  if (room.spot) {
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        const [fl, wl] = room.spot[x][y];
        if (fl > 0) needed.add(fl);
        if (wl > 0) needed.add(wl);
      }
    }
  }
  for (const ro of room.recorded_objects ?? []) {
    if (ro.type > 0) needed.add(ro.type);
  }
  await Promise.all(
    [...needed].map((id) => {
      const obj = objects[id];
      if (!obj) return Promise.resolve(null);
      return spriteForObj(obj, objset);
    }),
  );
}

/** Cache of ImageData → ImageBitmap to avoid repeated createImageBitmap calls. */
const bitmapCache = new WeakMap<ImageData, ImageBitmap>();

export async function getBitmap(img: ImageData): Promise<ImageBitmap> {
  if (bitmapCache.has(img)) return bitmapCache.get(img)!;
  const bm = await createImageBitmap(img);
  bitmapCache.set(img, bm);
  return bm;
}

/**
 * Pre-render the static room (background fill + floor + walls + non-takeable recorded objects)
 * to an OffscreenCanvas. Call once per room load or mode change.
 * Takeable items are excluded — they are rendered dynamically via floorItems.
 */
export async function buildRoomBackground(
  room: RoomData,
  objects: ObjDef[],
  objset: string,
): Promise<OffscreenCanvas> {
  const oc = new OffscreenCanvas(CANVAS_SIZE, CANVAS_SIZE);
  const ctx = oc.getContext('2d')!;
  const dark = getColorMode() === 'dark';

  // Border area (slightly darker than map background)
  ctx.fillStyle = dark ? '#111' : '#c8c8c8';
  ctx.fillRect(0, 0, CANVAS_SIZE, CANVAS_SIZE);

  // Map area background
  ctx.fillStyle = dark ? '#333' : '#e8e8e8';
  ctx.fillRect(BORDER, BORDER, GRID * TILE, GRID * TILE);

  const spriteMap = new Map<string, ImageBitmap | null>();
  const getSprite = async (id: number, opaque = false): Promise<ImageBitmap | null> => {
    const cacheKey = opaque ? `opaque:${id}` : `${id}`;
    if (spriteMap.has(cacheKey)) return spriteMap.get(cacheKey)!;
    const obj = objects[id];
    const imgData = obj ? await spriteForObj(obj, objset, opaque) : null;
    const bm = imgData ? await getBitmap(imgData) : null;
    spriteMap.set(cacheKey, bm);
    return bm;
  };

  // Floor then wall layer per tile — unmasked tiles are opaque (white → background color)
  if (room.spot) {
    for (let x = 0; x < GRID; x++) {
      for (let y = 0; y < GRID; y++) {
        const [flId, wlId] = room.spot[x][y];
        if (flId > 0) {
          const bm = await getSprite(flId, true);
          if (bm) ctx.drawImage(bm, BORDER + x * TILE, BORDER + y * TILE, TILE, TILE);
        }
        if (wlId > 0) {
          const bm = await getSprite(wlId, true);
          if (bm) ctx.drawImage(bm, BORDER + x * TILE, BORDER + y * TILE, TILE, TILE);
        }
      }
    }
  }

  // Recorded objects — skip takeable items (they're in floorItems)
  for (const ro of room.recorded_objects ?? []) {
    if (ro.type <= 0) continue;
    const obj = objects[ro.type];
    if (!obj?.bitmap) continue;
    if (obj.takeable) continue; // rendered dynamically
    const bm = await getSprite(ro.type);
    if (bm) ctx.drawImage(bm, BORDER + ro.x * TILE, BORDER + ro.y * TILE, TILE, TILE);
  }

  return oc;
}

export interface OtherPlayer {
  px: number;
  py: number;
  sprite: ImageData | null;
  dead?: boolean;
  team: number;
}

/**
 * Returns the stroke style for a player indicator box, or null if no indicator
 * should be drawn.
 *
 * @param teamsEnabled  true when the map has more than 1 team (teams_supported > 1)
 * @param localTeam     the local player's team number
 * @param otherTeam     the other player's team number
 * @param boxOtherPlayers  master toggle — false means no indicators for anyone
 */
const INDICATOR_TEAMMATE = '#00cc00';
const INDICATOR_ENEMY = '#ffffff';

export function playerIndicatorStyle(
  teamsEnabled: boolean,
  localTeam: number,
  otherTeam: number,
  boxOtherPlayers: boolean,
): { color: string; lineWidth: number } | null {
  if (!boxOtherPlayers) return null;
  if (teamsEnabled && otherTeam !== 0 && otherTeam === localTeam) {
    return { color: INDICATOR_TEAMMATE, lineWidth: 2 };
  }
  return { color: INDICATOR_ENEMY, lineWidth: 1 };
}

/**
 * Composite a pre-built background, dynamic floor items, and player sprites
 * onto the visible canvas.
 */
export async function renderFrame(
  canvas: HTMLCanvasElement,
  bg: OffscreenCanvas,
  playerSprite: ImageData | null,
  px: number,
  py: number,
  others: OtherPlayer[] = [],
  floorItems: Map<string, InventoryItem> = new Map(),
  objects: ObjDef[] = [],
  objset: string = '',
  tombstoneSprite: ImageData | null = null,
  localTeam: number = 0,
  boxOtherPlayers: boolean = false,
  teamsEnabled: boolean = false,
  isDead: boolean = false,
): Promise<void> {
  const ctx = canvas.getContext('2d')!;
  ctx.drawImage(bg, 0, 0);

  // Draw floor items (between background and players)
  for (const [key, item] of floorItems) {
    const [ix, iy] = key.split(',').map(Number);
    const obj = objects[item.type];
    if (!obj?.bitmap) continue;
    const imgData = await spriteForObj(obj, objset);
    if (imgData) {
      const bm = await getBitmap(imgData);
      ctx.drawImage(bm, BORDER + ix * TILE, BORDER + iy * TILE, TILE, TILE);
    }
  }

  // Draw other players first (behind local player)
  for (const other of others) {
    const effectiveSprite = other.dead && tombstoneSprite ? tombstoneSprite : other.sprite;
    if (effectiveSprite) {
      const bm = await getBitmap(effectiveSprite);
      ctx.drawImage(bm, BORDER + other.px * TILE, BORDER + other.py * TILE, TILE, TILE);
    } else {
      ctx.fillStyle = getColorMode() === 'dark' ? '#aaa' : '#666';
      ctx.fillRect(BORDER + other.px * TILE + 8, BORDER + other.py * TILE + 8, 16, 16);
    }
    if (boxOtherPlayers && !other.dead) {
      const style = playerIndicatorStyle(teamsEnabled, localTeam, other.team, boxOtherPlayers);
      if (style) {
        ctx.save();
        ctx.strokeStyle = style.color;
        ctx.lineWidth = style.lineWidth;
        ctx.strokeRect(BORDER + other.px * TILE, BORDER + other.py * TILE, TILE, TILE);
        ctx.restore();
      }
    }
  }

  // Draw local player on top
  if (playerSprite) {
    const bm = await getBitmap(playerSprite);
    ctx.drawImage(bm, BORDER + px * TILE, BORDER + py * TILE, TILE, TILE);
  } else {
    ctx.fillStyle = getColorMode() === 'dark' ? '#fff' : '#000';
    ctx.fillRect(BORDER + px * TILE + 8, BORDER + py * TILE + 8, 16, 16);
  }

  if (boxOtherPlayers && !isDead) {
    ctx.save();
    ctx.strokeStyle = INDICATOR_TEAMMATE;
    ctx.lineWidth = 2;
    ctx.strokeRect(BORDER + px * TILE, BORDER + py * TILE, TILE, TILE);
    ctx.restore();
  }
}
