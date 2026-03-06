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

// Margins around the terrain viewport
const MARGIN_X      = 48; // left and right
const MARGIN_BOTTOM = 48; // below terrain
const MARGIN_TOP    = 64; // gap between divider line and terrain top

// ── Original scene.c terrain data (40×20 tile map) ───────────────────────────

const BEACH_RANGES = [
  [17,39],[17,39],[30,39],[30,38],[30,37],[30,37],[30,37],[30,36],[30,35],[0,4],
  [0,7],  [0,8],  [0,9],  [0,14], [0,13], [0,13], [0,13], [0,12], [0,1],  [0,10],
];
const FOREST1_RANGES = [[0,13],[0,12],[0,11],[0,10],[0,9]];
const FOREST2_RANGES = [
  [20,22],[19,22],[18,23],[18,23],[18,23],[15,23],[14,23],[14,33],[14,39],[13,39],[27,39],[11,39],
];
const FOREST3_RANGES = [
  [39,39],[38,39],[38,39],[38,39],[37,39],[36,39],[35,39],[35,39],
  [35,39],[35,39],[35,39],[35,39],[35,39],[35,39],
];
const SAND1_RANGES = [
  [17,28],[17,28],[14,28],[13,28],[12,28],[11,19],[8,18],[8,17],[9,17],[10,17],
];
const SAND2_RANGES = [[23,28],[23,28],[24,33],[24,33],[24,33],[24,33],[24,33]];

const HPATHS = [[2,14,28],[3,6,12],[6,3,4],[9,30,33],[16,27,33],[18,2,25]];
const VPATHS = [[2,7,18],[5,4,5],[26,17,17],[29,3,8],[34,10,15]];
const LR_CORNERS = [[13,2],[5,3],[2,6],[26,16]];
const LL_CORNERS = [[29,2],[34,9]];
const UR_CORNERS = [[2,18],[29,9]];
const UL_CORNERS = [[5,6],[13,3],[26,18],[34,16]];

// River sections [count, col, row, col, row, ...]
const R0  = [39, 6,2, 5,2, 9,9, 11,11, 12,11, 13,11, 14,11, 15,11, 16,11, 18,7, 19,7, 20,7, 21,7, 22,7, 23,7, 24,7, 26,14, 27,14, 28,14];
const R2  = [37, 7,1, 9,4, 11,8, 13,9, 16,4, 17,4, 18,4, 19,4, 20,4, 21,4, 22,4, 23,4, 24,4, 25,4, 26,4, 28,11, 29,11, 30,11];
const R4  = [33, 6,0, 8,2, 10,5, 10,6, 10,7, 17,8, 17,9, 17,10, 28,6, 28,7, 28,8, 27,10, 32,13, 33,15, 34,18, 34,19];
const R6  = [33, 4,0, 4,1, 7,4, 7,5, 8,7, 8,8, 10,10, 14,7, 14,8, 15,5, 24,9, 24,10, 24,11, 25,13, 30,17, 31,19];
const R8  = [7,  14,6, 15,4, 24,8];
const R9  = [19, 8,1, 10,4, 12,8, 27,4, 28,5, 31,11, 32,12, 33,14, 34,17];
const R10 = [17, 4,2, 7,6, 8,9, 10,11, 24,12, 25,14, 29,15, 30,18];
const R11 = [5,  17,11, 28,9];
const R12 = [5,  17,7, 27,9];
const R13 = [17, 7,2, 8,6, 10,9, 25,12, 25,7, 29,14, 30,15, 31,18];
const R14 = [19, 6,1, 8,4, 10,8, 27,5, 27,11, 31,12, 32,14, 33,17, 12,9];
const R15 = [7,  14,9, 15,6, 25,8];
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
  ...Array.from({ length: 19 }, (_, i) => `river${i}`),
  'river20',
];

// Build a set of all river tile positions (col,row) for walker avoidance
function buildRiverSet(): Set<string> {
  const s = new Set<string>();
  const addPts = (pts: number[]) => {
    for (let i = 1; i < pts[0]; i += 2) s.add(`${pts[i]},${pts[i + 1]}`);
  };
  addPts(R0); addPts(R2); addPts(R4);  addPts(R6);
  addPts(R8); addPts(R9); addPts(R10); addPts(R11);
  addPts(R12); addPts(R13); addPts(R14); addPts(R15); addPts(R16);
  for (let col = 17; col <= 25; col++)
    for (let row = 5; row <= 6; row++)
      s.add(`${col},${row}`);
  return s;
}

const RIVER_SET = buildRiverSet();

function isRiverPx(px: number, py: number, mapW: number, mapH: number): boolean {
  if (py < 0 || py >= mapH) return false;
  const col = Math.floor(((px % mapW) + mapW) % mapW / TILE);
  const row = Math.floor(py / TILE);
  return RIVER_SET.has(`${col},${row}`);
}

// ── Types ────────────────────────────────────────────────────────────────────

interface FaceSprite { face: ImageBitmap | null; mask: ImageBitmap | null; }

interface LetterSprite {
  bm: ImageBitmap | null;
  w: number; h: number;
  targetX: number; targetY: number; x: number;
  extraY: number; done: boolean;
}

interface Walker {
  px: number; py: number;
  dx: number; dy: number;
  sprite: FaceSprite;
  nextDir: number;
}

// ── Helpers ──────────────────────────────────────────────────────────────────

const ALL_DIRS = [{dx:1,dy:0},{dx:-1,dy:0},{dx:0,dy:1},{dx:0,dy:-1}];

function randDir(): { dx: number; dy: number } {
  return ALL_DIRS[Math.floor(Math.random() * 4)];
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
        if (id.data[i] >= 200) { id.data[i + 3] = 0; }
        else { id.data[i] = 0; id.data[i+1] = 0; id.data[i+2] = 0; id.data[i+3] = 255; }
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

function drawRiverSection(ctx: OffscreenCanvasRenderingContext2D, pts: number[], tiles: Map<string, ImageBitmap | null>, tileA: string, tileB: string): void {
  for (let i = 1; i < pts[0]; i += 2) {
    const bm = tiles.get(rn2() === 0 ? tileA : tileB);
    if (bm) ctx.drawImage(bm, pts[i] * TILE, pts[i + 1] * TILE, TILE, TILE);
  }
}

function drawRiverFixed(ctx: OffscreenCanvasRenderingContext2D, pts: number[], tiles: Map<string, ImageBitmap | null>, tileName: string): void {
  for (let i = 1; i < pts[0]; i += 2) {
    const bm = tiles.get(tileName);
    if (bm) ctx.drawImage(bm, pts[i] * TILE, pts[i + 1] * TILE, TILE, TILE);
  }
}

async function buildTerrainMap(tiles: Map<string, ImageBitmap | null>): Promise<OffscreenCanvas> {
  const W = 40 * TILE;
  const H = 20 * TILE;
  const oc = new OffscreenCanvas(W, H);
  const ctx = oc.getContext('2d')!;
  ctx.fillStyle = '#fff';
  ctx.fillRect(0, 0, W, H);

  const t = (name: string) => tiles.get(name) ?? null;

  for (let row = 0; row <= 19; row++) {
    const [cs, ce] = BEACH_RANGES[row];
    for (let col = cs; col <= ce; col++) {
      const bm = t(`beach${Math.floor(Math.random() * 3)}`);
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }
  }
  for (let col = 0; col <= 16; col++)
    for (let row = 0; row <= 4; row++) { const bm = t('forest'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (let row = 5; row <= 9; row++) {
    const [cs, ce] = FOREST1_RANGES[row - 5];
    for (let col = cs; col <= ce; col++) { const bm = t('forest'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  }
  for (let row = 8; row <= 19; row++) {
    const [cs, ce] = FOREST2_RANGES[row - 8];
    for (let col = cs; col <= ce; col++) { const bm = t('forest'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  }
  for (let row = 3; row <= 16; row++) {
    const [cs, ce] = FOREST3_RANGES[row - 3];
    for (let col = cs; col <= ce; col++) { const bm = t('forest'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  }
  for (let row = 3; row <= 12; row++) {
    const [cs, ce] = SAND1_RANGES[row - 3];
    for (let col = cs; col <= ce; col++) { const bm = t('sand'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  }
  for (let row = 8; row <= 14; row++) {
    const [cs, ce] = SAND2_RANGES[row - 8];
    for (let col = cs; col <= ce; col++) { const bm = t('sand'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  }
  for (const [row, cs, ce] of HPATHS)
    for (let col = cs; col <= ce; col++) { const bm = t('hpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (const [col, rs, re] of VPATHS)
    for (let row = rs; row <= re; row++) { const bm = t('vpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (const [col, row] of LR_CORNERS) { const bm = t('lrpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (const [col, row] of LL_CORNERS) { const bm = t('llpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (const [col, row] of UR_CORNERS) { const bm = t('urpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }
  for (const [col, row] of UL_CORNERS) { const bm = t('ulpath'); if (bm) ctx.drawImage(bm, col*TILE, row*TILE, TILE, TILE); }

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
  for (let col = 17; col <= 25; col++)
    for (let row = 5; row <= 6; row++) {
      const bm = t(`river${17 + rn2()}`);
      if (bm) ctx.drawImage(bm, col * TILE, row * TILE, TILE, TILE);
    }

  // Invert for dark mode
  const id = ctx.getImageData(0, 0, W, H);
  const d = id.data;
  for (let i = 0; i < d.length; i += 4) {
    d[i] = 255 - d[i]; d[i+1] = 255 - d[i+1]; d[i+2] = 255 - d[i+2];
  }
  ctx.putImageData(id, 0, 0);

  return oc;
}

// ── Lobby logo ───────────────────────────────────────────────────────────────

export async function drawLogo(canvas: HTMLCanvasElement): Promise<void> {
  const rawLetters = await Promise.all(
    LETTER_NAMES.map((n) => loadBitmap(`/sprites/bitmaps/${n}.png`)),
  );
  const ctx = canvas.getContext('2d')!;
  const W = canvas.width;
  const H = canvas.height;

  let scale = 1;
  for (const bm of rawLetters)
    if (bm && bm.height > 0) scale = Math.min(scale, H / bm.height);

  const totalW = rawLetters.reduce((s, bm) => s + Math.floor((bm?.width ?? 48) * scale) + LETTER_PAD, -LETTER_PAD);
  if (totalW > W - 20) scale *= (W - 20) / totalW;

  const finalW = rawLetters.reduce((s, bm) => s + Math.floor((bm?.width ?? 48) * scale) + LETTER_PAD, -LETTER_PAD);
  let x = Math.floor((W - finalW) / 2);
  const tallest = rawLetters.reduce((h, bm) => Math.max(h, bm ? Math.floor(bm.height * scale) : 0), 0);
  const baseY = Math.floor((H - tallest) / 2);

  ctx.clearRect(0, 0, W, H);
  for (let i = 0; i < rawLetters.length; i++) {
    const bm = rawLetters[i];
    if (!bm) continue;
    const w = Math.floor(bm.width * scale);
    const h = Math.floor(bm.height * scale);
    const extraY = LETTER_NAMES[i] === 'j' ? Math.floor(h * 0.2) : 0;
    ctx.drawImage(bm, x, baseY + extraY, w, h);
    x += w + LETTER_PAD;
  }
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

  const topH    = Math.floor(H / 4);
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

  const [rawLetters, terrainBitmaps, faceSprites] = await Promise.all([
    Promise.all(LETTER_NAMES.map((n) => loadBitmap(`/sprites/bitmaps/${n}.png`))),
    Promise.all(TERRAIN_NAMES.map((n) => loadRawBitmap(`/sprites/bitmaps/${n}.png`))),
    Promise.all(AVATARS.map((n) => loadFaceSprite(n))),
  ]);

  if (dismissed) return done;

  const tileMap = new Map<string, ImageBitmap | null>(TERRAIN_NAMES.map((n, i) => [n, terrainBitmaps[i]]));
  const mapCanvas = await buildTerrainMap(tileMap);
  const mapW = mapCanvas.width;  // 1280
  const mapH = mapCanvas.height; // 640

  if (dismissed) return done;

  // Terrain viewport: centered, with margins, no scrolling
  const vpW  = Math.min(W - 2 * MARGIN_X, mapW);
  const vpH  = Math.min(bottomH - MARGIN_BOTTOM - MARGIN_TOP, mapH);
  const vpX  = Math.floor((W - vpW) / 2);
  const vpY  = bottomY + MARGIN_TOP;
  // Fixed camera offset: show the center of the scene
  const camX = Math.max(0, Math.floor((mapW - vpW) / 2));
  const camY = 0;

  // ── Letter layout ────────────────────────────────────────────────
  const PROMPT_FONT_SIZE = 13;
  const PROMPT_MARGIN = 8;
  const promptY = topH - PROMPT_MARGIN;
  const usableLetterH = promptY - PROMPT_FONT_SIZE - PROMPT_MARGIN * 2;

  let scale = 1;
  for (const bm of rawLetters)
    if (bm && bm.height > 0) scale = Math.min(scale, usableLetterH / bm.height);

  const totalLetterW = rawLetters.reduce((s, bm) => s + Math.floor((bm?.width ?? 48) * scale) + LETTER_PAD, -LETTER_PAD);
  if (totalLetterW > W - 40) scale *= (W - 40) / totalLetterW;

  const finalTotalW = rawLetters.reduce((s, bm) => s + Math.floor((bm?.width ?? 48) * scale) + LETTER_PAD, -LETTER_PAD);
  let xCursor = Math.max(20, Math.floor((W - finalTotalW) / 2));
  const tallestLetter = rawLetters.reduce((h, bm) => Math.max(h, bm ? Math.floor(bm.height * scale) : 0), 0);
  const letterBaseY = Math.max(0, Math.floor((usableLetterH - tallestLetter) / 2));

  const letterSprites: LetterSprite[] = rawLetters.map((bm, i) => {
    const w = bm ? Math.floor(bm.width * scale) : 48;
    const h = bm ? Math.floor(bm.height * scale) : 64;
    const sprite: LetterSprite = {
      bm, w, h,
      targetX: xCursor, targetY: letterBaseY,
      x: bm ? -w - 10 : xCursor,
      extraY: LETTER_NAMES[i] === 'j' ? Math.floor(h * 0.2) : 0,
      done: !bm,
    };
    xCursor += w + LETTER_PAD;
    return sprite;
  });

  let allLettersDone = false;
  let startedCount = 1;

  // ── Walkers — spawn only on non-river tiles ───────────────────────
  const walkers: Walker[] = Array.from({ length: AVATARS.length }, (_, i) => {
    let px: number, py: number;
    do {
      px = Math.floor(Math.random() * mapW);
      py = Math.floor(Math.random() * mapH);
    } while (isRiverPx(px, py, mapW, mapH));
    const d = randDir();
    return { px, py, dx: d.dx, dy: d.dy, sprite: faceSprites[i], nextDir: Math.floor(Math.random() * 120) + 30 };
  });

  function isOccupied(px: number, py: number, self: Walker): boolean {
    if (py < 0 || py >= mapH) return false;
    const col = Math.floor(((px % mapW) + mapW) % mapW / TILE);
    const row = Math.floor(py / TILE);
    for (const other of walkers) {
      if (other === self) continue;
      if (other.py < 0 || other.py >= mapH) continue;
      const oc = Math.floor(((other.px % mapW) + mapW) % mapW / TILE);
      const or_ = Math.floor(other.py / TILE);
      if (oc === col && or_ === row) return true;
    }
    return false;
  }

  let frame = 0;

  function loop() {
    if (dismissed) return;
    animId = requestAnimationFrame(loop);
    frame++;

    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, W, H);

    // ── Top band: letters + prompt ───────────────────────────────
    ctx.save();
    ctx.beginPath(); ctx.rect(0, 0, W, topH); ctx.clip();

    if (!allLettersDone) {
      for (let i = 0; i < startedCount && i < letterSprites.length; i++) {
        const s = letterSprites[i];
        if (!s.done) { s.x += 10; if (s.x >= s.targetX) { s.x = s.targetX; s.done = true; } }
      }
      if (startedCount < letterSprites.length) {
        const last = letterSprites[startedCount - 1];
        const totalDist = last.targetX - (-last.w - 10);
        const traveled  = last.x       - (-last.w - 10);
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

    // ── Bottom band: fixed terrain + walkers ─────────────────────
    ctx.save();
    ctx.beginPath(); ctx.rect(vpX, vpY, vpW, vpH); ctx.clip();

    ctx.drawImage(mapCanvas, camX, camY, vpW, vpH, vpX, vpY, vpW, vpH);

    for (const w of walkers) {
      // Periodic random direction change
      if (frame >= w.nextDir) {
        w.dx = randDir().dx; w.dy = randDir().dy;
        w.nextDir = frame + Math.floor(Math.random() * 120) + 60;
      }

      // Try to move; redirect if next tile is river or occupied by another walker
      const nextPx = w.px + w.dx;
      const nextPy = w.py + w.dy;
      if (isRiverPx(nextPx, nextPy, mapW, mapH) || isOccupied(nextPx, nextPy, w)) {
        // Pick a clear direction
        const shuffled = [...ALL_DIRS].sort(() => Math.random() - 0.5);
        for (const d of shuffled) {
          if (!isRiverPx(w.px + d.dx, w.py + d.dy, mapW, mapH) && !isOccupied(w.px + d.dx, w.py + d.dy, w)) {
            w.dx = d.dx; w.dy = d.dy;
            break;
          }
        }
        // Don't move this frame — stay put
      } else {
        w.px = nextPx;
        w.py = nextPy;
        // Lazy wrap: only once fully off-screen by a tile, so edges clip smoothly
        if (w.px < -TILE)          w.px = mapW;
        else if (w.px >= mapW + TILE) w.px = -TILE;
        if (w.py < -TILE)          w.py = mapH;
        else if (w.py >= mapH + TILE) w.py = -TILE;
      }

      const sx = vpX + (w.px - camX);
      const sy = vpY  + (w.py - camY);
      if (sx > vpX - TILE && sx < vpX + vpW + TILE && sy > vpY - TILE && sy < vpY + vpH + TILE)
        drawFace(ctx, w.sprite, sx, sy, TILE);
    }

    ctx.restore();

  }

  loop();
  return done;
}
