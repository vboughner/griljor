import { MapFile, ObjectFile } from './types';
import { Game } from './game';
import { ColorMode } from './assets';
import { GameNetwork } from './network';
import { fetchGames, GameInfo } from './lobby';
import { loadMaskedSprite } from './assets';
import { initMouseWidget, setHandItem } from './mouse-widget';

const AVATARS = [
  'aaron', 'adriana', 'albert', 'aragorn', 'avatar', 'bh', 'crescendo',
  'crom', 'drustan', 'duel', 'eric', 'gm', 'mahatma', 'mcelhoe', 'mel',
  'mike', 'mikey', 'moronus', 'ollie', 'savaki', 'spook', 'stefan',
  'stinglai', 'trevor', 'van',
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

function formatAge(ms: number): string {
  const s = ms / 1000;
  const m = s / 60;
  const h = m / 60;
  if (s < 45)   return 'just joined';
  if (s < 90)   return 'a minute';
  if (m < 45)   return `${Math.round(m)} minutes`;
  if (m < 90)   return 'an hour';
  if (h < 22)   return `${Math.round(h)} hours`;
  if (h < 36)   return 'a day';
  return `${Math.round(h / 24)} days`;
}

async function drawAvatarOnCanvas(canvas: HTMLCanvasElement, avatarName: string): Promise<void> {
  const imageData = await loadMaskedSprite(
    `/sprites/facebits/${avatarName}bit.png`,
    `/sprites/facebits/${avatarName}mask.png`,
  );
  const ctx = canvas.getContext('2d')!;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  if (!imageData) return;
  const tmp = new OffscreenCanvas(imageData.width, imageData.height);
  tmp.getContext('2d')!.putImageData(imageData, 0, 0);
  ctx.drawImage(tmp, 0, 0, canvas.width, canvas.height);
}

async function main(): Promise<void> {
  initMouseWidget();

  // DOM refs — lobby
  const lobbyScreen     = document.getElementById('lobby-screen') as HTMLElement;
  const gameScreen      = document.getElementById('game-screen') as HTMLElement;
  const playerNameInput = document.getElementById('player-name') as HTMLInputElement;
  const avatarSelect    = document.getElementById('avatar-select') as HTMLSelectElement;
  const avatarPreview   = document.getElementById('avatar-preview') as HTMLCanvasElement;
  const serverList      = document.getElementById('server-list') as HTMLElement;
  const refreshBtn      = document.getElementById('refresh-btn') as HTMLButtonElement;
  const lobbyStatus     = document.getElementById('lobby-status') as HTMLElement;

  // DOM refs — game
  const canvas     = document.getElementById('game-canvas') as HTMLCanvasElement;
  const roomInfo   = document.getElementById('room-info') as HTMLElement;
  const status     = document.getElementById('status') as HTMLElement;
  const modeToggle = document.getElementById('mode-toggle') as HTMLButtonElement;
  const leaveBtn   = document.getElementById('leave-btn') as HTMLButtonElement;
  const chatLog    = document.getElementById('chat-log') as HTMLElement;
  const chatInput  = document.getElementById('chat-input') as HTMLInputElement;
  const chatSend   = document.getElementById('chat-send') as HTMLButtonElement;
  const playerListEl = document.getElementById('player-list') as HTMLElement;
  const navBtns = {
    north: document.getElementById('btn-north') as HTMLButtonElement,
    east:  document.getElementById('btn-east')  as HTMLButtonElement,
    south: document.getElementById('btn-south') as HTMLButtonElement,
    west:  document.getElementById('btn-west')  as HTMLButtonElement,
  };

  // ── Avatar selector ──────────────────────────────────────────────
  for (const name of AVATARS) {
    const opt = document.createElement('option');
    opt.value = name;
    opt.textContent = name;
    avatarSelect.appendChild(opt);
  }
  const randomAvatar = AVATARS[Math.floor(Math.random() * AVATARS.length)];
  avatarSelect.value = randomAvatar;
  playerNameInput.value = randomAvatar;
  void drawAvatarOnCanvas(avatarPreview, randomAvatar);

  // ── Player list state ────────────────────────────────────────────
  interface PlayerEntry {
    id: number; name: string; avatar: string;
    kills: number; deaths: number; joinedAt: number;
    row: HTMLElement; timeEl: HTMLElement;
  }
  const playerMap = new Map<number, PlayerEntry>();
  let playerTickInterval: ReturnType<typeof setInterval> | null = null;

  function renderPlayerList(): void {
    playerListEl.innerHTML = '';
    for (const p of playerMap.values()) {
      playerListEl.appendChild(p.row);
    }
  }

  async function addPlayerRow(id: number, name: string, avatar: string,
                               kills: number, deaths: number, joinedAt: number): Promise<void> {
    const row = document.createElement('div');
    row.className = 'player-row';

    const avatarCanvas = document.createElement('canvas');
    avatarCanvas.width = 32;
    avatarCanvas.height = 32;
    void drawAvatarOnCanvas(avatarCanvas, avatar);

    const details = document.createElement('div');
    details.className = 'player-details';

    const nameEl = document.createElement('div');
    nameEl.className = 'player-name';
    nameEl.textContent = name;

    const kdEl = document.createElement('div');
    kdEl.className = 'player-kd';
    kdEl.textContent = `K:${kills}  D:${deaths}`;

    const timeEl = document.createElement('div');
    timeEl.className = 'player-time';
    timeEl.textContent = formatAge(Date.now() - joinedAt);

    details.appendChild(nameEl);
    details.appendChild(kdEl);
    details.appendChild(timeEl);
    row.appendChild(avatarCanvas);
    row.appendChild(details);

    playerMap.set(id, { id, name, avatar, kills, deaths, joinedAt, row, timeEl });
    renderPlayerList();
  }

  function updatePlayerStats(id: number, kills: number, deaths: number): void {
    const p = playerMap.get(id);
    if (!p) return;
    p.kills = kills;
    p.deaths = deaths;
    p.row.querySelector<HTMLElement>('.player-kd')!.textContent = `K:${kills}  D:${deaths}`;
  }

  function removePlayer(id: number): void {
    playerMap.delete(id);
    renderPlayerList();
  }

  function clearPlayerList(): void {
    playerMap.clear();
    playerListEl.innerHTML = '';
    if (playerTickInterval !== null) {
      clearInterval(playerTickInterval);
      playerTickInterval = null;
    }
  }

  function startPlayerTick(): void {
    playerTickInterval = setInterval(() => {
      const now = Date.now();
      for (const p of playerMap.values()) {
        p.timeEl.textContent = formatAge(now - p.joinedAt);
      }
    }, 1000);
  }

  // ── Chat ─────────────────────────────────────────────────────────
  function appendChat(name: string, text: string): void {
    const line = document.createElement('div');
    line.className = 'chat-msg';
    const nameSpan = document.createElement('span');
    nameSpan.className = 'chat-name';
    nameSpan.textContent = `${name}: `;
    line.appendChild(nameSpan);
    line.appendChild(document.createTextNode(text));
    chatLog.appendChild(line);
    chatLog.scrollTop = chatLog.scrollHeight;
  }

  function sendChat(): void {
    const text = chatInput.value.trim();
    if (!text || !currentNetwork) return;
    currentNetwork.sendMessage(text);
    chatInput.value = '';
  }

  chatSend.addEventListener('click', sendChat);
  chatInput.addEventListener('keydown', (e) => { if (e.key === 'Enter') sendChat(); });

  // ── State ─────────────────────────────────────────────────────────
  let currentGame: Game | null = null;
  let currentNetwork: GameNetwork | null = null;
  let currentMode: ColorMode = 'dark';
  let isJoining = false;

  function showLobby(): void {
    lobbyScreen.style.display = 'flex';
    gameScreen.style.display = 'none';
  }

  function showGame(): void {
    lobbyScreen.style.display = 'none';
    gameScreen.style.display = 'flex';
  }

  function setInputsDisabled(disabled: boolean): void {
    playerNameInput.disabled = disabled;
    avatarSelect.disabled = disabled;
    serverList.querySelectorAll<HTMLButtonElement>('.join-btn').forEach((btn) => {
      btn.disabled = disabled;
    });
  }

  // ── Lobby ─────────────────────────────────────────────────────────
  async function refreshServerList(): Promise<void> {
    lobbyStatus.textContent = 'Loading\u2026';
    serverList.innerHTML = '';
    let games: GameInfo[];
    try {
      games = await fetchGames();
    } catch (err) {
      lobbyStatus.textContent = `Error: ${err}`;
      return;
    }

    if (games.length === 0) {
      lobbyStatus.textContent = 'No active servers found.';
      return;
    }

    lobbyStatus.textContent = '';
    for (const game of games) {
      const row = document.createElement('div');
      row.className = 'server-row';
      row.innerHTML = `
        <span class="server-map">${game.mapName}</span>
        <span class="server-players">${game.players} / ${game.maxPlayers}</span>
        <button class="join-btn" data-host="${game.host}" data-port="${game.port}">Join</button>
      `;
      row.querySelector<HTMLButtonElement>('.join-btn')!.addEventListener('click', () => joinServer(game));
      serverList.appendChild(row);
    }
  }

  async function joinServer(gameInfo: GameInfo): Promise<void> {
    if (isJoining) return;
    isJoining = true;
    setInputsDisabled(true);
    lobbyStatus.textContent = `Connecting to ${gameInfo.mapName}\u2026`;

    const playerName = playerNameInput.value.trim() || 'player';

    try {
      const { mapData, objFile } = await loadMap(gameInfo.mapName);

      const network = new GameNetwork(`ws://${gameInfo.host}:${gameInfo.port}/ws`);
      currentNetwork = network;

      network.onRejected = (msg) => {
        setInputsDisabled(false);
        lobbyStatus.textContent = `Rejected: ${msg.msg}`;
        isJoining = false;
      };

      network.onMessage = (msg) => {
        if (msg.to === 'all') appendChat(msg.name, msg.text);
      };

      network.onClose = () => {
        if (gameScreen.style.display !== 'none') {
          currentGame?.destroy();
          currentGame = null;
          chatLog.innerHTML = '';
          clearPlayerList();
          showLobby();
          refreshServerList();
        }
        lobbyStatus.textContent = 'Disconnected from server.';
        setInputsDisabled(false);
        isJoining = false;
      };

      const game = new Game(mapData, objFile, canvas, roomInfo, status, navBtns, network);
      currentGame = game;

      // Wrap the callbacks Game.wireNetwork() just installed so both game
      // rendering and the player list stay in sync.
      const gameOnPlayerInfo = network.onPlayerInfo;
      network.onPlayerInfo = async (msg) => {
        await gameOnPlayerInfo(msg);
        if (playerMap.has(msg.id)) {
          updatePlayerStats(msg.id, msg.kills, msg.deaths);
        } else {
          void addPlayerRow(msg.id, msg.name, msg.avatar, msg.kills, msg.deaths, msg.joinedAt);
        }
      };

      const gameOnLeave = network.onLeave;
      network.onLeave = async (msg) => {
        await gameOnLeave(msg);
        removePlayer(msg.id);
      };

      network.onPlayerStats = (msg) => {
        updatePlayerStats(msg.id, msg.kills, msg.deaths);
      };

      network.onAccepted = (msg) => {
        showGame();
        startPlayerTick();
        status.textContent = `Connected as ${playerName} (id=${msg.id})`;
        game.setMyId(msg.id);
        // Server doesn't send PLAYER_INFO for the local player back to themselves
        void addPlayerRow(msg.id, playerName, avatarSelect.value, 0, 0, Date.now());
      };

      await game.setAvatar(avatarSelect.value);
      await game.goToRoom(0);
      network.join(playerName, avatarSelect.value);
    } catch (err) {
      lobbyStatus.textContent = `Error: ${err}`;
      setInputsDisabled(false);
      isJoining = false;
    }
  }

  modeToggle.addEventListener('click', async () => {
    currentMode = currentMode === 'dark' ? 'light' : 'dark';
    modeToggle.textContent = currentMode === 'dark' ? '☀ Light' : '☾ Dark';
    document.body.style.background = currentMode === 'dark' ? '#1a1a1a' : '#d0d0d0';
    document.body.style.color = currentMode === 'dark' ? '#ccc' : '#222';
    await currentGame?.setMode(currentMode);
  });

  leaveBtn.addEventListener('click', () => {
    currentNetwork?.sendLeave();
    currentGame?.destroy();
    currentGame = null;
    currentNetwork = null;
    isJoining = false;
    chatLog.innerHTML = '';
    clearPlayerList();
    setInputsDisabled(false);
    showLobby();
    refreshServerList();
  });

  avatarSelect.addEventListener('change', () => {
    playerNameInput.value = avatarSelect.value;
    void drawAvatarOnCanvas(avatarPreview, avatarSelect.value);
    currentGame?.setAvatar(avatarSelect.value);
  });

  refreshBtn.addEventListener('click', () => refreshServerList());

  showLobby();
  await refreshServerList();
}

export function setHandItems(left: string | null, right: string | null): void {
  setHandItem('left', left);
  setHandItem('right', right);
}

main().catch((err) => {
  const el = document.getElementById('lobby-status') ?? document.getElementById('status');
  if (el) el.textContent = `Fatal: ${err}`;
  console.error(err);
});
