import { describe, it, expect } from 'vitest';
import { applyHpPenalty, stepDelay } from '../utils';

describe('applyHpPenalty', () => {
  it('full HP applies no penalty', () => {
    const base = stepDelay(9); // 150ms
    expect(applyHpPenalty(base, 100, 100)).toBe(base);
  });

  it('50% HP doubles the delay', () => {
    const base = stepDelay(9); // 150ms
    expect(applyHpPenalty(base, 50, 100)).toBe(base * 2); // 300ms
  });

  it('25% HP quadruples the delay, capped at stepDelay(1)', () => {
    const base = stepDelay(9); // 150ms
    const uncapped = base * 4; // 600ms
    const cap = stepDelay(1); // 1350ms
    // 600ms is below the cap so it should pass through
    expect(applyHpPenalty(base, 25, 100)).toBe(Math.min(cap, uncapped));
  });

  it('very low HP is capped at stepDelay(1)', () => {
    const base = stepDelay(9); // 150ms
    // 1% HP → 100× base = 15000ms, capped at 1350ms
    expect(applyHpPenalty(base, 1, 100)).toBe(stepDelay(1));
  });

  it('hp equal to maxHp (any value) applies no penalty', () => {
    const base = stepDelay(5); // 270ms
    expect(applyHpPenalty(base, 75, 75)).toBe(base);
  });

  it('handles hp=0 without divide-by-zero (clamps to stepDelay(1))', () => {
    const base = stepDelay(9);
    // hp=0 would divide by zero without the Math.max(1, hp) guard
    expect(applyHpPenalty(base, 0, 100)).toBe(stepDelay(1));
  });

  it('handles maxHp=0 without divide-by-zero', () => {
    const base = stepDelay(9);
    expect(() => applyHpPenalty(base, 0, 0)).not.toThrow();
  });

  it('result is always an integer (Math.round)', () => {
    // hp=33 → fraction = 33/100 → base/fraction may be non-integer
    const base = stepDelay(9); // 150ms
    const result = applyHpPenalty(base, 33, 100);
    expect(result).toBe(Math.round(result));
  });
});
