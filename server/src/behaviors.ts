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

// ── ChaseBehavior ─────────────────────────────────────────────────────────

export class ChaseBehavior implements BehaviorHandler {
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

    const chaseRange = config.chaseRange ?? 10;
    if (!nearest || nearestDist > chaseRange) {
      // No target in range — fall back to wander
      return wanderBehavior.onTick(monster, config, context);
    }

    // Already adjacent — stay put (let combat handle it)
    if (nearestDist <= 1) return { type: 'idle' };

    // Move one Chebyshev step toward the target
    const dx = Math.sign(nearest.x - monster.x);
    const dy = Math.sign(nearest.y - monster.y);
    const nx = monster.x + dx;
    const ny = monster.y + dy;

    if (context.isWalkable(nx, ny) && !context.isOccupied(nx, ny)) {
      return { type: 'move', x: nx, y: ny };
    }

    // Direct path blocked — try cardinal components separately
    if (dx !== 0) {
      const altX = monster.x + dx;
      if (context.isWalkable(altX, monster.y) && !context.isOccupied(altX, monster.y)) {
        return { type: 'move', x: altX, y: monster.y };
      }
    }
    if (dy !== 0) {
      const altY = monster.y + dy;
      if (context.isWalkable(monster.x, altY) && !context.isOccupied(monster.x, altY)) {
        return { type: 'move', x: monster.x, y: altY };
      }
    }

    return { type: 'idle' };
  }
}

// ── PatrolBehavior ────────────────────────────────────────────────────────

export class PatrolBehavior implements BehaviorHandler {
  onTick(
    monster: Monster,
    config: MonsterBehaviorConfig,
    context: BehaviorContext,
  ): BehaviorAction {
    const waypoints = config.waypoints;
    if (!waypoints || waypoints.length === 0) {
      // No waypoints: fall back to wander
      return wanderBehavior.onTick(monster, config, context);
    }

    const target = waypoints[monster.patrolIndex % waypoints.length];

    // Reached current waypoint — advance to next
    if (monster.x === target.x && monster.y === target.y) {
      monster.patrolIndex = (monster.patrolIndex + 1) % waypoints.length;
      return { type: 'idle' }; // pause at waypoint for one tick
    }

    // Move one Chebyshev step toward the current waypoint
    const dx = Math.sign(target.x - monster.x);
    const dy = Math.sign(target.y - monster.y);
    const nx = monster.x + dx;
    const ny = monster.y + dy;

    if (context.isWalkable(nx, ny) && !context.isOccupied(nx, ny)) {
      return { type: 'move', x: nx, y: ny };
    }

    // Direct path blocked — try cardinal components
    if (dx !== 0) {
      const altX = monster.x + dx;
      if (context.isWalkable(altX, monster.y) && !context.isOccupied(altX, monster.y)) {
        return { type: 'move', x: altX, y: monster.y };
      }
    }
    if (dy !== 0) {
      const altY = monster.y + dy;
      if (context.isWalkable(monster.x, altY) && !context.isOccupied(monster.x, altY)) {
        return { type: 'move', x: monster.x, y: altY };
      }
    }

    return { type: 'idle' };
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
const chaseBehavior = new ChaseBehavior();
const patrolBehavior = new PatrolBehavior();
const stationaryBehavior = new StationaryBehavior();

const behaviorRegistry = new Map<string, BehaviorHandler>();
behaviorRegistry.set('wander', wanderBehavior);
behaviorRegistry.set('flee', fleeBehavior);
behaviorRegistry.set('chase', chaseBehavior);
behaviorRegistry.set('patrol', patrolBehavior);
behaviorRegistry.set('stationary', stationaryBehavior);

export function getBehavior(type: string): BehaviorHandler {
  return behaviorRegistry.get(type) ?? stationaryBehavior;
}
