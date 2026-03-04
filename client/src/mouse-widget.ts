import mousexbm from '../../bit/mouse?raw';
import movexbm from '../../bit/movemark?raw';

// XBM slot geometry (derived from original MOUSE_START_X/Y and bitmap hole analysis)
// Each hole is 32px wide; the 4×0x00 bytes per row give exactly 32 transparent pixels.
const SLOT_Y = 18;
const SLOT_W = 32;
const SLOT_H = 32;
const SLOT_X = [8, 48, 88]; // left-hand, right-hand, movement

function parseXbmBytes(raw: string): number[] {
  const match = raw.match(/\{([\s\S]*)\}/);
  if (!match) throw new Error('Invalid XBM');
  return match[1]
    .split(',')
    .map(s => s.trim())
    .filter(s => /^0x/i.test(s))
    .map(s => parseInt(s, 16));
}

function renderXbm(canvas: HTMLCanvasElement): void {
  const bytes = parseXbmBytes(mousexbm);
  const W = 128, H = 128;
  const ctx = canvas.getContext('2d')!;
  const img = ctx.createImageData(W, H);
  for (let y = 0; y < H; y++) {
    for (let x = 0; x < W; x++) {
      const idx = y * W + x;
      const isSet = (bytes[idx >> 3] >> (idx & 7)) & 1;
      const p = idx * 4;
      img.data[p]     = 255;
      img.data[p + 1] = 255;
      img.data[p + 2] = 255;
      img.data[p + 3] = isSet ? 255 : 0;
    }
  }
  ctx.putImageData(img, 0, 0);
}

function drawMovementSlot(canvas: HTMLCanvasElement): void {
  const bytes = parseXbmBytes(movexbm);
  const ctx = canvas.getContext('2d')!;
  const img = ctx.createImageData(SLOT_W, SLOT_H);
  for (let y = 0; y < SLOT_H; y++) {
    for (let x = 0; x < SLOT_W; x++) {
      const idx = y * SLOT_W + x;
      const isSet = (bytes[idx >> 3] >> (idx & 7)) & 1;
      const p = idx * 4;
      img.data[p]     = 255;
      img.data[p + 1] = 255;
      img.data[p + 2] = 255;
      img.data[p + 3] = isSet ? 0 : 255;
    }
  }
  ctx.putImageData(img, 0, 0);
}

export function initMouseWidget(): void {
  const bitmap = document.getElementById('mouse-bitmap') as HTMLCanvasElement;
  renderXbm(bitmap);
  drawMovementSlot(document.getElementById('hand-right-canvas') as HTMLCanvasElement);
}

export function setHandItem(slot: 'left' | 'right', label: string | null): void {
  const id = slot === 'left' ? 'hand-left-canvas' : 'hand-middle-canvas';
  const canvas = document.getElementById(id) as HTMLCanvasElement;
  const ctx = canvas.getContext('2d')!;
  ctx.clearRect(0, 0, SLOT_W, SLOT_H);
  if (label) {
    ctx.fillStyle = '#aaa';
    ctx.font = '10px monospace';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(label, SLOT_W / 2, SLOT_H / 2);
  }
}

export { SLOT_X, SLOT_Y, SLOT_W, SLOT_H };
