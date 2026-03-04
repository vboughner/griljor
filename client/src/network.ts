type C2SMessage =
  | { type: 'JOIN';         name: string; avatar: string }
  | { type: 'MY_LOCATION';  room: number; x: number; y: number }
  | { type: 'LEAVING_GAME' }
  | { type: 'MESSAGE';      to: number | 'all'; text: string };

type S2CMessage =
  | { type: 'ACCEPTED';     id: number; msg: string; mapName: string; rooms: number }
  | { type: 'REJECTED';     msg: string }
  | { type: 'PLAYER_INFO';  id: number; name: string; avatar: string;
                            room: number; x: number; y: number }
  | { type: 'MY_LOCATION';  id: number; room: number; x: number; y: number }
  | { type: 'LEAVING_GAME'; id: number }
  | { type: 'MESSAGE';      from: number; name: string; to: number | 'all'; text: string };

export class GameNetwork {
  private ws: WebSocket;

  onAccepted:   (msg: Extract<S2CMessage, { type: 'ACCEPTED' }>) => void    = () => {};
  onRejected:   (msg: Extract<S2CMessage, { type: 'REJECTED' }>) => void    = () => {};
  onPlayerInfo: (msg: Extract<S2CMessage, { type: 'PLAYER_INFO' }>) => void = () => {};
  onLocation:   (msg: Extract<S2CMessage, { type: 'MY_LOCATION' }>) => void = () => {};
  onLeave:      (msg: Extract<S2CMessage, { type: 'LEAVING_GAME' }>) => void = () => {};
  onMessage:    (msg: Extract<S2CMessage, { type: 'MESSAGE' }>) => void      = () => {};
  onClose:      () => void                                                   = () => {};

  constructor(url: string) {
    this.ws = new WebSocket(url);
    console.log(`[network] connecting to ${url}`);
    this.ws.addEventListener('open',  () => console.log('[network] connected'));
    this.ws.addEventListener('close', (ev) => { console.log(`[network] disconnected (code=${ev.code})`); this.onClose(); });
    this.ws.addEventListener('error', () => console.log('[network] connection error'));
    this.ws.addEventListener('message', (ev) => {
      let msg: S2CMessage;
      try {
        msg = JSON.parse(ev.data as string) as S2CMessage;
      } catch {
        return;
      }
      switch (msg.type) {
        case 'ACCEPTED':     console.log(`[network] accepted: id=${msg.id}, map=${msg.mapName}`); this.onAccepted(msg); break;
        case 'REJECTED':     console.log(`[network] rejected: ${msg.msg}`);                       this.onRejected(msg); break;
        case 'PLAYER_INFO':  this.onPlayerInfo(msg);  break;
        case 'MY_LOCATION':  this.onLocation(msg);    break;
        case 'LEAVING_GAME': this.onLeave(msg);       break;
        case 'MESSAGE':      this.onMessage(msg);     break;
      }
    });
  }

  join(name: string, avatar: string): void {
    console.log(`[network] joining as "${name}" (avatar: ${avatar})`);
    this.send({ type: 'JOIN', name, avatar });
  }

  sendLocation(room: number, x: number, y: number): void {
    this.send({ type: 'MY_LOCATION', room, x, y });
  }

  sendLeave(): void {
    this.send({ type: 'LEAVING_GAME' });
  }

  sendMessage(text: string): void {
    this.send({ type: 'MESSAGE', to: 'all', text });
  }

  private send(msg: C2SMessage): void {
    if (this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(msg));
    } else if (this.ws.readyState === WebSocket.CONNECTING) {
      this.ws.addEventListener('open', () => this.ws.send(JSON.stringify(msg)), { once: true });
    }
  }
}
