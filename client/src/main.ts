import { MapFile, ObjectFile } from './types';
import { Game } from './game';

const MAPS = [
  'battle', 'blowup', 'castle', 'default', 'flag', 'flames', 'flash',
  'hack', 'hack1', 'hometown', 'ivarr', 'main', 'outdoor', 'paradise2',
  'paradise3', 'ring', 'shelter', 'shooter', 'standard', 'sword', 'three',
  'title', 'trek', 'tunnel', 'two', 'twoperson',
];

async function loadMap(name: string): Promise<{ mapData: MapFile; objFile: ObjectFile }> {
  const mapResp = await fetch(`/data/maps/${name}.json`);
  if (!mapResp.ok) throw new Error(`Failed to load map: ${mapResp.status}`);
  const mapData = await mapResp.json() as MapFile;

  const objName = mapData.map.objfilename.replace(/\.obj$/, '');
  const objResp = await fetch(`/data/objects/${objName}.json`);
  if (!objResp.ok) throw new Error(`Failed to load objects: ${objResp.status}`);
  const objFile = await objResp.json() as ObjectFile;

  return { mapData, objFile };
}

async function main(): Promise<void> {
  const canvas = document.getElementById('game-canvas') as HTMLCanvasElement;
  const roomInfo = document.getElementById('room-info') as HTMLElement;
  const status = document.getElementById('status') as HTMLElement;
  const mapSelect = document.getElementById('map-select') as HTMLSelectElement;
  const navBtns = {
    north: document.getElementById('btn-north') as HTMLButtonElement,
    east:  document.getElementById('btn-east')  as HTMLButtonElement,
    south: document.getElementById('btn-south') as HTMLButtonElement,
    west:  document.getElementById('btn-west')  as HTMLButtonElement,
  };

  // Populate map selector
  for (const name of MAPS) {
    const opt = document.createElement('option');
    opt.value = name;
    opt.textContent = name;
    mapSelect.appendChild(opt);
  }
  mapSelect.value = 'battle';

  let currentGame: Game | null = null;

  async function startMap(name: string): Promise<void> {
    currentGame?.destroy();
    currentGame = null;
    status.textContent = `Loading ${name}…`;
    roomInfo.textContent = '';

    try {
      const { mapData, objFile } = await loadMap(name);
      const game = new Game(mapData, objFile, canvas, roomInfo, status, navBtns);
      currentGame = game;
      await game.loadPlayerSprite();
      await game.goToRoom(0);
    } catch (err) {
      status.textContent = `Error: ${err}`;
    }
  }

  mapSelect.addEventListener('change', () => startMap(mapSelect.value));

  await startMap('battle');
}

main().catch((err) => {
  const status = document.getElementById('status');
  if (status) status.textContent = `Fatal: ${err}`;
  console.error(err);
});
