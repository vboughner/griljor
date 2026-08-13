import { describe, it, expect } from 'vitest';
import {
  summarizeFlags,
  describeFlagSummary,
  countAcquiredFlags,
  FlagStatusEntry,
  FlagSummary,
} from '../game-utils';

/** The viewer's team in these tests. */
const ME = 1;

/** A flag instance sitting on the floor of a room owned by `team` (0 = neutral). */
function onFloor(objType: number, team = 0, room = 1): FlagStatusEntry {
  return { objType, room, x: 5, y: 5, heldBy: 0, heldByName: '', teamHolding: team };
}

/** A flag instance in a player's hands. */
function carried(objType: number, name: string, id = 1): FlagStatusEntry {
  return { objType, room: -1, x: 0, y: 0, heldBy: id, heldByName: name, teamHolding: 0 };
}

function summary(overrides: Partial<FlagSummary> = {}): FlagSummary {
  return { objType: 149, carriers: [], yours: 0, enemy: 0, loose: 0, ...overrides };
}

describe('summarizeFlags', () => {
  it('returns nothing for an empty status list', () => {
    expect(summarizeFlags([], ME)).toEqual([]);
  });

  it('collapses many instances of one flag type into a single summary', () => {
    // Regression: the HUD used to render one line per instance, so battle.json's
    // 12 Quaso flags produced 12 identical "Quaso flag: at base" lines.
    const flags = [
      ...Array.from({ length: 10 }, () => onFloor(149, 0)),
      onFloor(149, 1),
      onFloor(149, 2),
    ];
    expect(summarizeFlags(flags, ME)).toEqual([
      summary({ objType: 149, yours: 1, enemy: 1, loose: 10 }),
    ]);
  });

  it('keeps distinct flag types on separate lines', () => {
    const result = summarizeFlags([onFloor(149, 1), onFloor(148, 2)], ME);
    expect(result).toEqual([
      summary({ objType: 148, enemy: 1 }),
      summary({ objType: 149, yours: 1 }),
    ]);
  });

  it('sorts by objType so line order does not jitter between updates', () => {
    const result = summarizeFlags([onFloor(252), onFloor(63), onFloor(149)], ME);
    expect(result.map((s) => s.objType)).toEqual([63, 149, 252]);
  });

  it('separates carried, yours, enemy, and loose instances of the same type', () => {
    const result = summarizeFlags(
      [carried(149, 'Alice', 1), onFloor(149, 1), onFloor(149, 2), onFloor(149, 0)],
      ME,
    );
    expect(result).toEqual([summary({ carriers: ['Alice'], yours: 1, enemy: 1, loose: 1 })]);
  });

  it('counts the same flags as enemy-held from the other team’s view', () => {
    const flags = [onFloor(149, 1), onFloor(149, 2)];
    expect(summarizeFlags(flags, 2)).toEqual([summary({ yours: 1, enemy: 1 })]);
  });

  it('records every carrier when several players hold the same flag type', () => {
    const result = summarizeFlags([carried(149, 'Alice', 1), carried(149, 'Bob', 2)], ME);
    expect(result[0].carriers).toEqual(['Alice', 'Bob']);
  });

  it('substitutes a placeholder for a missing carrier name', () => {
    expect(summarizeFlags([carried(149, '', 1)], ME)[0].carriers).toEqual(['???']);
  });
});

describe('describeFlagSummary', () => {
  it('describes a flag sitting only in your rooms', () => {
    expect(describeFlagSummary(summary({ yours: 2 }))).toBe('2 in your rooms');
  });

  it('describes the battle.json case compactly', () => {
    expect(describeFlagSummary(summary({ yours: 1, enemy: 1, loose: 10 }))).toBe(
      '1 in your rooms, 1 in enemy rooms, 10 loose',
    );
  });

  it('leads with carriers, then floor counts', () => {
    expect(describeFlagSummary(summary({ carriers: ['Alice'], yours: 1, loose: 3 }))).toBe(
      'carried by Alice, 1 in your rooms, 3 loose',
    );
  });

  it('lists up to three carriers by name', () => {
    expect(describeFlagSummary(summary({ carriers: ['Alice', 'Bob', 'Cara'] }))).toBe(
      'carried by Alice, Bob, Cara',
    );
  });

  it('collapses the tail when more than three players carry the flag', () => {
    expect(describeFlagSummary(summary({ carriers: ['Alice', 'Bob', 'Cara', 'Dan', 'Eve'] }))).toBe(
      'carried by Alice, Bob, Cara +2 more',
    );
  });

  it('omits zero counts rather than printing "0 loose"', () => {
    expect(describeFlagSummary(summary({ loose: 4 }))).toBe('4 loose');
  });

  it('falls back to "none" when a type has no instances left', () => {
    expect(describeFlagSummary(summary())).toBe('none');
  });
});

describe('countAcquiredFlags', () => {
  const needsAll = () => true;

  it('counts only flags on the floor of your own rooms', () => {
    const flags = [onFloor(149, 1), onFloor(149, 1), onFloor(149, 2), onFloor(149, 0)];
    expect(countAcquiredFlags(flags, ME, needsAll)).toBe(2);
  });

  it('does not count a flag being carried through your own base', () => {
    // Carried flags report room -1 / teamHolding 0: the server only counts
    // instances resting on the floor, so progress must not jump while a player
    // walks the flag around.
    expect(countAcquiredFlags([carried(149, 'Alice', 1)], ME, needsAll)).toBe(0);
  });

  it('ignores flag types your team does not need', () => {
    const flags = [onFloor(148, 1), onFloor(149, 1)];
    expect(countAcquiredFlags(flags, ME, (t) => t === 149)).toBe(1);
  });

  it('is zero when nothing has been gathered yet', () => {
    expect(countAcquiredFlags([onFloor(149, 0), onFloor(149, 2)], ME, needsAll)).toBe(0);
  });
});
