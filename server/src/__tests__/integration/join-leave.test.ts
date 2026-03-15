import { describe, it, expect, beforeEach } from 'vitest';
import WebSocket from 'ws';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer, MockWebSocket } from './helpers';

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

  it('LEAVING_GAME broadcast when player disconnects', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    bob.ws.flush();
    alice.ws.close();
    expect(session.playerCount).toBe(1);
    const leaves = bob.ws.messagesOfType('LEAVING_GAME');
    expect(leaves.some((m) => m.id === alice.id)).toBe(true);
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
});
