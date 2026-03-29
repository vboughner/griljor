import movexbm from '../../pipeline/bit/movemark?raw';

const SLOT_W = 32;
const SLOT_H = 32;

function parseXbmBytes(raw: string): number[] {
  const match = raw.match(/\{([\s\S]*)\}/);
  if (!match) throw new Error('Invalid XBM');
  return match[1]
    .split(',')
    .map((s) => s.trim())
    .filter((s) => /^0x/i.test(s))
    .map((s) => parseInt(s, 16));
}

function drawMoveIcon(canvas: HTMLCanvasElement): void {
  const bytes = parseXbmBytes(movexbm);
  const ctx = canvas.getContext('2d')!;
  const img = ctx.createImageData(SLOT_W, SLOT_H);
  for (let y = 0; y < SLOT_H; y++) {
    for (let x = 0; x < SLOT_W; x++) {
      const idx = y * SLOT_W + x;
      const isSet = (bytes[idx >> 3] >> (idx & 7)) & 1;
      const p = idx * 4;
      img.data[p] = 150;
      img.data[p + 1] = 200;
      img.data[p + 2] = 255;
      img.data[p + 3] = isSet ? 0 : 200;
    }
  }
  ctx.putImageData(img, 0, 0);
}

export function initActionCards(): void {
  drawMoveIcon(document.getElementById('move-icon') as HTMLCanvasElement);
}

export function setActiveItem(imgData: ImageData | null, name: string | null): void {
  const canvas = document.getElementById('active-item-icon') as HTMLCanvasElement;
  const nameEl = document.getElementById('active-item-name') as HTMLElement;
  const ctx = canvas.getContext('2d')!;
  ctx.clearRect(0, 0, SLOT_W, SLOT_H);
  if (imgData) {
    const tmp = new OffscreenCanvas(imgData.width, imgData.height);
    tmp.getContext('2d')!.putImageData(imgData, 0, 0);
    ctx.drawImage(tmp, 0, 0, SLOT_W, SLOT_H);
  }
  nameEl.textContent = name ?? 'nothing';
}
