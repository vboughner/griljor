import { describe, it, expect } from 'vitest';
import { calcFireCooldown } from '../session';

describe('calcFireCooldown', () => {
  it('refire=0 (default) gives 850ms', () => {
    expect(calcFireCooldown(0)).toBe(850);
  });

  it('undefined refire defaults to 0 → 850ms', () => {
    expect(calcFireCooldown(undefined)).toBe(850);
  });

  it('refire=5 (maximum fast) gives 0ms', () => {
    expect(calcFireCooldown(5)).toBe(0);
  });

  it('refire=-5 (maximum slow) gives 1700ms', () => {
    expect(calcFireCooldown(-5)).toBe(1700);
  });

  it('refire=1 gives 680ms', () => {
    // 850 * (1 - 1/5) = 850 * 0.8 = 680
    expect(calcFireCooldown(1)).toBe(680);
  });

  it('refire=-1 gives 1020ms', () => {
    // 850 * (1 - (-1)/5) = 850 * 1.2 = 1020
    expect(calcFireCooldown(-1)).toBe(1020);
  });

  it('refire=2 gives 510ms', () => {
    // 850 * (1 - 2/5) = 850 * 0.6 = 510
    expect(calcFireCooldown(2)).toBe(510);
  });

  it('clamps refire above 5 to 5', () => {
    expect(calcFireCooldown(10)).toBe(calcFireCooldown(5));
  });

  it('clamps refire below -5 to -5', () => {
    expect(calcFireCooldown(-10)).toBe(calcFireCooldown(-5));
  });

  it('object-data unsigned byte convention: 255 = signed -1 → ~1020ms', () => {
    // In the raw object binary, negative refire values are stored as unsigned bytes
    // (e.g. 255 = -1). The pipeline emits them as unsigned; callers must sign-extend
    // before passing to calcFireCooldown. This test documents the intended usage.
    const signed = 255 - 256; // -1
    expect(calcFireCooldown(signed)).toBe(1020);
  });
});
