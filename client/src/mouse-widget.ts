const SLOT_W = 32;
const SLOT_H = 32;

function drawMoveIcon(canvas: HTMLCanvasElement): void {
  const ctx = canvas.getContext('2d')!;
  const cx = SLOT_W / 2;
  const cy = SLOT_H / 2;
  const color = '#d2aa3c';
  const dimColor = '#8a7028';

  ctx.clearRect(0, 0, SLOT_W, SLOT_H);

  // Draw an arrow from center outward
  function drawArrow(angle: number, len: number, dim: boolean): void {
    const rad = (angle * Math.PI) / 180;
    const ex = cx + Math.cos(rad) * len;
    const ey = cy + Math.sin(rad) * len;
    const headLen = 4;
    ctx.strokeStyle = dim ? dimColor : color;
    ctx.lineWidth = dim ? 1 : 1.5;
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(ex, ey);
    // Arrowhead
    ctx.lineTo(ex - headLen * Math.cos(rad - 0.4), ey - headLen * Math.sin(rad - 0.4));
    ctx.moveTo(ex, ey);
    ctx.lineTo(ex - headLen * Math.cos(rad + 0.4), ey - headLen * Math.sin(rad + 0.4));
    ctx.stroke();
  }

  // Cardinal directions (N, E, S, W) — brighter, longer
  for (const angle of [270, 0, 90, 180]) {
    drawArrow(angle, 13, false);
  }
  // Diagonal directions (NE, SE, SW, NW) — dimmer, shorter
  for (const angle of [315, 45, 135, 225]) {
    drawArrow(angle, 10, true);
  }

  // Center dot
  ctx.fillStyle = color;
  ctx.beginPath();
  ctx.arc(cx, cy, 1.5, 0, Math.PI * 2);
  ctx.fill();
}

export function initActionCards(): void {
  drawMoveIcon(document.getElementById('move-icon') as HTMLCanvasElement);
}

export function setActiveItem(
  imgData: ImageData | null,
  name: string | null,
  count: number | null,
): void {
  const canvas = document.getElementById('active-item-icon') as HTMLCanvasElement;
  const nameEl = document.getElementById('active-item-name') as HTMLElement;
  const countEl = document.getElementById('active-item-count') as HTMLElement;
  const ctx = canvas.getContext('2d')!;
  ctx.clearRect(0, 0, SLOT_W, SLOT_H);
  if (imgData) {
    const tmp = new OffscreenCanvas(imgData.width, imgData.height);
    tmp.getContext('2d')!.putImageData(imgData, 0, 0);
    ctx.drawImage(tmp, 0, 0, SLOT_W, SLOT_H);
  }
  nameEl.textContent = name ?? 'holding nothing';
  countEl.textContent = count !== null ? String(count) : '';
}
