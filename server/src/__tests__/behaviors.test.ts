import { describe, it, expect } from 'vitest';
import {
  WanderBehavior,
  FleeBehavior,
  ChaseBehavior,
  PatrolBehavior,
  StationaryBehavior,
  BehaviorContext,
} from '../behaviors';
import { Monster, MonsterBehaviorConfig } from '../monster-types';

// ── Helpers ───────────────────────────────────────────────────────────────

function makeMonster(overrides: Partial<Monster> = {}): Monster {
  return {
    id: -1,
    defId: 'test',
    name: 'Test Monster',
    avatar: 'test',
    room: 0,
    x: 10,
    y: 10,
    hp: 50,
    maxHp: 50,
    team: 0,
    dead: false,
    carriedItems: [],
    homeRoom: 0,
    homeX: 10,
    homeY: 10,
    currentTarget: null,
    patrolIndex: 0,
    lastFireTime: 0,
    moveTimer: null,
    chatTimer: null,
    respawnTimer: null,
    ...overrides,
  };
}

function makeContext(overrides: Partial<BehaviorContext> = {}): BehaviorContext {
  return {
    nearbyPlayers: [],
    isWalkable: () => true,
    isOccupied: () => false,
    ...overrides,
  };
}

// ── WanderBehavior ────────────────────────────────────────────────────────

describe('WanderBehavior', () => {
  const wander = new WanderBehavior();
  const config: MonsterBehaviorConfig = { type: 'wander', moveInterval: 2000 };

  it('returns a move to an adjacent tile', () => {
    const monster = makeMonster();
    const action = wander.onTick(monster, config, makeContext());
    expect(action.type).toBe('move');
    if (action.type === 'move') {
      // Should be adjacent (Chebyshev distance = 1)
      expect(Math.abs(action.x - monster.x)).toBeLessThanOrEqual(1);
      expect(Math.abs(action.y - monster.y)).toBeLessThanOrEqual(1);
      // Should not be same tile
      expect(action.x !== monster.x || action.y !== monster.y).toBe(true);
    }
  });

  it('returns idle when all adjacent tiles are blocked', () => {
    const monster = makeMonster();
    const context = makeContext({ isWalkable: () => false });
    const action = wander.onTick(monster, config, context);
    expect(action.type).toBe('idle');
  });

  it('returns idle when all adjacent tiles are occupied', () => {
    const monster = makeMonster();
    const context = makeContext({ isOccupied: () => true });
    const action = wander.onTick(monster, config, context);
    expect(action.type).toBe('idle');
  });

  it('respects pauseChance (always pauses when chance = 1)', () => {
    const monster = makeMonster();
    const pauseConfig: MonsterBehaviorConfig = {
      type: 'wander',
      moveInterval: 2000,
      pauseChance: 1.0,
    };
    const action = wander.onTick(monster, pauseConfig, makeContext());
    expect(action.type).toBe('idle');
  });

  it('never pauses when pauseChance = 0', () => {
    const monster = makeMonster();
    const noPauseConfig: MonsterBehaviorConfig = {
      type: 'wander',
      moveInterval: 2000,
      pauseChance: 0,
    };
    // Run multiple times — should always move
    for (let i = 0; i < 20; i++) {
      const action = wander.onTick(monster, noPauseConfig, makeContext());
      expect(action.type).toBe('move');
    }
  });

  it('respects wander radius', () => {
    const monster = makeMonster({ x: 12, y: 10, homeX: 10, homeY: 10 });
    const radiusConfig: MonsterBehaviorConfig = {
      type: 'wander',
      moveInterval: 2000,
      radius: 2,
    };
    // Monster is at (12,10), home at (10,10) — distance 2. Can only move toward home or sideways.
    for (let i = 0; i < 50; i++) {
      const action = wander.onTick(monster, radiusConfig, makeContext());
      if (action.type === 'move') {
        const dist = Math.max(Math.abs(action.x - 10), Math.abs(action.y - 10));
        expect(dist).toBeLessThanOrEqual(2);
      }
    }
  });

  it('avoids unwalkable tiles', () => {
    const monster = makeMonster();
    // Only tile (11,10) is walkable
    const context = makeContext({
      isWalkable: (x, y) => x === 11 && y === 10,
    });
    for (let i = 0; i < 20; i++) {
      const action = wander.onTick(monster, config, context);
      if (action.type === 'move') {
        expect(action.x).toBe(11);
        expect(action.y).toBe(10);
      }
    }
  });
});

// ── FleeBehavior ──────────────────────────────────────────────────────────

describe('FleeBehavior', () => {
  const flee = new FleeBehavior();
  const config: MonsterBehaviorConfig = {
    type: 'flee',
    moveInterval: 800,
    fleeRange: 6,
    idleBehavior: 'stationary',
  };

  it('moves away from the nearest player', () => {
    const monster = makeMonster({ x: 10, y: 10 });
    const context = makeContext({
      nearbyPlayers: [{ id: 1, x: 8, y: 10, team: 1 }],
    });

    const action = flee.onTick(monster, config, context);
    expect(action.type).toBe('move');
    if (action.type === 'move') {
      // Should move east (away from player at x=8)
      expect(action.x).toBeGreaterThan(monster.x);
    }
  });

  it('returns idle when no players are nearby', () => {
    const monster = makeMonster();
    const context = makeContext({ nearbyPlayers: [] });
    const action = flee.onTick(monster, config, context);
    expect(action.type).toBe('idle');
  });

  it('returns idle when players are beyond fleeRange', () => {
    const monster = makeMonster({ x: 10, y: 10 });
    const context = makeContext({
      nearbyPlayers: [{ id: 1, x: 0, y: 0, team: 1 }], // distance 10, beyond fleeRange 6
    });
    const action = flee.onTick(monster, config, context);
    expect(action.type).toBe('idle');
  });

  it('falls back to wander when idleBehavior is wander and no threat', () => {
    const wanderFallback: MonsterBehaviorConfig = {
      type: 'flee',
      moveInterval: 800,
      fleeRange: 6,
      idleBehavior: 'wander',
    };
    const monster = makeMonster();
    const context = makeContext({ nearbyPlayers: [] });
    const action = flee.onTick(monster, wanderFallback, context);
    // Should wander (move), not idle
    expect(action.type).toBe('move');
  });

  it('does not flee from same-team players', () => {
    const monster = makeMonster({ x: 10, y: 10, team: 1 });
    const context = makeContext({
      nearbyPlayers: [{ id: 1, x: 9, y: 10, team: 1 }], // same team
    });
    const action = flee.onTick(monster, config, context);
    // Team 1 monster shouldn't flee from team 1 player
    expect(action.type).toBe('idle');
  });

  it('returns idle when boxed in with nowhere to flee', () => {
    const monster = makeMonster({ x: 10, y: 10 });
    const context = makeContext({
      nearbyPlayers: [{ id: 1, x: 9, y: 10, team: 1 }],
      isWalkable: () => false,
    });
    const action = flee.onTick(monster, config, context);
    expect(action.type).toBe('idle');
  });
});

// ── ChaseBehavior ─────────────────────────────────────────────────────────

describe('ChaseBehavior', () => {
  const chase = new ChaseBehavior();
  const config: MonsterBehaviorConfig = {
    type: 'chase',
    moveInterval: 1000,
    chaseRange: 10,
  };

  it('moves toward the nearest player', () => {
    const monster = makeMonster({ x: 10, y: 10 });
    const context = makeContext({
      nearbyPlayers: [{ id: 1, x: 13, y: 10, team: 1 }],
    });
    const action = chase.onTick(monster, config, context);
    expect(action.type).toBe('move');
    if (action.type === 'move') {
      expect(action.x).toBe(11); // one step east toward player
      expect(action.y).toBe(10);
    }
  });

  it('stops when adjacent to target', () => {
    const monster = makeMonster({ x: 10, y: 10 });
    const context = makeContext({
      nearbyPlayers: [{ id: 1, x: 11, y: 10, team: 1 }],
    });
    const action = chase.onTick(monster, config, context);
    expect(action.type).toBe('idle');
  });

  it('falls back to wander when no target in range', () => {
    const monster = makeMonster({ x: 10, y: 10 });
    const context = makeContext({ nearbyPlayers: [] });
    const action = chase.onTick(monster, config, context);
    // Should wander (move to adjacent tile), not idle
    expect(action.type).toBe('move');
  });

  it('does not chase same-team players', () => {
    const monster = makeMonster({ x: 10, y: 10, team: 1 });
    const context = makeContext({
      nearbyPlayers: [{ id: 1, x: 13, y: 10, team: 1 }],
    });
    const action = chase.onTick(monster, config, context);
    // No valid target — falls back to wander
    expect(action.type).toBe('move');
  });

  it('tries cardinal directions when diagonal is blocked', () => {
    const monster = makeMonster({ x: 10, y: 10 });
    const context = makeContext({
      nearbyPlayers: [{ id: 1, x: 12, y: 12, team: 1 }],
      // Block diagonal (11,11) but allow (11,10)
      isWalkable: (x, y) => !(x === 11 && y === 11),
      isOccupied: () => false,
    });
    const action = chase.onTick(monster, config, context);
    expect(action.type).toBe('move');
    if (action.type === 'move') {
      expect(action.x).toBe(11);
      expect(action.y).toBe(10);
    }
  });
});

// ── PatrolBehavior ────────────────────────────────────────────────────────

describe('PatrolBehavior', () => {
  const patrol = new PatrolBehavior();

  it('moves toward the current waypoint', () => {
    const monster = makeMonster({ x: 10, y: 10, patrolIndex: 0 });
    const config: MonsterBehaviorConfig = {
      type: 'patrol',
      moveInterval: 1000,
      waypoints: [
        { x: 15, y: 10 },
        { x: 15, y: 15 },
      ],
    };
    const action = patrol.onTick(monster, config, makeContext());
    expect(action.type).toBe('move');
    if (action.type === 'move') {
      expect(action.x).toBe(11); // one step east toward waypoint (15,10)
      expect(action.y).toBe(10);
    }
  });

  it('advances to next waypoint when reaching current one', () => {
    const monster = makeMonster({ x: 15, y: 10, patrolIndex: 0 });
    const config: MonsterBehaviorConfig = {
      type: 'patrol',
      moveInterval: 1000,
      waypoints: [
        { x: 15, y: 10 },
        { x: 15, y: 15 },
      ],
    };
    const action = patrol.onTick(monster, config, makeContext());
    expect(action.type).toBe('idle'); // pause at waypoint
    expect(monster.patrolIndex).toBe(1); // advanced to next
  });

  it('wraps around to first waypoint after last', () => {
    const monster = makeMonster({ x: 15, y: 15, patrolIndex: 1 });
    const config: MonsterBehaviorConfig = {
      type: 'patrol',
      moveInterval: 1000,
      waypoints: [
        { x: 15, y: 10 },
        { x: 15, y: 15 },
      ],
    };
    patrol.onTick(monster, config, makeContext());
    expect(monster.patrolIndex).toBe(0); // wrapped
  });

  it('falls back to wander when no waypoints', () => {
    const monster = makeMonster();
    const config: MonsterBehaviorConfig = {
      type: 'patrol',
      moveInterval: 1000,
    };
    const action = patrol.onTick(monster, config, makeContext());
    expect(action.type).toBe('move');
  });
});

// ── StationaryBehavior ────────────────────────────────────────────────────

describe('StationaryBehavior', () => {
  const stationary = new StationaryBehavior();

  it('always returns idle', () => {
    const monster = makeMonster();
    const config: MonsterBehaviorConfig = { type: 'stationary', moveInterval: 1000 };
    for (let i = 0; i < 10; i++) {
      const action = stationary.onTick(monster, config, makeContext());
      expect(action.type).toBe('idle');
    }
  });
});
