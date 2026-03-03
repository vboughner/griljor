// Client → Server messages
export type C2SMessage =
  | { type: 'JOIN';         name: string; avatar: string }
  | { type: 'MY_LOCATION';  room: number; x: number; y: number }
  | { type: 'LEAVING_GAME' }
  | { type: 'MESSAGE';      to: number | 'all'; text: string };

// Server → Client messages
export type S2CMessage =
  | { type: 'ACCEPTED';     id: number; msg: string; mapName: string; rooms: number }
  | { type: 'REJECTED';     msg: string }
  | { type: 'PLAYER_INFO';  id: number; name: string; avatar: string;
                            room: number; x: number; y: number }
  | { type: 'MY_LOCATION';  id: number; room: number; x: number; y: number }
  | { type: 'LEAVING_GAME'; id: number }
  | { type: 'MESSAGE';      from: number; to: number | 'all'; text: string };
