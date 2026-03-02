import { MapFile, ObjectFile, ObjDef } from './types';
import { loadMaskedSprite } from './assets';
import { preloadRoomSprites, renderRoom } from './renderer';

export class Game {
  private mapData: MapFile;
  private objects: ObjDef[];
  private objset: string;
  private currentRoom: number = 0;
  private playerSprite: ImageData | null = null;
  private canvas: HTMLCanvasElement;
  private roomInfo: HTMLElement;
  private status: HTMLElement;
  private navBtns: Record<string, HTMLButtonElement>;

  constructor(
    mapData: MapFile,
    objFile: ObjectFile,
    canvas: HTMLCanvasElement,
    roomInfo: HTMLElement,
    status: HTMLElement,
    navBtns: Record<string, HTMLButtonElement>
  ) {
    this.mapData = mapData;
    this.objects = objFile.objects;
    // Derive objset name from objfilename: "default.obj" → "default"
    this.objset = mapData.map.objfilename.replace(/\.obj$/, '');
    this.canvas = canvas;
    this.roomInfo = roomInfo;
    this.status = status;
    this.navBtns = navBtns;
  }

  async loadPlayerSprite(): Promise<void> {
    this.playerSprite = await loadMaskedSprite(
      '/sprites/facebits/crombit.png',
      '/sprites/facebits/crommask.png'
    );
  }

  async goToRoom(index: number): Promise<void> {
    if (index < 0 || index >= this.mapData.rooms.length) return;
    this.currentRoom = index;
    await this.render();
  }

  private async render(): Promise<void> {
    const room = this.mapData.rooms[this.currentRoom];
    this.status.textContent = 'Loading room sprites…';
    await preloadRoomSprites(room, this.objects, this.objset);
    this.status.textContent = 'Rendering…';
    await renderRoom(this.canvas, room, this.objects, this.objset, this.playerSprite);
    this.roomInfo.textContent =
      `Room ${this.currentRoom}: "${room.name}" — Team: ${room.team}`;
    this.updateNav(room);
    this.status.textContent = '';
  }

  private updateNav(room: { exit_north: number; exit_east: number; exit_south: number; exit_west: number }): void {
    this.navBtns['north'].disabled = room.exit_north < 0;
    this.navBtns['east'].disabled  = room.exit_east  < 0;
    this.navBtns['south'].disabled = room.exit_south < 0;
    this.navBtns['west'].disabled  = room.exit_west  < 0;

    this.navBtns['north'].onclick = () => this.goToRoom(room.exit_north);
    this.navBtns['east'].onclick  = () => this.goToRoom(room.exit_east);
    this.navBtns['south'].onclick = () => this.goToRoom(room.exit_south);
    this.navBtns['west'].onclick  = () => this.goToRoom(room.exit_west);
  }
}
