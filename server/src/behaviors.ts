import { Monster, MonsterBehaviorConfig } from './monster-types';

// ── Behavior types ────────────────────────────────────────────────────────

export type BehaviorAction = { type: 'move'; x: number; y: number } | { type: 'idle' };

export interface BehaviorContext {
  /** Players in the same room (alive only). */
  nearbyPlayers: Array<{ id: number; x: number; y: number; team: number }>;
  /** Check if a tile is walkable (terrain only, not occupancy). */
  isWalkable(x: number, y: number): boolean;
  /** Check if a tile is occupied by a player or monster. */
  isOccupied(x: number, y: number): boolean;
}

export interface BehaviorHandler {
  onTick(monster: Monster, config: MonsterBehaviorConfig, context: BehaviorContext): BehaviorAction;
}

// ── 8-directional movement offsets (Chebyshev neighbors) ──────────────────

const DIRS: Array<[number, number]> = [
  [0, -1],
  [1, -1],
  [1, 0],
  [1, 1],
  [0, 1],
  [-1, 1],
  [-1, 0],
  [-1, -1],
];

// ── WanderBehavior ────────────────────────────────────────────────────────

export class WanderBehavior implements BehaviorHandler {
  onTick(
    monster: Monster,
    config: MonsterBehaviorConfig,
    context: BehaviorContext,
  ): BehaviorAction {
    // Chance to pause (do nothing)
    if (config.pauseChance !== undefined && Math.random() < config.pauseChance) {
      return { type: 'idle' };
    }

    // Collect valid adjacent tiles
    const candidates: Array<{ x: number; y: number }> = [];
    for (const [dx, dy] of DIRS) {
      const nx = monster.x + dx;
      const ny = monster.y + dy;
      if (!context.isWalkable(nx, ny)) continue;
      if (context.isOccupied(nx, ny)) continue;

      // Respect wander radius if set
      if (config.radius !== undefined) {
        const dist = Math.max(Math.abs(nx - monster.homeX), Math.abs(ny - monster.homeY));
        if (dist > config.radius) continue;
      }

      candidates.push({ x: nx, y: ny });
    }

    if (candidates.length === 0) return { type: 'idle' };
    const pick = candidates[Math.floor(Math.random() * candidates.length)];
    return { type: 'move', x: pick.x, y: pick.y };
  }
}

// ── FleeBehavior ──────────────────────────────────────────────────────────

export class FleeBehavior implements BehaviorHandler {
  onTick(
    monster: Monster,
    config: MonsterBehaviorConfig,
    context: BehaviorContext,
  ): BehaviorAction {
    // Find nearest non-team player
    let nearest: { id: number; x: number; y: number } | null = null;
    let nearestDist = Infinity;
    for (const p of context.nearbyPlayers) {
      if (p.team === monster.team && monster.team !== 0) continue;
      const dist = Math.max(Math.abs(p.x - monster.x), Math.abs(p.y - monster.y));
      if (dist < nearestDist) {
        nearestDist = dist;
        nearest = p;
      }
    }

    // If no threat nearby or already far enough, fall back to idle behavior
    const fleeRange = config.fleeRange ?? 6;
    if (!nearest || nearestDist > fleeRange) {
      if (config.idleBehavior === 'wander') {
        return wanderBehavior.onTick(monster, config, context);
      }
      return { type: 'idle' };
    }

    // Move away from the nearest threat: pick the adjacent tile that maximizes distance
    let bestTile: { x: number; y: number } | null = null;
    let bestDist = -1;
    for (const [dx, dy] of DIRS) {
      const nx = monster.x + dx;
      const ny = monster.y + dy;
      if (!context.isWalkable(nx, ny)) continue;
      if (context.isOccupied(nx, ny)) continue;
      const dist = Math.max(Math.abs(nx - nearest.x), Math.abs(ny - nearest.y));
      if (dist > bestDist) {
        bestDist = dist;
        bestTile = { x: nx, y: ny };
      }
    }

    if (!bestTile) return { type: 'idle' };
    return { type: 'move', x: bestTile.x, y: bestTile.y };
  }
}

// ── StationaryBehavior ────────────────────────────────────────────────────

export class StationaryBehavior implements BehaviorHandler {
  onTick(): BehaviorAction {
    return { type: 'idle' };
  }
}

// ── Behavior Registry ─────────────────────────────────────────────────────

const wanderBehavior = new WanderBehavior();
const fleeBehavior = new FleeBehavior();
const stationaryBehavior = new StationaryBehavior();

const behaviorRegistry = new Map<string, BehaviorHandler>();
behaviorRegistry.set('wander', wanderBehavior);
behaviorRegistry.set('flee', fleeBehavior);
behaviorRegistry.set('stationary', stationaryBehavior);
// Patrol and chase will use wander as fallback until Phase 4
behaviorRegistry.set('patrol', wanderBehavior);
behaviorRegistry.set('chase', wanderBehavior);

export function getBehavior(type: string): BehaviorHandler {
  return behaviorRegistry.get(type) ?? stationaryBehavior;
}
