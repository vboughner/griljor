import { describe, it, expect } from 'vitest';
import { effectiveLightRadius } from '../session';
import { ObjDef } from '../world';
import { InventoryItem } from '../protocol';

const BASE_DARK_RADIUS = 2;

function makeObj(overrides: Partial<ObjDef> = {}): ObjDef {
  return { _index: 1, ...overrides } as ObjDef;
}

describe('effectiveLightRadius', () => {
  it('returns BASE_DARK_RADIUS with no items', () => {
    expect(effectiveLightRadius(null, [], [])).toBe(BASE_DARK_RADIUS);
  });

  it('returns BASE_DARK_RADIUS when no items have flashlight', () => {
    const objects: Array<ObjDef | null> = [null, makeObj({ name: 'sword' })];
    const inv: Array<InventoryItem | null> = [{ type: 1, quantity: 1 }, null];
    expect(effectiveLightRadius(null, inv, objects)).toBe(BASE_DARK_RADIUS);
  });

  it('returns flashlight value from hand item', () => {
    const objects: Array<ObjDef | null> = [null, makeObj({ flashlight: 5 })];
    const hand: InventoryItem = { type: 1, quantity: 1 };
    expect(effectiveLightRadius(hand, [], objects)).toBe(5);
  });

  it('returns flashlight value from inventory item', () => {
    const objects: Array<ObjDef | null> = [null, makeObj({ flashlight: 6 })];
    const inv: Array<InventoryItem | null> = [{ type: 1, quantity: 1 }];
    expect(effectiveLightRadius(null, inv, objects)).toBe(6);
  });

  it('uses the best flashlight across hand and inventory', () => {
    const objects: Array<ObjDef | null> = [
      null,
      makeObj({ flashlight: 3 }), // hand
      makeObj({ flashlight: 7 }), // inventory
      makeObj({ flashlight: 5 }), // inventory
    ];
    const hand: InventoryItem = { type: 1, quantity: 1 };
    const inv: Array<InventoryItem | null> = [
      { type: 2, quantity: 1 },
      { type: 3, quantity: 1 },
    ];
    expect(effectiveLightRadius(hand, inv, objects)).toBe(7);
  });

  it('skips null inventory slots', () => {
    const objects: Array<ObjDef | null> = [null, makeObj({ flashlight: 4 })];
    const inv: Array<InventoryItem | null> = [null, null, { type: 1, quantity: 1 }, null];
    expect(effectiveLightRadius(null, inv, objects)).toBe(4);
  });
});
