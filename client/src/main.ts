import { MapFile, ObjectFile } from './types';
import { Game } from './game';

async function main(): Promise<void> {
  const canvas = document.getElementById('game-canvas') as HTMLCanvasElement;
  const roomInfo = document.getElementById('room-info') as HTMLElement;
  const status = document.getElementById('status') as HTMLElement;
  const navBtns = {
    north: document.getElementById('btn-north') as HTMLButtonElement,
    east:  document.getElementById('btn-east')  as HTMLButtonElement,
    south: document.getElementById('btn-south') as HTMLButtonElement,
    west:  document.getElementById('btn-west')  as HTMLButtonElement,
  };

  status.textContent = 'Fetching map data…';

  let mapData: MapFile;
  let objFile: ObjectFile;

  try {
    const mapResp = await fetch('/data/maps/battle.json');

    if (!mapResp.ok) throw new Error(`Failed to load map: ${mapResp.status}`);
    mapData = await mapResp.json() as MapFile;

    // Derive object filename from map metadata
    const objName = mapData.map.objfilename.replace(/\.obj$/, '');
    const objResp = await fetch(`/data/objects/${objName}.json`);
    if (!objResp.ok) throw new Error(`Failed to load objects: ${objResp.status}`);
    objFile = await objResp.json() as ObjectFile;
  } catch (err) {
    status.textContent = `Error: ${err}`;
    return;
  }

  status.textContent = 'Loading player sprite…';

  const game = new Game(mapData, objFile, canvas, roomInfo, status, navBtns);
  await game.loadPlayerSprite();
  await game.goToRoom(0);
}

main().catch((err) => {
  const status = document.getElementById('status');
  if (status) status.textContent = `Fatal: ${err}`;
  console.error(err);
});
