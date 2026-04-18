import { describe, it, expect } from 'vitest';
import { arcDirections } from '../session';

describe('arcDirections', () => {
  it('returns a single direction when spreadCount is 1', () => {
    const dirs = arcDirections(0, 22, 1, 5, 10, 10);
    expect(dirs.length).toBe(1);
    // Center angle 0 = north (dy negative)
    expect(dirs[0].dy).toBe(-1);
  });

  it('returns multiple directions for a spread of 3', () => {
    // Firing north (angle=0) with 22-degree arc, 3 projectiles
    const dirs = arcDirections(0, 22, 3, 5, 10, 10);
    expect(dirs.length).toBeGreaterThanOrEqual(2); // at least 2 unique after dedup
    expect(dirs.length).toBeLessThanOrEqual(3);
  });

  it('fires in a wider cone with larger arc', () => {
    // 180-degree arc should produce wider spread than 22-degree arc
    const narrow = arcDirections(0, 22, 5, 5, 10, 10);
    const wide = arcDirections(0, 180, 5, 5, 10, 10);
    // Get the x-range of targets for each
    const narrowXRange =
      Math.max(...narrow.map((d) => d.targetX)) - Math.min(...narrow.map((d) => d.targetX));
    const wideXRange =
      Math.max(...wide.map((d) => d.targetX)) - Math.min(...wide.map((d) => d.targetX));
    expect(wideXRange).toBeGreaterThan(narrowXRange);
  });

  it('deduplicates targets that round to the same tile', () => {
    // Very narrow arc at short range — some projectiles may land on the same tile
    const dirs = arcDirections(0, 5, 10, 2, 10, 10);
    const keys = dirs.map((d) => `${d.targetX},${d.targetY}`);
    expect(keys.length).toBe(new Set(keys).size);
  });

  it('generates directions relative to the origin position', () => {
    const dirs = arcDirections(Math.PI / 2, 22, 3, 5, 5, 5); // firing east
    // All targets should be east of origin (targetX > originX)
    for (const d of dirs) {
      expect(d.targetX).toBeGreaterThan(5);
    }
  });
});
