import { describe, it, expect } from 'vitest';
import { readFileSync } from 'fs';
import { join } from 'path';
import { readdirSync } from 'fs';

const objDir = join(__dirname, '..', '..', '..', 'pipeline', 'out', 'data', 'objects');
const objFiles = readdirSync(objDir).filter((f) => f.endsWith('.json'));

describe('object set item names', () => {
  it.each(objFiles)('%s: all takeable items must have a real name (not "no name")', (file) => {
    const raw = readFileSync(join(objDir, file), 'utf-8');
    const data = JSON.parse(raw) as { objects: Array<Record<string, unknown> | null> };
    const takeable = data.objects.filter(
      (o): o is Record<string, unknown> => o !== null && o.takeable === true,
    );

    const noName = takeable.filter((o) => !o.name || o.name === 'no name');
    expect(noName).toEqual([]);
  });
});
