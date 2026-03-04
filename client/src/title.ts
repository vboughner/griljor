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

// Margins around the terrain viewport (px)
const MARGIN_X = 48;
const MARGIN_BOTTOM = 48;

// ── Original scene.c terrain data (40×20 tile map) ───────────────────────────

// Beach: for each row 0-19, beach tiles span columns [start, end]
const BEACH_RANGES = [
  [17,39],[17,39],[30,39],[30,38],[30,37],[30,37],[30,37],[30,36],[30,35],[0,4],
  [0,7],  [0,8],  [0,9],  [0,14], [0,13], [0,13], [0,13], [0,12], [0,1],  [0,10],
];
// Forest loop 1: rows 5-9
const FOREST1_RANGES = [[0,13],[0,12],[0,11],[0,10],[0,9]];
// Forest loop 2: rows 8-19
const FOREST2_RANGES = [
  [20,22],[19,22],[18,23],[18,23],[18,23],[15,23],[14,23],[14,33],[14,39],[13,39],[27,39],[11,39],
];
// Forest loop 3: rows 3-16
const FOREST3_RANGES = [
  [39,39],[38,39],[38,39],[38,39],[37,39],[36,39],[35,39],[35,39],
  [35,39],[35,39],[35,39],[35,39],[35,39],[35,39],
];
// Sand loop 1: rows 3-12
const SAND1_RANGES = [
  [17,28],[17,28],[14,28],[13,28],[12,28],[11,19],[8,18],[8,17],[9,17],[10,17],
];
// Sand loop 2: rows 8-14
const SAND2_RANGES = [[23,28],[23,28],[24,33],[24,33],[24,33],[24,33],[24,33]];

// Horizontal paths: [row, colStart, colEnd]
const HPATHS = [[2,14,28],[3,6,12],[6,3,4],[9,30,33],[16,27,33],[18,2,25]];
// Vertical paths: [col, rowStart, rowEnd]
const VPATHS = [[2,7,18],[5,4,5],[26,17,17],[29,3,8],[34,10,15]];
// Corner path tiles: [col, row]
const LR_CORNERS = [[13,2],[5,3],[2,6],[26,16]];
const LL_CORNERS = [[29,2],[34,9]];
const UR_CORNERS = [[2,18],[29,9]];
const UL_CORNERS = [[5,6],[13,3],[26,18],[34,16]];

// River sections — raw arrays from scene.c [count, x, y, x, y, ...]
// r0: uses river[0|1] + mask[0|1]
const R0 = [39, 6,2, 5,2, 9,9, 11,11, 12,11, 13,11, 14,11, 15,11, 16,11, 18,7, 19,7, 20,7, 21,7, 22,7, 23,7, 24,7, 26,14, 27,14, 28,14];
// r2: uses river[2|3] + mask[2|3]
const R2 = [37, 7,1, 9,4, 11,8, 13,9, 16,4, 17,4, 18,4, 19,4, 20,4, 21,4, 22,4, 23,4, 24,4, 25,4, 26,4, 28,11, 29,11, 30,11];
// r4: uses river[4|5] + mask[4|5]
const R4 = [33, 6,0, 8,2, 10,5, 10,6, 10,7, 17,8, 17,9, 17,10, 28,6, 28,7, 28,8, 27,10, 32,13, 33,15, 34,18, 34,19];
// r6: uses river[6|7] + mask[6|7]
const R6 = [33, 4,0, 4,1, 7,4, 7,5, 8,7, 8,8, 10,10, 14,7, 14,8, 15,5, 24,9, 24,10, 24,11, 25,13, 30,17, 31,19];
// r8: uses river[8] + mask[8]
const R8 = [7, 14,6, 15,4, 24,8];
// r9: uses river[9] + mask[9]
const R9 = [19, 8,1, 10,4, 12,8, 27,4, 28,5, 31,11, 32,12, 33,14, 34,17];
// r10: uses river[10|20] + mask[10|20]
const R10 = [17, 4,2, 7,6, 8,9, 10,11, 24,12, 25,14, 29,15, 30,18];
// r11: uses river[11] + mask[11]
const R11 = [5, 17,11, 28,9];
// r12: uses river[12] + mask[12]
const R12 = [5, 17,7, 27,9];
// r13: uses river[13] + mask[13]
const R13 = [17, 7,2, 8,6, 10,9, 25,12, 25,7, 29,14, 30,15, 31,18];
// r14: uses river[14] + mask[14]
const R14 = [19, 6,1, 8,4, 10,8, 27,5, 27,11, 31,12, 32,14, 33,17, 12,9];
// r15: uses river[15] + mask[15]
const R15 = [7, 14,9, 15,6, 25,8];
// r16: uses river[16|17], no mask
const R16 = [123,
  5,0, 5,1, 8,5, 9,5, 9,6, 9,7, 9,8, 11,9, 11,10, 12,10, 13,10, 14,10, 15,10, 16,10,
  15,7, 15,8, 15,9, 16,5, 16,6, 16,7, 16,8, 16,9,
  26,5, 26,6, 26,7, 26,8, 26,9, 26,10, 26,11, 26,12, 26,13,
  26,6, 26,7, 26,8, 27,6, 27,7, 27,8, 25,9, 25,10, 25,11,
  26,12, 27,12, 28,12, 29,12, 30,12,
  26,13, 27,13, 28,13, 29,13, 30,13, 31,13,
  31,15, 30,14, 31,14, 32,15, 31,17, 32,17, 32,18, 32,19, 33,18, 33,19,
];

const TERRAIN_NAMES = [
  'beach0', 'beach1', 'beach2', 'forest', 'sand',
  'hpath', 'vpath', 'lrpath', 'llpath', 'urpath', 'ulpath',
  ...Array.from({ length: 19 }, (_, i) => `river${i}`), // river0-river18
  'river20',
];

// ── Types ────────────────────────────────────────────────────────────────────

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

// ── Helpers ──────────────────────────────────────────────────────────────────

function randDir(): { dx: number; dy: number } {
  const dirs = [{ dx: 1, dy: 0 }, { dx: -1, dy: 0 }, { dx: 0, dy: 1 }, { dx: 0, dy: -1 }];
  return dirs[Math.floor(Math.random() * dirs.length)];
}

function rn2(): number { return Math.floor(Math.random() * 2); }

async function loadBitmap(url: string): Promise<ImageBitmap | null> {
  const id = await loadSprite(url);
  return id ? createImageBitmap(id) : null;
}

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
      const c = oc.getContext('2d')!;
      c.drawImage(img, 0, 0);
      const id = c.getImageData(0, 0, img.width, img.height);
      for (let i = 0; i < id.data.length; i += 4) {
        if (id.data[i] >= 200) {
          id.data[i + 3] = 0;
        } else {
          id.data[i] = 0; id.data[i + 1] = 0; id.data[i + 2] = 0; id.data[i + 3] = 255;
        }
      }
      c.putImageData(id, 0, 0);
      createImageBitmap(oc).then(resolve).catch(() => resolve(null));
    };
    img.onerror = () => resolve(null);
    img.src = maskUrl;
  });
}

async function loadFaceSprite(name: string): Promise<FaceSprite> {
  const burl = `/sprites/facebits/${name}bit.png`;
  const murl = `/sprites/facebits/${name}mask.png`;
  const [face, mask] = await Promise.all([loadMaskedBitmap(burl, murl), loadMaskSilhouette(murl)]);
  return { face, mask };
}

function drawFace(ctx: CanvasRenderingContext2D, sprite: FaceSprite, x: number, y: number, size: number): void {
  if (sprite.mask) ctx.drawImage(sprite.mask, x, y, size, size);
  if (sprite.face) ctx.drawImage(sprite.face, x, y, size, size);
}

// ── Terrain map (ported from scene.c) ────────────────────────────────────────

function drawRiverSection(
  ctx: OffscreenCanvasRenderingContext2D,
  pts: number[],
  tiles: Map<string, ImageBitmap | null>,
  tileA: string,
  tileB: string,
): void {
  for (let i = 1; i < pts[0]; i += 2) {
    const bm = tiles.get(rn2() === 0 ? tileA : tileB);
    if (bm) ctx.drawImage(bm, pts[i] * TILE, pts[i + 1] * TILE, TILE, TILE);
  }
}

function drawRiverFixed(
  ctx: OffscreenCanvasRenderingContext2D,
  pts: number[],
  tiles: Map<string, ImageBitmap | null>,
  tileName: string,
): void {
  for (let i = 1; i < pts[0]; i += 2) {
    const bm = tiles.get(tileName);
    if (bm) ctx.drawImage(bm, pts[i] * TILE, pts[i + 1] * TILE, TILE, TILE);
  }
}

async function buildTerrainMap(tiles: Map<string, ImageBitmap | null>): Promise<OffscreenCanvas> {
  const W = 40 * TILE; // 1280
  const H = 20 * TILE; // 640
  const oc = new OffscreenCanvas(W, H);
  const ctx = oc.getContext('2d')!;

  // White background — will be inverted to black at the end
  ctx.fillStyle = '#fff';
  ctx.fillRect(0, 0, W, H);

  const t = (name: string) => tiles.get(name) ?? null;

  // Beach loop
  for (let row = 0; row <= 19; row++) {
    const [cs, ce] = BEACH_RANGES[row];
    for (let col = cs; col <= ce; col++) {
      const bm = t(`beach${Math.floor(Math.random() * 3)}`);
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }
  }

  // Forest loop 0: full rectangle x=0..16, y=0..4
  for (let col = 0; col <= 16; col++)
    for (let row = 0; row <= 4; row++) {
      const bm = t('forest');
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }

  // Forest loop 1: rows 5-9
  for (let row = 5; row <= 9; row++) {
    const [cs, ce] = FOREST1_RANGES[row - 5];
    for (let col = cs; col <= ce; col++) {
      const bm = t('forest');
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }
  }

  // Forest loop 2: rows 8-19
  for (let row = 8; row <= 19; row++) {
    const [cs, ce] = FOREST2_RANGES[row - 8];
    for (let col = cs; col <= ce; col++) {
      const bm = t('forest');
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }
  }

  // Forest loop 3: rows 3-16
  for (let row = 3; row <= 16; row++) {
    const [cs, ce] = FOREST3_RANGES[row - 3];
    for (let col = cs; col <= ce; col++) {
      const bm = t('forest');
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }
  }

  // Sand loop 1: rows 3-12
  for (let row = 3; row <= 12; row++) {
    const [cs, ce] = SAND1_RANGES[row - 3];
    for (let col = cs; col <= ce; col++) {
      const bm = t('sand');
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }
  }

  // Sand loop 2: rows 8-14
  for (let row = 8; row <= 14; row++) {
    const [cs, ce] = SAND2_RANGES[row - 8];
    for (let col = cs; col <= ce; col++) {
      const bm = t('sand');
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }
  }

  // Horizontal paths
  for (const [row, cs, ce] of HPATHS)
    for (let col = cs; col <= ce; col++) {
      const bm = t('hpath');
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }

  // Vertical paths
  for (const [col, rs, re] of VPATHS)
    for (let row = rs; row <= re; row++) {
      const bm = t('vpath');
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }

  // Corner path tiles
  for (const [col, row] of LR_CORNERS) { const bm = t('lrpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (const [col, row] of LL_CORNERS) { const bm = t('llpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (const [col, row] of UR_CORNERS) { const bm = t('urpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (const [col, row] of UL_CORNERS) { const bm = t('ulpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }

  // River sections (ported from scene.c)
  drawRiverSection(ctx, R0,  tiles, 'river0',  'river1');
  drawRiverSection(ctx, R2,  tiles, 'river2',  'river3');
  drawRiverSection(ctx, R4,  tiles, 'river4',  'river5');
  drawRiverSection(ctx, R6,  tiles, 'river6',  'river7');
  drawRiverFixed  (ctx, R8,  tiles, 'river8');
  drawRiverFixed  (ctx, R9,  tiles, 'river9');
  drawRiverSection(ctx, R10, tiles, 'river10', 'river20');
  drawRiverFixed  (ctx, R11, tiles, 'river11');
  drawRiverFixed  (ctx, R12, tiles, 'river12');
  drawRiverFixed  (ctx, R13, tiles, 'river13');
  drawRiverFixed  (ctx, R14, tiles, 'river14');
  drawRiverFixed  (ctx, R15, tiles, 'river15');
  drawRiverSection(ctx, R16, tiles, 'river16', 'river17');

  // River rectangle x=17..25, y=5..6
  for (let col = 17; col <= 25; col++)
    for (let row = 5; row <= 6; row++) {
      const bm = t(`river${17 + rn2()}`);
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }

  // Invert all pixels for dark mode: white bg → black, black features → white
  const id = ctx.getImageData(0, 0, W, H);
  const d = id.data;
  for (let i = 0; i < d.length; i += 4) {
    d[i]   = 255 - d[i];
    d[i+1] = 255 - d[i+1];
    d[i+2] = 255 - d[i+2];
  }
  ctx.putImageData(id, 0, 0);

  return oc;
}

// ── Main entry point ─────────────────────────────────────────────────────────

export async function runTitleScreen(canvas: HTMLCanvasElement): Promise<void> {
  let resolveP!: () => void;
  const done = new Promise<void>((r) => { resolveP = r; });

  const W = window.innerWidth || 800;
  const H = window.innerHeight || 600;
  canvas.width = W;
  canvas.height = H;
  const ctx = canvas.getContext('2d')!;

  // Layout: top band (letters) = H/4, bottom band (terrain) = 3H/4
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
    Promise.all(TERRAIN_NAMES.map((n) => loadRawBitmap(`/sprites/bitmaps/${n}.png`))),
    Promise.all(AVATARS.map((n) => loadFaceSprite(n))),
  ]);

  if (dismissed) return done;

  const tileMap = new Map<string, ImageBitmap | null>(
    TERRAIN_NAMES.map((n, i) => [n, terrainBitmaps[i]])
  );

  const mapCanvas = await buildTerrainMap(tileMap);
  const mapW = mapCanvas.width;  // 1280
  const mapH = mapCanvas.height; // 640

  if (dismissed) return done;

  // Terrain viewport: the portion of the bottom band that shows the map
  const vpW = Math.min(W - 2 * MARGIN_X, mapW);
  const vpH = Math.min(bottomH - MARGIN_BOTTOM, mapH);
  const vpX = Math.floor((W - vpW) / 2); // centered horizontally
  const maxCamX = Math.max(0, mapW - vpW);
  const maxCamY = Math.max(0, mapH - vpH);

  // ── Letter layout ────────────────────────────────────────────────
  const PROMPT_FONT_SIZE = 13;
  const PROMPT_MARGIN = 8;
  const promptY = topH - PROMPT_MARGIN;
  const usableLetterH = promptY - PROMPT_FONT_SIZE - PROMPT_MARGIN * 2;

  let scale = 1;
  for (const bm of rawLetters)
    if (bm && bm.height > 0) scale = Math.min(scale, usableLetterH / bm.height);

  const totalLetterW = rawLetters.reduce(
    (s, bm) => s + Math.floor((bm?.width ?? 48) * scale) + LETTER_PAD, -LETTER_PAD
  );
  if (totalLetterW > W - 40) scale *= (W - 40) / totalLetterW;

  const finalTotalW = rawLetters.reduce(
    (s, bm) => s + Math.floor((bm?.width ?? 48) * scale) + LETTER_PAD, -LETTER_PAD
  );
  let xCursor = Math.max(20, Math.floor((W - finalTotalW) / 2));

  const tallestLetter = rawLetters.reduce(
    (h, bm) => Math.max(h, bm ? Math.floor(bm.height * scale) : 0), 0
  );
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
  const walkers: Walker[] = Array.from({ length: AVATARS.length }, (_, i) => {
    const d = randDir();
    return {
      px: Math.floor(Math.random() * mapW),
      py: Math.floor(Math.random() * mapH),
      dx: d.dx, dy: d.dy,
      sprite: faceSprites[i],
      nextDir: Math.floor(Math.random() * 120) + 30,
    };
  });

  // ── Camera ───────────────────────────────────────────────────────
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

    for (const s of letterSprites)
      if (s.bm && s.x > -s.w)
        ctx.drawImage(s.bm, s.x, s.targetY + s.extraY, s.w, s.h);

    ctx.fillStyle = '#888';
    ctx.font = `${PROMPT_FONT_SIZE}px monospace`;
    ctx.textAlign = 'center';
    ctx.fillText('press any key or click to continue', W / 2, promptY);
    ctx.textAlign = 'left';

    ctx.restore();

    // ── Bottom band: terrain + walkers ────────────────────────────
    ctx.save();
    ctx.beginPath(); ctx.rect(vpX, bottomY, vpW, vpH); ctx.clip();

    // Camera panning: right → down → left → up
    if      (camDir === 0) { camX += CAM_SPEED; if (camX >= maxCamX) { camX = maxCamX; camDir = 1; } }
    else if (camDir === 1) { camY += CAM_SPEED; if (camY >= maxCamY) { camY = maxCamY; camDir = 2; } }
    else if (camDir === 2) { camX -= CAM_SPEED; if (camX <= 0)       { camX = 0;       camDir = 3; } }
    else                   { camY -= CAM_SPEED; if (camY <= 0)       { camY = 0;       camDir = 0; } }

    ctx.drawImage(mapCanvas, camX, camY, vpW, vpH, vpX, bottomY, vpW, vpH);

    for (const w of walkers) {
      w.px = ((w.px + w.dx) % mapW + mapW) % mapW;
      w.py = ((w.py + w.dy) % mapH + mapH) % mapH;
      if (frame >= w.nextDir) {
        const d = randDir();
        w.dx = d.dx; w.dy = d.dy;
        w.nextDir = frame + Math.floor(Math.random() * 120) + 60;
      }
      const sx = vpX + (w.px - camX);
      const sy = bottomY + (w.py - camY);
      if (sx > vpX - TILE && sx < vpX + vpW && sy > bottomY - TILE && sy < bottomY + vpH)
        drawFace(ctx, w.sprite, sx, sy, TILE);
    }

    ctx.restore();

    // Divider line between top and bottom bands
    ctx.strokeStyle = '#2a2a2a';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(0, topH); ctx.lineTo(W, topH);
    ctx.stroke();
  }

  loop();
  return done;
}
