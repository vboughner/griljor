import { describe, it, expect } from 'vitest';
import { chooseSpriteKind } from '../renderer';

/** Defaults: alive team-1 player, no game over, all sprites loaded. */
function opts(overrides: Partial<Parameters<typeof chooseSpriteKind>[0]> = {}) {
  return {
    team: 1,
    gameOver: false,
    winningTeam: 0,
    hasTombstone: true,
    hasWinnerLoser: true,
    ...overrides,
  };
}

describe('chooseSpriteKind', () => {
  it('uses the character sprite for a living player in a normal game', () => {
    expect(chooseSpriteKind(opts())).toBe('own');
  });

  it('uses the tombstone for a dead player in a normal game', () => {
    expect(chooseSpriteKind(opts({ dead: true }))).toBe('tombstone');
  });

  it('falls back to the character sprite when no tombstone is loaded', () => {
    expect(chooseSpriteKind(opts({ dead: true, hasTombstone: false }))).toBe('own');
  });

  // ── game over ──────────────────────────────────────────────────────────

  it('uses the winner sprite for a living player on the winning team', () => {
    expect(chooseSpriteKind(opts({ gameOver: true, team: 2, winningTeam: 2 }))).toBe('winner');
  });

  it('uses the loser sprite for a living player on a losing team', () => {
    expect(chooseSpriteKind(opts({ gameOver: true, team: 1, winningTeam: 2 }))).toBe('loser');
  });

  it('keeps dead players as tombstones during the grace period', () => {
    // Regression: the game-over branch used to take precedence over the dead
    // check, so players who died just before the capture (respawn timers are
    // cancelled at game over) rendered as standing winners/losers.
    expect(chooseSpriteKind(opts({ dead: true, gameOver: true, team: 2, winningTeam: 2 }))).toBe(
      'tombstone',
    );
    expect(chooseSpriteKind(opts({ dead: true, gameOver: true, team: 1, winningTeam: 2 }))).toBe(
      'tombstone',
    );
  });

  it('never gives monsters winner or loser sprites', () => {
    // Regression: the swap applied to every entry in `others`, which includes
    // monsters, so monsters rendered as human winners/losers.
    expect(
      chooseSpriteKind(opts({ isMonster: true, gameOver: true, team: 2, winningTeam: 2 })),
    ).toBe('own');
    expect(
      chooseSpriteKind(opts({ isMonster: true, gameOver: true, team: 0, winningTeam: 2 })),
    ).toBe('own');
  });

  it('keeps a dead monster as a tombstone during the grace period', () => {
    expect(chooseSpriteKind(opts({ isMonster: true, dead: true, gameOver: true }))).toBe(
      'tombstone',
    );
  });

  it('falls back to the character sprite when winner/loser sprites are not loaded', () => {
    expect(
      chooseSpriteKind(opts({ gameOver: true, team: 2, winningTeam: 2, hasWinnerLoser: false })),
    ).toBe('own');
  });

  it('treats team 0 as a loser when a team has won', () => {
    expect(chooseSpriteKind(opts({ gameOver: true, team: 0, winningTeam: 1 }))).toBe('loser');
  });
});
