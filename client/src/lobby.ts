export interface GameInfo {
  mapName: string;
  host: string;
  port: number;
  players: number;
  maxPlayers: number;
}

const LOBBY_URL = 'http://localhost:3000';

export async function fetchGames(): Promise<GameInfo[]> {
  const res = await fetch(`${LOBBY_URL}/games`);
  if (!res.ok) throw new Error(`Lobby error: ${res.status}`);
  return res.json() as Promise<GameInfo[]>;
}
