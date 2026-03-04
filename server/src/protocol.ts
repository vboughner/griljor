export interface InventoryItem {
  type: number;
  quantity: number;
}

// Client → Server messages
export type C2SMessage =
  | { type: 'JOIN';         name: string; avatar: string }
  | { type: 'MY_LOCATION';  room: number; x: number; y: number }
  | { type: 'LEAVING_GAME' }
  | { type: 'MESSAGE';      to: number | 'all'; text: string }
  | { type: 'PICKUP';       x: number; y: number; hand: 'left' | 'right' }
  | { type: 'DROP';         source: 'left' | 'right' | number }
  | { type: 'INV_SWAP';     slot: number; hand: 'left' | 'right' };

// Server → Client messages
export type S2CMessage =
  | { type: 'ACCEPTED';     id: number; msg: string; mapName: string; rooms: number }
  | { type: 'REJECTED';     msg: string }
  | { type: 'PLAYER_INFO';  id: number; name: string; avatar: string;
                            room: number; x: number; y: number;
                            kills: number; deaths: number; joinedAt: number }
  | { type: 'PLAYER_STATS'; id: number; kills: number; deaths: number }
  | { type: 'MY_LOCATION';  id: number; room: number; x: number; y: number }
  | { type: 'LEAVING_GAME'; id: number }
  | { type: 'MESSAGE';      from: number; name: string; to: number | 'all'; text: string }
  | { type: 'ITEM_REMOVED'; room: number; x: number; y: number }
  | { type: 'ITEM_ADDED';   room: number; x: number; y: number; item: InventoryItem }
  | { type: 'YOUR_INVENTORY';
      leftHand: InventoryItem | null;
      rightHand: InventoryItem | null;
      inventory: Array<InventoryItem | null>;
      currentWeight: number; maxWeight: number }
  | { type: 'ITEMS_SYNC';
      items: Array<{ room: number; x: number; y: number; item: InventoryItem }> };
