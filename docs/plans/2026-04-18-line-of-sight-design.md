# Line-of-Sight Fix: DDA Supercover Ray

**Date**: 2026-04-18
**Branch**: `line-of-sight`

## Problem

The current `chebyshevPath` function walks diagonally one Chebyshev step at a time, visiting tiles that the actual geometric line between tile centers does NOT pass through. This causes false LOS blocks along diagonal sightlines.

Example: Player at (0,0), target at (1,3). Chebyshev visits (1,1) which the real line never touches — if opaque, sight is incorrectly blocked.

## Solution

Replace `chebyshevPath` with `losRayTiles` — a DDA grid traversal that returns all tiles the center-to-center line segment actually passes through.

**Boundary rule**: When the ray passes exactly along a tile edge or corner (touches but doesn't enter interior), that tile is NOT checked. This is the permissive option — you can "peek" along walls.

## Algorithm

DDA (Digital Differential Analyzer) grid walk from `(x1+0.5, y1+0.5)` to `(x2+0.5, y2+0.5)`:
1. Compute step direction (sx, sy) and distances to next grid line (tMaxX, tMaxY) and per-step increments (tDeltaX, tDeltaY)
2. At each step, advance along whichever axis hits a grid line first
3. When both axes hit simultaneously (corner crossing), step diagonally — skip the two tiles the ray merely grazes
4. Collect each tile entered; exclude start tile, include target tile

## Files Changed

| File | Change |
|------|--------|
| `client/src/los.ts` | Replace `chebyshevPath` with `losRayTiles`; update `spotIsVisible`, `tileIsVisible` |
| `server/src/session.ts` | Same replacement (duplicated LOS functions) |
| `client/src/__tests__/los.test.ts` | Replace chebyshev tests with ray tests; add diagonal/boundary cases |
| `server/src/__tests__/los.test.ts` | Same test updates |

## Not Changed

- `tileViewBlocked` — unchanged
- `isTileBlocked` / `pathIsWalkable` — movement pathing, separate from LOS
- Missile tracing — own Bresenham trace in session.ts
- Fog-of-war rendering — calls `tileIsVisible`, automatically gets new ray
