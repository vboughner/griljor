/** Convert a movement speed (1-9, where 9=fastest) to a step delay in ms.
 *  speed 9 → 150 ms, speed 5 → 270 ms, speed 4 → 338 ms. */
export function stepDelay(spd: number): number {
  return Math.max(50, Math.round((150 * 9) / Math.max(1, spd)));
}
