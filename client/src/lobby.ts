export interface GameInfo {
  mapName: string;
  title: string;
  teams: number;
  rooms: number;
  wsUrl: string;
  players: number;
  maxPlayers: number;
  avatars: Array<{ avatar: string; name: string; team: number }>;
  monsterAvatars: string[];
  startedAt: number;
}

const LOBBY_HTTP =
  (import.meta.env.VITE_LOBBY_URL as string | undefined) ?? 'http://localhost:3000';
// Derive WebSocket URL from HTTP URL (http→ws, https→wss)
const LOBBY_WS = LOBBY_HTTP.replace(/^http/, 'ws') + '/watch';

export async function fetchGames(): Promise<GameInfo[]> {
  const res = await fetch(`${LOBBY_HTTP}/games`);
  if (!res.ok) throw new Error(`Lobby error: ${res.status}`);
  return res.json() as Promise<GameInfo[]>;
}

export async function resetGame(
  wsUrl: string,
): Promise<{ ok: boolean; reason?: string; startedAt: number }> {
  const res = await fetch(`${LOBBY_HTTP}/reset`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ wsUrl }),
  });
  if (!res.ok) throw new Error(`Reset error: ${res.status}`);
  return res.json() as Promise<{ ok: boolean; reason?: string; startedAt: number }>;
}

export function watchGames(
  onUpdate: (games: GameInfo[]) => void,
  onError: (err: Event) => void,
): WebSocket {
  const ws = new WebSocket(LOBBY_WS);
  ws.addEventListener('message', (ev) => {
    try {
      onUpdate(JSON.parse(ev.data as string) as GameInfo[]);
    } catch {
      /* ignore malformed */
    }
  });
  ws.addEventListener('error', onError);
  return ws;
}
