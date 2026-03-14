import { describe, it, expect } from 'vitest';
import { calcItemWeight } from '../session';
import { ObjDef } from '../world';
import { InventoryItem } from '../protocol';

function makeObj(overrides: Partial<ObjDef> = {}): ObjDef {
  return { _index: 1, weight: 10, ...overrides } as ObjDef;
}
function makeItem(quantity: number): InventoryItem {
  return { type: 1, quantity };
}

describe('calcItemWeight', () => {
  it('returns 0 for null obj', () => {
    expect(calcItemWeight(null, makeItem(5))).toBe(0);
  });

  it('returns 0 for undefined obj', () => {
    expect(calcItemWeight(undefined, makeItem(5))).toBe(0);
  });

  it('returns weight × quantity for non-numbered stackable items', () => {
    const obj = makeObj({ weight: 5, numbered: false });
    expect(calcItemWeight(obj, makeItem(3))).toBe(15);
  });

  it('returns flat weight for numbered items regardless of quantity', () => {
    const obj = makeObj({ weight: 20, numbered: true });
    expect(calcItemWeight(obj, makeItem(10))).toBe(20);
  });

  it('treats missing weight as 0', () => {
    const obj = makeObj({ weight: undefined, numbered: false });
    expect(calcItemWeight(obj, makeItem(5))).toBe(0);
  });

  it('single quantity stackable: weight × 1 = weight', () => {
    const obj = makeObj({ weight: 7, numbered: false });
    expect(calcItemWeight(obj, makeItem(1))).toBe(7);
  });
});
