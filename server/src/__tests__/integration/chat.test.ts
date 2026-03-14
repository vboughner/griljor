import { describe, it, expect, beforeEach } from 'vitest';
import { GameSession } from '../../session';
import { buildTestWorld, joinPlayer } from './helpers';

describe('chat', () => {
  let session: GameSession;

  beforeEach(() => {
    session = new GameSession(buildTestWorld());
  });

  it('broadcast message is received by all players', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'MESSAGE', to: 'all', text: 'hello' });

    const aliceMsgs = alice.ws.messagesOfType('MESSAGE');
    const bobMsgs = bob.ws.messagesOfType('MESSAGE');
    expect(aliceMsgs.some((m) => m.text === 'hello')).toBe(true);
    expect(bobMsgs.some((m) => m.text === 'hello')).toBe(true);
  });

  it('new player receives chat history on join', () => {
    const alice = joinPlayer(session, 'Alice');
    alice.ws.receive({ type: 'MESSAGE', to: 'all', text: 'before bob joined' });

    const bob = joinPlayer(session, 'Bob');
    const bobHistory = bob.ws.messagesOfType('MESSAGE');
    expect(bobHistory.some((m) => m.text === 'before bob joined')).toBe(true);
  });

  it('profanity triggers GM scold broadcast', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    alice.ws.flush();
    bob.ws.flush();

    alice.ws.receive({ type: 'MESSAGE', to: 'all', text: 'damn' });

    const bobMsgs = bob.ws.messagesOfType('MESSAGE');
    const gmScold = bobMsgs.find((m) => m.from === 0 && m.name === 'GM');
    expect(gmScold).toBeDefined();
  });

  it('direct message only reaches sender and target', () => {
    const alice = joinPlayer(session, 'Alice');
    const bob = joinPlayer(session, 'Bob');
    const carol = joinPlayer(session, 'Carol');
    alice.ws.flush();
    bob.ws.flush();
    carol.ws.flush();

    alice.ws.receive({ type: 'MESSAGE', to: bob.id, text: 'secret' });

    expect(alice.ws.messagesOfType('MESSAGE').some((m) => m.text === 'secret')).toBe(true);
    expect(bob.ws.messagesOfType('MESSAGE').some((m) => m.text === 'secret')).toBe(true);
    expect(carol.ws.messagesOfType('MESSAGE').some((m) => m.text === 'secret')).toBe(false);
  });
});
