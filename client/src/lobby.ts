export interface GameInfo {
  mapName: string;
  host: string;
  port: number;
  players: number;
  maxPlayers: number;
}

const LOBBY_HTTP = 'http://localhost:3000';
const LOBBY_WS   = 'ws://localhost:3000/watch';

export async function fetchGames(): Promise<GameInfo[]> {
  const res = await fetch(`${LOBBY_HTTP}/games`);
  if (!res.ok) throw new Error(`Lobby error: ${res.status}`);
  return res.json() as Promise<GameInfo[]>;
}

export function watchGames(
  onUpdate: (games: GameInfo[]) => void,
  onError: (err: Event) => void,
): WebSocket {
  const ws = new WebSocket(LOBBY_WS);
  ws.addEventListener('message', (ev) => {
    try {
      onUpdate(JSON.parse(ev.data as string) as GameInfo[]);
    } catch { /* ignore malformed */ }
  });
  ws.addEventListener('error', onError);
  return ws;
}
