import { loadSprite, loadMaskedSprite } from './assets';

const AVATARS = [
  'aaron', 'adriana', 'albert', 'aragorn', 'avatar', 'bh', 'crescendo',
  'crom', 'drustan', 'duel', 'eric', 'gm', 'mahatma', 'mcelhoe', 'mel',
  'mike', 'mikey', 'moronus', 'ollie', 'savaki', 'spook', 'stefan',
  'stinglai', 'trevor', 'van',
];

const LETTER_NAMES = ['g', 'r1', 'i', 'l', 'j', 'o1', 'r2'];
const TILE = 32;
const LETTER_PAD = 8;
const CAM_SPEED = 2;

const BASE_TILE_POOL = [
  'beach0', 'beach0', 'beach0',
  'beach1', 'beach1', 'beach1',
  'beach2', 'beach2',
  'sand', 'sand', 'sand', 'sand', 'sand',
  'forest', 'forest', 'forest',
];

const TERRAIN_NAMES = [
  'beach0', 'beach1', 'beach2', 'sand', 'forest', 'hpath',
  ...Array.from({ length: 10 }, (_, i) => `river${i}`),
];

interface FaceSprite {
  face: ImageBitmap | null;
  mask: ImageBitmap | null;
}

interface LetterSprite {
  bm: ImageBitmap | null;
  w: number;
  h: number;
  targetX: number;
  targetY: number;
  x: number;
  extraY: number;
  done: boolean;
}

interface Walker {
  px: number;
  py: number;
  dx: number;
  dy: number;
  sprite: FaceSprite;
  nextDir: number;
}

function randDir(): { dx: number; dy: number } {
  const dirs = [{ dx: 1, dy: 0 }, { dx: -1, dy: 0 }, { dx: 0, dy: 1 }, { dx: 0, dy: -1 }];
  return dirs[Math.floor(Math.random() * dirs.length)];
}

async function loadBitmap(url: string): Promise<ImageBitmap | null> {
  const id = await loadSprite(url);
  return id ? createImageBitmap(id) : null;
}

/** Load a PNG as-is with no color processing, for use with manual inversion. */
async function loadRawBitmap(url: string): Promise<ImageBitmap | null> {
  return new Promise((resolve) => {
    const img = new Image();
    img.onload = () => createImageBitmap(img).then(resolve).catch(() => resolve(null));
    img.onerror = () => resolve(null);
    img.src = url;
  });
}

async function loadMaskedBitmap(burl: string, murl: string): Promise<ImageBitmap | null> {
  const id = await loadMaskedSprite(burl, murl);
  return id ? createImageBitmap(id) : null;
}

async function loadMaskSilhouette(maskUrl: string): Promise<ImageBitmap | null> {
  return new Promise((resolve) => {
    const img = new Image();
    img.onload = () => {
      const oc = new OffscreenCanvas(img.width, img.height);
      const ctx = oc.getContext('2d')!;
      ctx.drawImage(img, 0, 0);
      const id = ctx.getImageData(0, 0, img.width, img.height);
      for (let i = 0; i < id.data.length; i += 4) {
        if (id.data[i] >= 200) {
          id.data[i + 3] = 0;
        } else {
          id.data[i] = 0; id.data[i + 1] = 0; id.data[i + 2] = 0;
          id.data[i + 3] = 255;
        }
      }
      ctx.putImageData(id, 0, 0);
      createImageBitmap(oc).then(resolve).catch(() => resolve(null));
    };
    img.onerror = () => resolve(null);
    img.src = maskUrl;
  });
}

async function loadFaceSprite(name: string): Promise<FaceSprite> {
  const burl = `/sprites/facebits/${name}bit.png`;
  const murl = `/sprites/facebits/${name}mask.png`;
  const [face, mask] = await Promise.all([
    loadMaskedBitmap(burl, murl),
    loadMaskSilhouette(murl),
  ]);
  return { face, mask };
}

function drawFace(ctx: CanvasRenderingContext2D, sprite: FaceSprite, x: number, y: number, size: number): void {
  if (sprite.mask) ctx.drawImage(sprite.mask, x, y, size, size);
  if (sprite.face) ctx.drawImage(sprite.face, x, y, size, size);
}

async function buildTerrainMap(
  tiles: Map<string, ImageBitmap | null>,
  cols: number,
  rows: number,
): Promise<OffscreenCanvas> {
  const W = cols * TILE;
  const H = rows * TILE;
  const oc = new OffscreenCanvas(W, H);
  const ctx = oc.getContext('2d')!;
  ctx.fillStyle = '#111';
  ctx.fillRect(0, 0, W, H);

  for (let c = 0; c < cols; c++) {
    for (let r = 0; r < rows; r++) {
      const name = BASE_TILE_POOL[Math.floor(Math.random() * BASE_TILE_POOL.length)];
      const bm = tiles.get(name);
      if (bm) ctx.drawImage(bm, c * TILE, r * TILE, TILE, TILE);
    }
  }

  // River band
  const riverRow = Math.floor(rows * 0.55);
  for (let c = 0; c < cols; c++) {
    const bm = tiles.get(`river${Math.floor(Math.random() * 10)}`);
    if (bm) ctx.drawImage(bm, c * TILE, riverRow * TILE, TILE, TILE);
  }

  // Horizontal path
  const pathRow = Math.floor(rows * 0.3);
  for (let c = 0; c < cols; c++) {
    if (Math.random() < 0.6) {
      const bm = tiles.get('hpath');
      if (bm) ctx.drawImage(bm, c * TILE, pathRow * TILE, TILE, TILE);
    }
  }

  // Invert all pixels for dark mode (white bg → black, black features → white)
  const imgData = ctx.getImageData(0, 0, W, H);
  const d = imgData.data;
  for (let i = 0; i < d.length; i += 4) {
    d[i]   = 255 - d[i];
    d[i+1] = 255 - d[i+1];
    d[i+2] = 255 - d[i+2];
    // alpha unchanged
  }
  ctx.putImageData(imgData, 0, 0);

  return oc;
}

export async function runTitleScreen(canvas: HTMLCanvasElement): Promise<void> {
  let resolveP!: () => void;
  const done = new Promise<void>((r) => { resolveP = r; });

  const W = window.innerWidth || 800;
  const H = window.innerHeight || 600;
  canvas.width = W;
  canvas.height = H;
  const ctx = canvas.getContext('2d')!;

  // Layout: top band (letters) = 1/4 height, bottom band (terrain) = 3/4 height
  const topH = Math.floor(H / 4);
  const bottomY = topH;
  const bottomH = H - topH;

  let animId = 0;
  let dismissed = false;

  function dismiss() {
    if (dismissed) return;
    dismissed = true;
    cancelAnimationFrame(animId);
    document.removeEventListener('keydown', onKey);
    document.removeEventListener('pointerdown', onPointer);
    resolveP();
  }

  function onKey() { dismiss(); }
  function onPointer() { dismiss(); }
  document.addEventListener('keydown', onKey);
  document.addEventListener('pointerdown', onPointer);

  // Load assets
  const [rawLetters, terrainBitmaps, faceSprites] = await Promise.all([
    Promise.all(LETTER_NAMES.map((n) => loadBitmap(`/sprites/bitmaps/${n}.png`))),
    Promise.all(TERRAIN_NAMES.map((n) => loadBitmap(`/sprites/bitmaps/${n}.png`))),
    Promise.all(AVATARS.map((n) => loadFaceSprite(n))),
  ]);

  if (dismissed) return done;

  const tileMap = new Map<string, ImageBitmap | null>(
    TERRAIN_NAMES.map((n, i) => [n, terrainBitmaps[i]])
  );

  // Build terrain map sized to cover viewport with room to pan
  const mapCols = Math.ceil(W / TILE) + 24;
  const mapRows = Math.ceil(bottomH / TILE) + 12;
  const mapCanvas = await buildTerrainMap(tileMap, mapCols, mapRows);
  const mapW = mapCanvas.width;
  const mapH = mapCanvas.height;

  if (dismissed) return done;

  // ── Letter layout ────────────────────────────────────────────────
  const maxLetterH = Math.floor(topH * 0.7);
  let scale = 1;
  for (const bm of rawLetters) {
    if (bm && bm.height > 0) scale = Math.min(scale, maxLetterH / bm.height);
  }
  const totalLetterW = rawLetters.reduce(
    (s, bm) => s + Math.floor((bm?.width ?? 48) * scale) + LETTER_PAD, -LETTER_PAD
  );
  if (totalLetterW > W - 40) scale *= (W - 40) / totalLetterW;

  const finalTotalW = rawLetters.reduce(
    (s, bm) => s + Math.floor((bm?.width ?? 48) * scale) + LETTER_PAD, -LETTER_PAD
  );
  let xCursor = Math.max(20, Math.floor((W - finalTotalW) / 2));

  // Reserve space at the bottom of the top band for the prompt line
  const PROMPT_FONT_SIZE = 13;
  const PROMPT_MARGIN = 8;
  const promptY = topH - PROMPT_MARGIN; // baseline of prompt text

  // Letters fill the remaining height above the prompt
  const usableLetterH = promptY - PROMPT_FONT_SIZE - PROMPT_MARGIN * 2;
  const tallestLetter = rawLetters.reduce((h, bm) => Math.max(h, bm ? Math.floor(bm.height * scale) : 0), 0);
  const letterBaseY = Math.max(0, Math.floor((usableLetterH - tallestLetter) / 2));

  const letterSprites: LetterSprite[] = rawLetters.map((bm, i) => {
    const w = bm ? Math.floor(bm.width * scale) : 48;
    const h = bm ? Math.floor(bm.height * scale) : 64;
    const sprite: LetterSprite = {
      bm,
      w, h,
      targetX: xCursor,
      targetY: letterBaseY,
      x: bm ? -w - 10 : xCursor,
      extraY: LETTER_NAMES[i] === 'j' ? Math.floor(h * 0.2) : 0,
      done: !bm,
    };
    xCursor += w + LETTER_PAD;
    return sprite;
  });

  let allLettersDone = false;
  let startedCount = 1;

  // ── Walkers ──────────────────────────────────────────────────────
  const walkers: Walker[] = Array.from({ length: 12 }, (_, i) => {
    const d = randDir();
    return {
      px: Math.floor(Math.random() * mapW),
      py: Math.floor(Math.random() * mapH),
      dx: d.dx, dy: d.dy,
      sprite: faceSprites[i % faceSprites.length],
      nextDir: Math.floor(Math.random() * 120) + 30,
    };
  });

  // ── Camera ───────────────────────────────────────────────────────
  const maxCamX = Math.max(0, mapW - W);
  const maxCamY = Math.max(0, mapH - bottomH);
  let camX = 0, camY = 0, camDir = 0;

  let frame = 0;

  function loop() {
    if (dismissed) return;
    animId = requestAnimationFrame(loop);
    frame++;

    ctx.fillStyle = '#0a0a0a';
    ctx.fillRect(0, 0, W, H);

    // ── Top band: GRILJOR letters ─────────────────────────────────
    ctx.save();
    ctx.beginPath(); ctx.rect(0, 0, W, topH); ctx.clip();

    if (!allLettersDone) {
      for (let i = 0; i < startedCount && i < letterSprites.length; i++) {
        const s = letterSprites[i];
        if (!s.done) {
          s.x += 10;
          if (s.x >= s.targetX) { s.x = s.targetX; s.done = true; }
        }
      }
      if (startedCount < letterSprites.length) {
        const last = letterSprites[startedCount - 1];
        const totalDist = last.targetX - (-last.w - 10);
        const traveled = last.x - (-last.w - 10);
        if (totalDist > 0 && traveled / totalDist >= 0.8) startedCount++;
      }
      if (letterSprites.every((s) => s.done)) allLettersDone = true;
    }

    for (const s of letterSprites) {
      if (s.bm && s.x > -s.w) {
        ctx.drawImage(s.bm, s.x, s.targetY + s.extraY, s.w, s.h);
      }
    }

    ctx.fillStyle = '#888';
    ctx.font = `${PROMPT_FONT_SIZE}px monospace`;
    ctx.textAlign = 'center';
    ctx.fillText('press any key or click to continue', W / 2, promptY);
    ctx.textAlign = 'left';

    ctx.restore();

    // ── Bottom band: terrain + walkers ────────────────────────────
    ctx.save();
    ctx.beginPath(); ctx.rect(0, bottomY, W, bottomH); ctx.clip();

    if      (camDir === 0) { camX += CAM_SPEED; if (camX >= maxCamX) { camX = maxCamX; camDir = 1; } }
    else if (camDir === 1) { camY += CAM_SPEED; if (camY >= maxCamY) { camY = maxCamY; camDir = 2; } }
    else if (camDir === 2) { camX -= CAM_SPEED; if (camX <= 0)       { camX = 0;       camDir = 3; } }
    else                   { camY -= CAM_SPEED; if (camY <= 0)       { camY = 0;       camDir = 0; } }

    ctx.drawImage(mapCanvas, camX, camY, W, bottomH, 0, bottomY, W, bottomH);

    for (const w of walkers) {
      w.px = ((w.px + w.dx) % mapW + mapW) % mapW;
      w.py = ((w.py + w.dy) % mapH + mapH) % mapH;
      if (frame >= w.nextDir) {
        const d = randDir();
        w.dx = d.dx; w.dy = d.dy;
        w.nextDir = frame + Math.floor(Math.random() * 120) + 60;
      }
      const sx = w.px - camX;
      const sy = bottomY + (w.py - camY);
      if (sx > -TILE && sx < W + TILE && sy > bottomY - TILE && sy < bottomY + bottomH) {
        drawFace(ctx, w.sprite, sx, sy, TILE);
      }
    }

    ctx.restore();

    // ── Divider ───────────────────────────────────────────────────
    ctx.strokeStyle = '#2a2a2a';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(0, topH); ctx.lineTo(W, topH);
    ctx.stroke();
  }

  loop();
  return done;
}
