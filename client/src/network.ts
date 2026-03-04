import { InventoryItem } from './types';

type C2SMessage =
  | { type: 'JOIN';         name: string; avatar: string }
  | { type: 'MY_LOCATION';  room: number; x: number; y: number }
  | { type: 'LEAVING_GAME' }
  | { type: 'MESSAGE';      to: number | 'all'; text: string }
  | { type: 'PICKUP';       x: number; y: number; hand: 'left' | 'right' }
  | { type: 'DROP';         source: 'left' | 'right' | number }
  | { type: 'INV_SWAP';     slot: number; hand: 'left' | 'right' }
  | { type: 'FIRE_WEAPON';  hand: 'left' | 'right'; targetX: number; targetY: number };

type S2CMessage =
  | { type: 'ACCEPTED';      id: number; msg: string; mapName: string; rooms: number }
  | { type: 'REJECTED';      msg: string }
  | { type: 'PLAYER_INFO';   id: number; name: string; avatar: string;
                             room: number; x: number; y: number;
                             kills: number; deaths: number; joinedAt: number }
  | { type: 'PLAYER_STATS';  id: number; kills: number; deaths: number }
  | { type: 'MY_LOCATION';   id: number; room: number; x: number; y: number }
  | { type: 'LEAVING_GAME';  id: number }
  | { type: 'MESSAGE';       from: number; name: string; to: number | 'all'; text: string }
  | { type: 'ITEM_REMOVED';  room: number; x: number; y: number }
  | { type: 'ITEM_ADDED';    room: number; x: number; y: number; item: InventoryItem }
  | { type: 'YOUR_INVENTORY';
      leftHand: InventoryItem | null;
      rightHand: InventoryItem | null;
      inventory: Array<InventoryItem | null>;
      currentWeight: number; maxWeight: number }
  | { type: 'ITEMS_SYNC';
      items: Array<{ room: number; x: number; y: number; item: InventoryItem }> }
  | { type: 'YOUR_STATS';    hp: number; maxHp: number; power: number; maxPower: number;
                             xp: number; level: number }
  | { type: 'PLAYER_HEALTH'; id: number; hp: number; maxHp: number }
  | { type: 'MISSILE';       room: number; fromX: number; fromY: number;
                             toX: number; toY: number; objType: number }
  | { type: 'REPORT';        text: string }
  | { type: 'YOU_DIED';      killedBy: number; killerName: string;
                             respawnRoom: number; respawnX: number; respawnY: number };

export class GameNetwork {
  private ws: WebSocket;

  onAccepted:    (msg: Extract<S2CMessage, { type: 'ACCEPTED' }>) => void       = () => {};
  onRejected:    (msg: Extract<S2CMessage, { type: 'REJECTED' }>) => void       = () => {};
  onPlayerInfo:  (msg: Extract<S2CMessage, { type: 'PLAYER_INFO' }>) => void    = () => {};
  onPlayerStats: (msg: Extract<S2CMessage, { type: 'PLAYER_STATS' }>) => void   = () => {};
  onLocation:    (msg: Extract<S2CMessage, { type: 'MY_LOCATION' }>) => void    = () => {};
  onLeave:       (msg: Extract<S2CMessage, { type: 'LEAVING_GAME' }>) => void   = () => {};
  onMessage:     (msg: Extract<S2CMessage, { type: 'MESSAGE' }>) => void        = () => {};
  onItemRemoved: (msg: Extract<S2CMessage, { type: 'ITEM_REMOVED' }>) => void   = () => {};
  onItemAdded:   (msg: Extract<S2CMessage, { type: 'ITEM_ADDED' }>) => void     = () => {};
  onInventory:   (msg: Extract<S2CMessage, { type: 'YOUR_INVENTORY' }>) => void = () => {};
  onItemsSync:   (msg: Extract<S2CMessage, { type: 'ITEMS_SYNC' }>) => void     = () => {};
  onYourStats:   (msg: Extract<S2CMessage, { type: 'YOUR_STATS' }>) => void     = () => {};
  onPlayerHealth:(msg: Extract<S2CMessage, { type: 'PLAYER_HEALTH' }>) => void  = () => {};
  onMissile:     (msg: Extract<S2CMessage, { type: 'MISSILE' }>) => void        = () => {};
  onReport:      (msg: Extract<S2CMessage, { type: 'REPORT' }>) => void         = () => {};
  onYouDied:     (msg: Extract<S2CMessage, { type: 'YOU_DIED' }>) => void       = () => {};
  onClose:       () => void                                                      = () => {};

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
        case 'ACCEPTED':       console.log(`[network] accepted: id=${msg.id}, map=${msg.mapName}`); this.onAccepted(msg); break;
        case 'REJECTED':       console.log(`[network] rejected: ${msg.msg}`);                       this.onRejected(msg); break;
        case 'PLAYER_INFO':    this.onPlayerInfo(msg);   break;
        case 'PLAYER_STATS':   this.onPlayerStats(msg);  break;
        case 'MY_LOCATION':    this.onLocation(msg);     break;
        case 'LEAVING_GAME':   this.onLeave(msg);        break;
        case 'MESSAGE':        this.onMessage(msg);      break;
        case 'ITEM_REMOVED':   this.onItemRemoved(msg);  break;
        case 'ITEM_ADDED':     this.onItemAdded(msg);    break;
        case 'YOUR_INVENTORY': this.onInventory(msg);    break;
        case 'ITEMS_SYNC':     this.onItemsSync(msg);    break;
        case 'YOUR_STATS':     this.onYourStats(msg);    break;
        case 'PLAYER_HEALTH':  this.onPlayerHealth(msg); break;
        case 'MISSILE':        this.onMissile(msg);      break;
        case 'REPORT':         this.onReport(msg);       break;
        case 'YOU_DIED':       this.onYouDied(msg);      break;
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

  sendPickup(x: number, y: number, hand: 'left' | 'right'): void {
    this.send({ type: 'PICKUP', x, y, hand });
  }

  sendDrop(source: 'left' | 'right' | number): void {
    this.send({ type: 'DROP', source });
  }

  sendInvSwap(slot: number, hand: 'left' | 'right'): void {
    this.send({ type: 'INV_SWAP', slot, hand });
  }

  sendFireWeapon(hand: 'left' | 'right', targetX: number, targetY: number): void {
    this.send({ type: 'FIRE_WEAPON', hand, targetX, targetY });
  }

  private send(msg: C2SMessage): void {
    if (this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(msg));
    } else if (this.ws.readyState === WebSocket.CONNECTING) {
      this.ws.addEventListener('open', () => this.ws.send(JSON.stringify(msg)), { once: true });
    }
  }
}
