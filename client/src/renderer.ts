import { RoomData, ObjDef } from './types';
import { loadMaskedSprite, loadSprite, getColorMode } from './assets';

const TILE = 32;
const GRID = 20;
export const CANVAS_SIZE = TILE * GRID; // 640

function bitmapUrl(objset: string, filename: string): string {
  return `/data/objects/bitmaps/${objset}/${filename}`;
}

async function spriteForObj(obj: ObjDef, objset: string): Promise<ImageData | null> {
  if (!obj.bitmap) return null;
  const burl = bitmapUrl(objset, obj.bitmap);
  if (obj.masked && obj.mask) {
    return loadMaskedSprite(burl, bitmapUrl(objset, obj.mask));
  }
  return loadSprite(burl);
}

/** Preload all sprites referenced by tiles in the given room. */
export async function preloadRoomSprites(
  room: RoomData,
  objects: ObjDef[],
  objset: string
): Promise<void> {
  const needed = new Set<number>();
  for (let x = 0; x < GRID; x++) {
    for (let y = 0; y < GRID; y++) {
      const [fl, wl] = room.spot[x][y];
      if (fl > 0) needed.add(fl);
      if (wl > 0) needed.add(wl);
    }
  }
  for (const ro of room.recorded_objects) {
    if (ro.type > 0) needed.add(ro.type);
  }
  await Promise.all([...needed].map((id) => {
    const obj = objects[id];
    if (!obj) return Promise.resolve(null);
    return spriteForObj(obj, objset);
  }));
}

/** Convert ImageData to ImageBitmap once for fast drawImage calls. */
async function toBitmap(img: ImageData): Promise<ImageBitmap> {
  return createImageBitmap(img);
}

/** Cache of ImageData → ImageBitmap to avoid re-converting on each draw. */
const bitmapCache = new WeakMap<ImageData, ImageBitmap>();

async function getBitmap(img: ImageData): Promise<ImageBitmap> {
  if (bitmapCache.has(img)) return bitmapCache.get(img)!;
  const bm = await toBitmap(img);
  bitmapCache.set(img, bm);
  return bm;
}

/**
 * Pre-render the static room (background fill + floor + walls + recorded objects)
 * to an OffscreenCanvas. Call once per room load or mode change.
 */
export async function buildRoomBackground(
  room: RoomData,
  objects: ObjDef[],
  objset: string
): Promise<OffscreenCanvas> {
  const oc = new OffscreenCanvas(CANVAS_SIZE, CANVAS_SIZE);
  const ctx = oc.getContext('2d')!;

  ctx.fillStyle = getColorMode() === 'dark' ? '#333' : '#e8e8e8';
  ctx.fillRect(0, 0, CANVAS_SIZE, CANVAS_SIZE);

  const spriteMap = new Map<number, ImageBitmap | null>();
  const getSprite = async (id: number): Promise<ImageBitmap | null> => {
    if (spriteMap.has(id)) return spriteMap.get(id)!;
    const obj = objects[id];
    const imgData = obj ? await spriteForObj(obj, objset) : null;
    const bm = imgData ? await getBitmap(imgData) : null;
    spriteMap.set(id, bm);
    return bm;
  };

  // Floor layer
  for (let x = 0; x < GRID; x++) {
    for (let y = 0; y < GRID; y++) {
      const flId = room.spot[x][y][0];
      if (flId > 0) {
        const bm = await getSprite(flId);
        if (bm) ctx.drawImage(bm, x * TILE, y * TILE, TILE, TILE);
      }
    }
  }

  // Wall layer
  for (let x = 0; x < GRID; x++) {
    for (let y = 0; y < GRID; y++) {
      const wlId = room.spot[x][y][1];
      if (wlId > 0) {
        const bm = await getSprite(wlId);
        if (bm) ctx.drawImage(bm, x * TILE, y * TILE, TILE, TILE);
      }
    }
  }

  // Recorded objects
  for (const ro of room.recorded_objects) {
    if (ro.type <= 0) continue;
    const obj = objects[ro.type];
    if (!obj?.bitmap) continue;
    const bm = await getSprite(ro.type);
    if (bm) ctx.drawImage(bm, ro.x * TILE, ro.y * TILE, TILE, TILE);
  }

  return oc;
}

export interface OtherPlayer {
  px: number;
  py: number;
  sprite: ImageData | null;
}

/**
 * Composite a pre-built background and player sprite onto the visible canvas.
 * Draws the local player and any other players in the same room.
 */
export async function renderFrame(
  canvas: HTMLCanvasElement,
  bg: OffscreenCanvas,
  playerSprite: ImageData | null,
  px: number,
  py: number,
  others: OtherPlayer[] = []
): Promise<void> {
  const ctx = canvas.getContext('2d')!;
  ctx.drawImage(bg, 0, 0);

  // Draw other players first (behind local player)
  for (const other of others) {
    if (other.sprite) {
      const bm = await getBitmap(other.sprite);
      ctx.drawImage(bm, other.px * TILE, other.py * TILE, TILE, TILE);
    } else {
      ctx.fillStyle = getColorMode() === 'dark' ? '#aaa' : '#666';
      ctx.fillRect(other.px * TILE + 8, other.py * TILE + 8, 16, 16);
    }
  }

  // Draw local player on top
  if (playerSprite) {
    const bm = await getBitmap(playerSprite);
    ctx.drawImage(bm, px * TILE, py * TILE, TILE, TILE);
  } else {
    ctx.fillStyle = getColorMode() === 'dark' ? '#fff' : '#000';
    ctx.fillRect(px * TILE + 8, py * TILE + 8, 16, 16);
  }
}
