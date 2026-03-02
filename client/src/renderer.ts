import { RoomData, ObjDef } from './types';
import { loadMaskedSprite, loadSprite } from './assets';

const TILE = 32;
const GRID = 20;
export const CANVAS_SIZE = TILE * GRID; // 640

/**
 * Build the URL for an object's bitmap or mask, given the objset name.
 */
function bitmapUrl(objset: string, filename: string): string {
  return `/data/objects/bitmaps/${objset}/${filename}`;
}

/**
 * Load the sprite ImageData for a single object definition.
 * Returns null if no bitmap is defined.
 */
async function spriteForObj(obj: ObjDef, objset: string): Promise<ImageData | null> {
  if (!obj.bitmap) return null;
  const burl = bitmapUrl(objset, obj.bitmap);
  if (obj.masked && obj.mask) {
    return loadMaskedSprite(burl, bitmapUrl(objset, obj.mask));
  }
  return loadSprite(burl);
}

/**
 * Preload all sprites referenced by tiles in the given room.
 */
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

/**
 * Draw a single ImageData tile at grid position (gx, gy) on the canvas context.
 */
function drawTile(ctx: CanvasRenderingContext2D, img: ImageData, gx: number, gy: number): void {
  const oc = new OffscreenCanvas(TILE, TILE);
  const octx = oc.getContext('2d')!;
  octx.putImageData(img, 0, 0);
  ctx.drawImage(oc, gx * TILE, gy * TILE, TILE, TILE);
}

/**
 * Render the full room to the canvas.
 */
export async function renderRoom(
  canvas: HTMLCanvasElement,
  room: RoomData,
  objects: ObjDef[],
  objset: string,
  playerSprite: ImageData | null
): Promise<void> {
  const ctx = canvas.getContext('2d')!;

  // Background
  ctx.fillStyle = '#333';
  ctx.fillRect(0, 0, CANVAS_SIZE, CANVAS_SIZE);

  // Build sprite cache for this render pass
  const spriteMap = new Map<number, ImageData | null>();
  const getSprite = async (id: number): Promise<ImageData | null> => {
    if (spriteMap.has(id)) return spriteMap.get(id)!;
    const obj = objects[id];
    const img = obj ? await spriteForObj(obj, objset) : null;
    spriteMap.set(id, img);
    return img;
  };

  // Floor layer
  for (let x = 0; x < GRID; x++) {
    for (let y = 0; y < GRID; y++) {
      const flId = room.spot[x][y][0];
      if (flId > 0) {
        const img = await getSprite(flId);
        if (img) drawTile(ctx, img, x, y);
      }
    }
  }

  // Wall layer (drawn on top of floor)
  for (let x = 0; x < GRID; x++) {
    for (let y = 0; y < GRID; y++) {
      const wlId = room.spot[x][y][1];
      if (wlId > 0) {
        const img = await getSprite(wlId);
        if (img) drawTile(ctx, img, x, y);
      }
    }
  }

  // Recorded objects
  for (const ro of room.recorded_objects) {
    if (ro.type <= 0) continue;
    const img = await getSprite(ro.type);
    if (img) {
      drawTile(ctx, img, ro.x, ro.y);
    } else {
      // Fallback: colored dot
      ctx.fillStyle = '#f80';
      ctx.fillRect(ro.x * TILE + 12, ro.y * TILE + 12, 8, 8);
    }
  }

  // Player avatar at center (10, 10)
  if (playerSprite) {
    drawTile(ctx, playerSprite, 10, 10);
  } else {
    // Fallback: white square
    ctx.fillStyle = '#fff';
    ctx.fillRect(10 * TILE + 8, 10 * TILE + 8, 16, 16);
  }
}
