import { ObjDef, InventoryItem } from './types';

let el: HTMLElement | null = null;

function getEl(): HTMLElement {
  if (!el) el = document.getElementById('tooltip')!;
  return el;
}

export function showTooltip(html: string, x: number, y: number): void {
  const tip = getEl();
  tip.innerHTML = html;
  tip.style.display = 'block';
  positionTooltip(x, y);
}

export function moveTooltip(x: number, y: number): void {
  if (getEl().style.display === 'none') return;
  positionTooltip(x, y);
}

export function hideTooltip(): void {
  getEl().style.display = 'none';
}

function positionTooltip(x: number, y: number): void {
  const tip = getEl();
  const margin = 8;
  const ox = 14, oy = 14;
  tip.style.left = '0';
  tip.style.top  = '0';
  const tw = tip.offsetWidth;
  const th = tip.offsetHeight;
  const vw = window.innerWidth;
  const vh = window.innerHeight;
  const lx = (x + ox + tw > vw - margin) ? x - ox - tw : x + ox;
  const ly = (y + oy + th > vh - margin) ? y - oy - th : y + oy;
  tip.style.left = `${lx}px`;
  tip.style.top  = `${ly}px`;
}

export function buildItemHtml(obj: ObjDef, item: InventoryItem): string {
  const rows: string[] = [];

  const name = obj.name ?? `Object #${obj._index}`;
  rows.push(`<div class="tip-name">${name}</div>`);

  if (obj.weapon) {
    const parts: string[] = ['Weapon'];
    if (obj.damage != null) parts.push(`dmg ${obj.damage}`);
    if (obj.range  != null) parts.push(`range ${obj.range}`);
    if (obj.speed  != null) parts.push(`spd ${obj.speed}`);
    rows.push(`<div class="tip-row">${parts.join(' · ')}</div>`);
  }

  if (obj.weight != null) {
    rows.push(`<div class="tip-row"><span class="tip-lbl">Weight</span> ${obj.weight}</div>`);
  }

  if (obj.capacity != null) {
    rows.push(`<div class="tip-row"><span class="tip-lbl">Capacity</span> ${obj.capacity}</div>`);
  }

  if (obj.movement != null && obj.movement > 0 && obj.movement !== 9) {
    const delay = Math.max(50, Math.round(150 * 9 / obj.movement));
    rows.push(`<div class="tip-row"><span class="tip-lbl">Move</span> ${delay} ms/step</div>`);
  }

  if (item.quantity > 1 || obj.numbered) {
    rows.push(`<div class="tip-row"><span class="tip-lbl">Qty</span> ${item.quantity}</div>`);
  }

  return rows.join('');
}
