import { describe, it, expect } from 'vitest';
import WebSocket from 'ws';
import { GameSession } from '../../session';
import { World, RoomData } from '../../world';
import { buildTestWorld, joinPlayer, MockWebSocket } from './helpers';

// ── helpers ────────────────────────────────────────────────────────────────

function buildMultiTeamWorld(): World {
  const spot: number[][][] = Array.from({ length: 20 }, () =>
    Array.from({ length: 20 }, () => [1, 0]),
  );

  const objects = [null, { _index: 1, name: 'floor', movement: 5, permeable: true }];

  const room1: RoomData = { name: 'team1-room', floor: 0, team: 1, recorded_objects: [], spot };
  const room2: RoomData = { name: 'team2-room', floor: 0, team: 2, recorded_objects: [], spot };

  return {
    mapName: 'test-multi',
    title: 'Test Multi-Team Map',
    teams: 2,
    roomCount: 2,
    rooms: [room1, room2],
    objects,
    resetOnEmpty: false,
    resetAfterSeconds: 30,
    maxPlayers: 16,
  };
}

// ── tests ──────────────────────────────────────────────────────────────────

describe('team selection — onJoin team validation', () => {
  it('uses msg.team when it is valid (team 1)', () => {
    const session = new GameSession(buildMultiTeamWorld());
    joinPlayer(session, 'Alice', 'a', 1);
    const avatars = session.playerAvatars;
    expect(avatars.find((a) => a.name === 'Alice')?.team).toBe(1);
  });

  it('uses msg.team when it is valid (team 2)', () => {
    const session = new GameSession(buildMultiTeamWorld());
    joinPlayer(session, 'Alice', 'a', 2);
    expect(session.playerAvatars.find((a) => a.name === 'Alice')?.team).toBe(2);
  });

  it('falls back to team 1 when msg.team is 0', () => {
    const session = new GameSession(buildMultiTeamWorld());
    const ws = new MockWebSocket();
    session.handleConnection(ws as unknown as WebSocket);
    ws.receive({ type: 'JOIN', name: 'Alice', avatar: 'a', team: 0 });
    expect(session.playerAvatars.find((a) => a.name === 'Alice')?.team).toBe(1);
  });

  it('falls back to team 1 when msg.team exceeds world.teams', () => {
    const session = new GameSession(buildMultiTeamWorld());
    const ws = new MockWebSocket();
    session.handleConnection(ws as unknown as WebSocket);
    ws.receive({ type: 'JOIN', name: 'Alice', avatar: 'a', team: 3 });
    expect(session.playerAvatars.find((a) => a.name === 'Alice')?.team).toBe(1);
  });

  it('uses team 1 on a single-team map regardless of msg.team', () => {
    const session = new GameSession(buildTestWorld()); // world.teams === 0
    joinPlayer(session, 'Alice', 'a', 2);
    expect(session.playerAvatars.find((a) => a.name === 'Alice')?.team).toBe(1);
  });
});

describe('team selection — playerAvatars getter', () => {
  it('includes team field for each player', () => {
    const session = new GameSession(buildMultiTeamWorld());
    joinPlayer(session, 'Alice', 'a', 1);
    joinPlayer(session, 'Bob', 'b', 2);
    const avatars = session.playerAvatars;
    expect(avatars).toHaveLength(2);
    expect(avatars.find((a) => a.name === 'Alice')?.team).toBe(1);
    expect(avatars.find((a) => a.name === 'Bob')?.team).toBe(2);
  });
});

describe('team selection — spawn room', () => {
  it('team 1 player spawns in a team-1 room', () => {
    const session = new GameSession(buildMultiTeamWorld());
    const p = joinPlayer(session, 'Alice', 'a', 1);
    // room index 0 has team:1, room index 1 has team:2
    expect(p.room).toBe(0);
  });

  it('team 2 player spawns in a team-2 room', () => {
    const session = new GameSession(buildMultiTeamWorld());
    const p = joinPlayer(session, 'Alice', 'a', 2);
    // room index 1 has team:2
    expect(p.room).toBe(1);
  });

  it('player with out-of-range team spawns in team-1 room (fallback)', () => {
    const session = new GameSession(buildMultiTeamWorld());
    const ws = new MockWebSocket();
    session.handleConnection(ws as unknown as WebSocket);
    ws.receive({ type: 'JOIN', name: 'Alice', avatar: 'a', team: 99 });
    const accepted = ws.lastOfType('ACCEPTED');
    expect(accepted).toBeDefined();
    expect(accepted!.room).toBe(0); // team 1 fallback → room 0
  });
});
