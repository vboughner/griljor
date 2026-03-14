import { describe, it, expect } from 'vitest';
import { stepDelay, formatAge } from '../utils';

describe('stepDelay', () => {
  it('speed 9 returns 150ms', () => {
    expect(stepDelay(9)).toBe(150);
  });

  it('speed 1 returns 1350ms', () => {
    expect(stepDelay(1)).toBe(1350);
  });

  it('speed 5 returns 270ms', () => {
    expect(stepDelay(5)).toBe(270);
  });

  it('speed 0 does not divide by zero (clamps to spd=1)', () => {
    expect(stepDelay(0)).toBe(1350);
  });

  it('speed below 1 uses minimum denominator', () => {
    expect(stepDelay(-5)).toBe(1350);
  });

  it('returns minimum 50ms for very high speed values', () => {
    expect(stepDelay(1000)).toBe(50);
  });
});

describe('formatAge', () => {
  it('under 45s → "just joined"', () => {
    expect(formatAge(30_000)).toBe('just joined');
  });

  it('45s → "a minute"', () => {
    expect(formatAge(45_000)).toBe('a minute');
  });

  it('90s → starts "2 minutes"', () => {
    expect(formatAge(90_000)).toBe('2 minutes');
  });

  it('44 minutes → "44 minutes"', () => {
    expect(formatAge(44 * 60_000)).toBe('44 minutes');
  });

  it('45 minutes → "an hour"', () => {
    expect(formatAge(45 * 60_000)).toBe('an hour');
  });

  it('90 minutes → starts "2 hours"', () => {
    expect(formatAge(90 * 60_000)).toBe('2 hours');
  });

  it('22 hours → "a day"', () => {
    expect(formatAge(22 * 3_600_000)).toBe('a day');
  });

  it('36 hours → "2 days"', () => {
    expect(formatAge(36 * 3_600_000)).toBe('2 days');
  });
});
