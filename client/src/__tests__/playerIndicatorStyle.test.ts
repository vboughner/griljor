import { describe, it, expect } from 'vitest';
import { playerIndicatorStyle } from '../renderer';

describe('playerIndicatorStyle', () => {
  it('returns null when boxOtherPlayers is false (master off)', () => {
    expect(playerIndicatorStyle(true, 1, 1, false)).toBeNull();
    expect(playerIndicatorStyle(false, 0, 0, false)).toBeNull();
    expect(playerIndicatorStyle(true, 2, 2, false)).toBeNull();
  });

  it('returns green corner style for teammates in a 2-team game', () => {
    const style = playerIndicatorStyle(true, 1, 1, true);
    expect(style).toEqual({ color: '#00cc00', lineWidth: 1, corners: true });
  });

  it('returns red full-rect style for enemies in a 2-team game', () => {
    const style = playerIndicatorStyle(true, 1, 2, true);
    expect(style).toEqual({ color: '#ff4444', lineWidth: 1, corners: false });
  });

  it('treats team 0 players as non-teammates even if local is also team 0', () => {
    const style = playerIndicatorStyle(true, 0, 0, true);
    expect(style).toEqual({ color: '#ff4444', lineWidth: 1, corners: false });
  });

  it('free-for-all map (teamsEnabled=false): all players get red full-rect indicator', () => {
    // teamsEnabled=false — no green teammate outlines
    expect(playerIndicatorStyle(false, 1, 1, true)).toEqual({
      color: '#ff4444',
      lineWidth: 1,
      corners: false,
    });
    expect(playerIndicatorStyle(false, 2, 2, true)).toEqual({
      color: '#ff4444',
      lineWidth: 1,
      corners: false,
    });
    expect(playerIndicatorStyle(false, 0, 3, true)).toEqual({
      color: '#ff4444',
      lineWidth: 1,
      corners: false,
    });
  });

  it('free-for-all map, boxOtherPlayers=false → null', () => {
    expect(playerIndicatorStyle(false, 1, 1, false)).toBeNull();
  });
});
