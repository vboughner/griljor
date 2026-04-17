import { describe, it, expect, beforeEach } from 'vitest';
import WebSocket from 'ws';
import { GameSession } from '../../session';
import { buildTestWorld, buildTwoRoomWorld, joinPlayer, MockWebSocket } from './helpers';

describe('join / leave', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildTestWorld());
  });

  it('ACCEPTED is the first message when joining', () => {
    const p = joinPlayer(session, 'Alice');
    expect(p.ws.messages()[0].type).toBe('ACCEPTED');
  });

  it('player count increments on join', () => {
    expect(session.playerCount).toBe(0);
    joinPlayer(session, 'Alice');
    expect(session.playerCount).toBe(1);
    joinPlayer(session, 'Bob');
    expect(session.playerCount).toBe(2);
  });

  it('REJECTED when name is already taken', () => {
    joinPlayer(session, 'Alice');
    const ws2 = new MockWebSocket();
    session.handleConnection(ws2 as unknown as WebSocket);
    ws2.receive({ type: 'JOIN', name: 'Alice', avatar: 'b', team: 1 });
    expect(ws2.lastOfType('REJECTED')).toBeDefined();
  });

  it('REJECTED when name matches existing player name case-insensitively', () => {
    joinPlayer(session, 'Alice');
    const ws2 = new MockWebSocket();
    session.handleConnection(ws2 as unknown as WebSocket);
    ws2.receive({ type: 'JOIN', name: 'ALICE', avatar: 'b', team: 1 });
    expect(ws2.lastOfType('REJECTED')).toBeDefined();
  });

  it('second player receives PLAYER_INFO about first', () => {
    joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    const infos = bob.ws.messagesOfType('PLAYER_INFO');
    expect(infos.some((m) => m.name === 'Alice')).toBe(true);
  });

  it('first player receives PLAYER_INFO when second joins', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();
    joinPlayer(session, 'Bob');
    const infos = alice.ws.messagesOfType('PLAYER_INFO');
    expect(infos.some((m) => m.name === 'Bob')).toBe(true);
  });

  it('LEAVING_GAME broadcast with reason "disconnected" when player closes connection', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.flush();
    alice.ws.close();
    expect(session.playerCount).toBe(1);
    const leave = bob.ws.messagesOfType('LEAVING_GAME').find((m) => m.id === alice.id);
    expect(leave).toBeDefined();
    expect(leave!.name).toBe('Alice');
    expect(leave!.reason).toBe('disconnected');
  });

  it('LEAVING_GAME broadcast with reason "left" when player sends LEAVING_GAME', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.flush();
    alice.ws.receive({ type: 'LEAVING_GAME' });
    expect(session.playerCount).toBe(1);
    const leave = bob.ws.messagesOfType('LEAVING_GAME').find((m) => m.id === alice.id);
    expect(leave).toBeDefined();
    expect(leave!.name).toBe('Alice');
    expect(leave!.reason).toBe('left');
  });

  it('player count decrements after disconnect', () => {
    const alice = joinPlayer(session, 'Alice');
    joinPlayer(session, 'Bob');
    alice.ws.close();
    expect(session.playerCount).toBe(1);
  });

  it('ACCEPTED includes the player team', () => {
    // Default test world has teams=0, so any value is clamped to 1
    const alice = joinPlayer(session, 'Alice', 'a', 1);
    const accepted = alice.ws.lastOfType('ACCEPTED');
    expect(accepted?.team).toBe(1);
  });

  it('PLAYER_INFO includes the player team', () => {
    // Use a session with teams=2 so team values are preserved
    const multiTeamSession = new GameSession({ ...buildTestWorld(), teams: 2 });
    joinPlayer(multiTeamSession, 'Alice', 'a', 2);
    const bob = joinPlayer(multiTeamSession, 'Bob', 'b', 1);
    const aliceInfo = bob.ws.messagesOfType('PLAYER_INFO').find((m) => m.name === 'Alice');
    expect(aliceInfo?.team).toBe(2);
    multiTeamSession.destroy();
  });

  it('GM broadcasts a join message to existing players when a new player joins', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();
    joinPlayer(session, 'Bob');
    const gmMsgs = alice.ws
      .messagesOfType('MESSAGE')
      .filter((m) => m.name === 'GM' && m.text.includes('Bob'));
    expect(gmMsgs).toHaveLength(1);
    expect(gmMsgs[0].text).toBe('Bob joined the game.');
  });

  it('GM join message includes team number when map has multiple teams', () => {
    const multiTeamSession = new GameSession({ ...buildTestWorld(), teams: 2 });
    const alice = joinPlayer(multiTeamSession, 'Alice', 'a', 1);
    alice.ws.flush();
    joinPlayer(multiTeamSession, 'Bob', 'b', 2);
    const gmMsgs = alice.ws
      .messagesOfType('MESSAGE')
      .filter((m) => m.name === 'GM' && m.text.includes('Bob'));
    expect(gmMsgs).toHaveLength(1);
    expect(gmMsgs[0].text).toBe('Bob joined the game (team 2).');
    multiTeamSession.destroy();
  });

  it('GM join message omits team when map has no teams', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();
    joinPlayer(session, 'Bob');
    const gmMsgs = alice.ws
      .messagesOfType('MESSAGE')
      .filter((m) => m.name === 'GM' && m.text.includes('Bob'));
    expect(gmMsgs[0].text).not.toContain('team');
  });

  it('new player receives their own GM join message', () => {
    joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    const gmMsgs = bob.ws
      .messagesOfType('MESSAGE')
      .filter((m) => m.name === 'GM' && m.text.includes('Bob joined'));
    expect(gmMsgs).toHaveLength(1);
  });

  it('PLAYER_JOINED is sent to existing players when a new player joins', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.flush();
    joinPlayer(session, 'Bob', 'b');
    const joined = alice.ws.messagesOfType('PLAYER_JOINED');
    expect(joined.some((m) => m.name === 'Bob')).toBe(true);
  });

  it('PLAYER_JOINED is sent to new player for each existing player', () => {
    joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    const joined = bob.ws.messagesOfType('PLAYER_JOINED');
    expect(joined.some((m) => m.name === 'Alice')).toBe(true);
  });

  it('PLAYER_JOINED does not include position fields', () => {
    joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    const joined = bob.ws.messagesOfType('PLAYER_JOINED').find((m) => m.name === 'Alice');
    expect(joined).toBeDefined();
    expect(joined).not.toHaveProperty('room');
    expect(joined).not.toHaveProperty('x');
    expect(joined).not.toHaveProperty('y');
  });

  it('PLAYER_JOINED is sent even when players are in different rooms', () => {
    // Use teams to force players into different rooms deterministically
    const world = buildTwoRoomWorld();
    world.teams = 2;
    world.rooms[0].team = 1;
    world.rooms[1].team = 2;
    const twoRoomSession = new GameSession(world);
    const alice = joinPlayer(twoRoomSession, 'Alice', 'a', 1); // spawns room 0
    alice.ws.flush();
    const bob = joinPlayer(twoRoomSession, 'Bob', 'b', 2); // spawns room 1
    expect(alice.room).not.toBe(bob.room);
    const joined = alice.ws.messagesOfType('PLAYER_JOINED');
    expect(joined.some((m) => m.name === 'Bob')).toBe(true);
    // Alice should NOT get PLAYER_INFO for Bob since they're in different rooms
    const infos = alice.ws.messagesOfType('PLAYER_INFO').filter((m) => m.name === 'Bob');
    expect(infos).toHaveLength(0);
    twoRoomSession.destroy();
  });
});
