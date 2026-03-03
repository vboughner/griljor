import { MapFile, ObjectFile } from './types';
import { Game } from './game';
import { ColorMode } from './assets';
import { GameNetwork } from './network';

const AVATARS = [
  'aaron', 'adriana', 'albert', 'aragorn', 'avatar', 'bh', 'crescendo',
  'crom', 'drustan', 'duel', 'eric', 'gm', 'mahatma', 'mcelhoe', 'mel',
  'mike', 'mikey', 'moronus', 'ollie', 'savaki', 'spook', 'stefan',
  'stinglai', 'trevor', 'van',
];

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
  const avatarSelect = document.getElementById('avatar-select') as HTMLSelectElement;
  const playerNameInput = document.getElementById('player-name') as HTMLInputElement;
  const modeToggle = document.getElementById('mode-toggle') as HTMLButtonElement;
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

  // Populate avatar selector
  for (const name of AVATARS) {
    const opt = document.createElement('option');
    opt.value = name;
    opt.textContent = name;
    avatarSelect.appendChild(opt);
  }
  avatarSelect.value = 'crom';

  let currentGame: Game | null = null;
  let currentMode: ColorMode = 'dark';

  modeToggle.addEventListener('click', async () => {
    currentMode = currentMode === 'dark' ? 'light' : 'dark';
    modeToggle.textContent = currentMode === 'dark' ? '☀ Light' : '☾ Dark';
    document.body.style.background = currentMode === 'dark' ? '#1a1a1a' : '#d0d0d0';
    document.body.style.color = currentMode === 'dark' ? '#ccc' : '#222';
    await currentGame?.setMode(currentMode);
  });

  async function startMap(name: string): Promise<void> {
    currentGame?.destroy();
    currentGame = null;
    status.textContent = `Loading ${name}…`;
    roomInfo.textContent = '';

    try {
      const { mapData, objFile } = await loadMap(name);

      // Try to connect to the game server
      let network: GameNetwork | undefined;
      try {
        network = new GameNetwork('ws://localhost:3001/ws');
      } catch {
        // Server unavailable — single-player mode
      }

      const game = new Game(mapData, objFile, canvas, roomInfo, status, navBtns, network);
      currentGame = game;
      await game.setAvatar(avatarSelect.value);
      await game.goToRoom(0);

      if (network) {
        const playerName = playerNameInput.value.trim() || 'player';
        network.onAccepted = (msg) => {
          game.setMyId(msg.id);
          status.textContent = `Connected as ${playerName} (id=${msg.id})`;
        };
        network.onRejected = (msg) => {
          status.textContent = `Rejected: ${msg.msg}`;
        };
        network.join(playerName, avatarSelect.value);
      }
    } catch (err) {
      status.textContent = `Error: ${err}`;
    }
  }

  mapSelect.addEventListener('change', () => startMap(mapSelect.value));
  avatarSelect.addEventListener('change', () => currentGame?.setAvatar(avatarSelect.value));

  await startMap('battle');
}

main().catch((err) => {
  const status = document.getElementById('status');
  if (status) status.textContent = `Fatal: ${err}`;
  console.error(err);
});
