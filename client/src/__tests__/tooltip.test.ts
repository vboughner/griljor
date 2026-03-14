import { describe, it, expect } from 'vitest';
import { buildItemHtml } from '../tooltip';
import { ObjDef, InventoryItem } from '../types';

function makeObj(overrides: Partial<ObjDef> = {}): ObjDef {
  return { _index: 1, name: 'Test Item', ...overrides } as ObjDef;
}
function makeItem(quantity = 1): InventoryItem {
  return { type: 1, quantity };
}

describe('buildItemHtml', () => {
  it('includes item name', () => {
    const html = buildItemHtml(makeObj({ name: 'Sword' }), makeItem());
    expect(html).toContain('Sword');
  });

  it('falls back to "Object #N" when name is missing', () => {
    const html = buildItemHtml(makeObj({ name: undefined, _index: 42 }), makeItem());
    expect(html).toContain('Object #42');
  });

  it('shows weapon row when weapon=true', () => {
    const html = buildItemHtml(makeObj({ weapon: true, damage: 15, range: 3 }), makeItem());
    expect(html).toContain('Weapon');
    expect(html).toContain('dmg 15');
    expect(html).toContain('range 3');
  });

  it('omits damage/range from weapon row when undefined', () => {
    const html = buildItemHtml(makeObj({ weapon: true }), makeItem());
    expect(html).toContain('Weapon');
    expect(html).not.toContain('dmg');
    expect(html).not.toContain('range');
  });

  it('shows weight when present', () => {
    const html = buildItemHtml(makeObj({ weight: 12 }), makeItem());
    expect(html).toContain('Weight');
    expect(html).toContain('12');
  });

  it('shows capacity when present', () => {
    const html = buildItemHtml(makeObj({ capacity: 50 }), makeItem());
    expect(html).toContain('Capacity');
  });

  it('shows heal amount for negative health value', () => {
    const html = buildItemHtml(makeObj({ health: -25 }), makeItem());
    expect(html).toContain('Heals');
    expect(html).toContain('25 HP');
  });

  it('does not show heal row for positive health value', () => {
    const html = buildItemHtml(makeObj({ health: 10 }), makeItem());
    expect(html).not.toContain('Heals');
  });

  it('shows qty when quantity > 1', () => {
    const html = buildItemHtml(makeObj(), makeItem(5));
    expect(html).toContain('Qty');
    expect(html).toContain('5');
  });

  it('shows qty when item is numbered (even quantity=1)', () => {
    const html = buildItemHtml(makeObj({ numbered: true }), makeItem(1));
    expect(html).toContain('Qty');
  });

  it('does not show qty for non-numbered single item', () => {
    const html = buildItemHtml(makeObj({ numbered: false }), makeItem(1));
    expect(html).not.toContain('Qty');
  });
});
