export interface InventoryItem {
  type: number;
  quantity: number;
}

// Client → Server messages
export type C2SMessage =
  | { type: 'JOIN'; name: string; avatar: string; team: number }
  | { type: 'MY_LOCATION'; room: number; x: number; y: number }
  | { type: 'LEAVING_GAME' }
  | { type: 'MESSAGE'; to: number | 'all'; text: string }
  | { type: 'PICKUP'; x: number; y: number; hand: 'left' | 'right' }
  | { type: 'DROP'; source: 'left' | 'right' | number }
  | { type: 'INV_SWAP'; slot: number; hand: 'left' | 'right' }
  | { type: 'FIRE_WEAPON'; hand: 'left' | 'right'; targetX: number; targetY: number }
  | { type: 'USE_ITEM'; hand: 'left' | 'right'; targetX: number; targetY: number }
  | { type: 'PING' }
  | { type: 'VOLUNTARY_RESPAWN' };

// Server → Client messages
export type S2CMessage =
  | {
      type: 'ACCEPTED';
      id: number;
      msg: string;
      mapName: string;
      rooms: number;
      room: number;
      x: number;
      y: number;
    }
  | { type: 'REJECTED'; msg: string }
  | {
      type: 'PLAYER_INFO';
      id: number;
      name: string;
      avatar: string;
      room: number;
      x: number;
      y: number;
      kills: number;
      deaths: number;
      joinedAt: number;
      dead: boolean;
    }
  | { type: 'PLAYER_STATS'; id: number; kills: number; deaths: number }
  | { type: 'MY_LOCATION'; id: number; room: number; x: number; y: number }
  | { type: 'LEAVING_GAME'; id: number }
  | { type: 'MESSAGE'; from: number; name: string; to: number | 'all'; text: string }
  | { type: 'ITEM_REMOVED'; room: number; x: number; y: number }
  | { type: 'ITEM_ADDED'; room: number; x: number; y: number; item: InventoryItem }
  | {
      type: 'YOUR_INVENTORY';
      leftHand: InventoryItem | null;
      rightHand: InventoryItem | null;
      inventory: Array<InventoryItem | null>;
      currentWeight: number;
      maxWeight: number;
    }
  | {
      type: 'ITEMS_SYNC';
      items: Array<{ room: number; x: number; y: number; item: InventoryItem }>;
    }
  | { type: 'YOUR_STATS'; hp: number; maxHp: number }
  | { type: 'PLAYER_HEALTH'; id: number; hp: number; maxHp: number }
  | {
      type: 'MISSILE_START';
      id: number;
      room: number;
      path: Array<{ x: number; y: number }>;
      objType: number;
      msPerStep: number;
      dx: number;
      dy: number;
    }
  | { type: 'MISSILE_END'; id: number }
  | { type: 'REPORT'; text: string }
  | { type: 'PLAYER_HIT'; victimId: number; room: number; x: number; y: number; damage: number }
  | { type: 'PLAYER_HEAL'; playerId: number; room: number; x: number; y: number; amount: number }
  | { type: 'YOU_DIED'; killedBy: number; killerName: string; deadForMs: number }
  | { type: 'YOU_RESPAWNED'; room: number; x: number; y: number }
  | { type: 'ROOM_OBJECT_CHANGED'; room: number; x: number; y: number; newType: number };
