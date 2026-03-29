import { describe, it, expect } from 'vitest';
import { readFileSync } from 'fs';
import { resolve } from 'path';

const src = readFileSync(resolve(__dirname, '../game.ts'), 'utf-8');

describe('key bindings', () => {
  it('maps s to south movement', () => {
    expect(src).toMatch(/keyDirs[\s\S]*?s.*\[0,\s*1\]/);
  });

  it('uses g for pickup', () => {
    expect(src).toMatch(/e\.key === 'g'/);
    expect(src).toMatch(/sendPickup/);
  });

  it('uses b for drop', () => {
    expect(src).toMatch(/e\.key === 'b'/);
    expect(src).toMatch(/sendDrop/);
  });

  it('does not use s for pickup', () => {
    expect(src).not.toMatch(/e\.key === 's'[\s\S]*?sendPickup/);
  });

  it('does not use Shift+Z for drop', () => {
    expect(src).not.toMatch(/e\.key === 'Z'[\s\S]*?sendDrop/);
  });
});
