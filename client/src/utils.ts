/** Convert a movement speed (1-9, where 9=fastest) to a step delay in ms.
 *  speed 9 → 150 ms, speed 5 → 270 ms, speed 4 → 338 ms. */
export function stepDelay(spd: number): number {
  return Math.max(50, Math.round((150 * 9) / Math.max(1, spd)));
}

/** Apply the HP-based movement speed penalty to a base delay.
 *  At full HP the delay is unchanged; at 50% HP the delay is ~1.41× (sqrt of the full penalty).
 *  Capped at stepDelay(1) (the slowest possible speed). */
export function applyHpPenalty(baseDelayMs: number, hp: number, maxHp: number): number {
  const hpFraction = Math.max(1, hp) / Math.max(1, maxHp);
  return Math.min(stepDelay(1), Math.round(baseDelayMs / Math.sqrt(hpFraction)));
}

/** Convert an elapsed duration (ms) to a human-readable age string. */
export function formatAge(ms: number): string {
  const s = ms / 1000;
  const m = s / 60;
  const h = m / 60;
  if (s < 45) return 'just joined';
  if (s < 90) return 'a minute';
  if (m < 45) return `${Math.round(m)} minutes`;
  if (m < 90) return 'an hour';
  if (h < 22) return `${Math.round(h)} hours`;
  if (h < 36) return 'a day';
  return `${Math.round(h / 24)} days`;
}
