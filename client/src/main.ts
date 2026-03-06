import { MapFile, ObjectFile, ObjDef, InventoryItem } from './types';
import { Game } from './game';
import { ColorMode } from './assets';
import { GameNetwork } from './network';
import { fetchGames, watchGames, GameInfo } from './lobby';
import { loadMaskedSprite, loadSprite } from './assets';
import { initMouseWidget, setHandItem } from './mouse-widget';
import { runTitleScreen } from './title';
import { showTooltip, hideTooltip, moveTooltip, buildItemHtml } from './tooltip';

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

const INV_SIZE = 35;

// Inventory panel state
let invObjects: ObjDef[] = [];
let invObjset = '';
let invNetwork: GameNetwork | null = null;

// Track current hand items for tooltips
let currentLeftHand:  InventoryItem | null = null;
let currentRightHand: InventoryItem | null = null;
// Per-slot item for tooltips (indexed 0..INV_SIZE-1)
const slotItems: Array<InventoryItem | null> = [];

window.addEventListener('mousemove', (e) => moveTooltip(e.clientX, e.clientY));

function buildInvGrid(): void {
  const grid = document.getElementById('inv-grid')!;
  grid.innerHTML = '';
  for (let i = 0; i < INV_SIZE; i++) {
    const cell = document.createElement('div');
    cell.className = 'inv-cell';
    cell.dataset['slot'] = String(i);

    const c = document.createElement('canvas');
    c.width = 32;
    c.height = 32;
    cell.appendChild(c);

    const countSpan = document.createElement('span');
    countSpan.className = 'inv-count';
    cell.appendChild(countSpan);

    cell.addEventListener('mousedown', (e) => {
      e.preventDefault();
      if (!invNetwork) return;
      if (e.button === 0) {
        // Left click: swap slot with left hand
        invNetwork.sendInvSwap(i, 'left');
      } else if (e.button === 1) {
        // Middle click: swap slot with right hand
        invNetwork.sendInvSwap(i, 'right');
      }
    });
    cell.addEventListener('contextmenu', (e) => {
      e.preventDefault();
      if (!invNetwork) return;
      // Right click: drop item in slot
      invNetwork.sendDrop(i);
    });

    cell.addEventListener('mouseenter', (e) => {
      const item = slotItems[i];
      if (!item) return;
      const obj = invObjects[item.type];
      if (!obj) return;
      showTooltip(buildItemHtml(obj, item), (e as MouseEvent).clientX, (e as MouseEvent).clientY);
    });
    cell.addEventListener('mouseleave', () => hideTooltip());

    grid.appendChild(cell);
  }
}

function initHandTooltips(): void {
  const hands: Array<{ id: string; getItem: () => InventoryItem | null }> = [
    { id: 'hand-left-canvas',   getItem: () => currentLeftHand  },
    { id: 'hand-middle-canvas', getItem: () => currentRightHand },
  ];
  for (const { id, getItem } of hands) {
    const canvas = document.getElementById(id)!;
    canvas.addEventListener('mouseenter', (e) => {
      const item = getItem();
      if (!item) return;
      const obj = invObjects[item.type];
      if (!obj) return;
      showTooltip(buildItemHtml(obj, item), (e as MouseEvent).clientX, (e as MouseEvent).clientY);
    });
    canvas.addEventListener('mouseleave', () => hideTooltip());
  }
}

async function getItemImgData(item: InventoryItem): Promise<ImageData | null> {
  const obj = invObjects[item.type];
  if (!obj?.bitmap) return null;
  const baseUrl = `/data/objects/bitmaps/${invObjset}/${obj.bitmap}`;
  if (obj.masked && obj.mask) {
    return loadMaskedSprite(baseUrl, `/data/objects/bitmaps/${invObjset}/${obj.mask}`);
  }
  return loadSprite(baseUrl);
}

async function drawItemOnCanvas(canvas: HTMLCanvasElement, item: InventoryItem): Promise<void> {
  const imgData = await getItemImgData(item);
  const ctx = canvas.getContext('2d')!;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  if (!imgData) return;
  const tmp = new OffscreenCanvas(imgData.width, imgData.height);
  tmp.getContext('2d')!.putImageData(imgData, 0, 0);
  ctx.drawImage(tmp, 0, 0, canvas.width, canvas.height);
}

async function updateInventoryPanel(msg: {
  leftHand: InventoryItem | null;
  rightHand: InventoryItem | null;
  inventory: Array<InventoryItem | null>;
  currentWeight: number;
  maxWeight: number;
}): Promise<void> {
  // Track hand items for tooltips
  currentLeftHand  = msg.leftHand;
  currentRightHand = msg.rightHand;

  // Update weight display
  const weightEl = document.getElementById('inv-weight');
  if (weightEl) weightEl.textContent = `${msg.currentWeight} / ${msg.maxWeight}`;

  // Update hand slot icons
  setHandItems(
    msg.leftHand  ? await getItemImgData(msg.leftHand)  : null,
    msg.rightHand ? await getItemImgData(msg.rightHand) : null,
  );

  // Update hand slot click handlers (set once via data attr trick, easier to redo here)
  const handLeftCanvas = document.getElementById('hand-left-canvas') as HTMLCanvasElement;
  const handMiddleCanvas = document.getElementById('hand-middle-canvas') as HTMLCanvasElement;
  if (handLeftCanvas) {
    handLeftCanvas.onclick = () => {
      if (msg.leftHand && invNetwork) invNetwork.sendDrop('left');
    };
  }
  if (handMiddleCanvas) {
    handMiddleCanvas.onclick = () => {
      if (msg.rightHand && invNetwork) invNetwork.sendDrop('right');
    };
  }

  // Update each inventory slot
  const grid = document.getElementById('inv-grid');
  if (!grid) return;
  const cells = grid.querySelectorAll<HTMLElement>('.inv-cell');
  for (let i = 0; i < INV_SIZE; i++) {
    const cell = cells[i];
    if (!cell) continue;
    const canvas = cell.querySelector('canvas') as HTMLCanvasElement;
    const countSpan = cell.querySelector('.inv-count') as HTMLElement;
    const item = msg.inventory[i];
    slotItems[i] = item ?? null;
    const ctx = canvas.getContext('2d')!;
    ctx.clearRect(0, 0, 32, 32);
    countSpan.textContent = '';
    if (item) {
      await drawItemOnCanvas(canvas, item);
      const obj = invObjects[item.type];
      if (obj?.numbered && item.quantity > 1) {
        countSpan.textContent = String(item.quantity);
      }
    }
  }
}

async function main(): Promise<void> {
  initMouseWidget();
  buildInvGrid();
  initHandTooltips();

  // DOM refs — title
  const titleScreen  = document.getElementById('title-screen') as HTMLElement;
  const titleCanvas  = document.getElementById('title-canvas') as HTMLCanvasElement;

  // DOM refs — lobby
  const lobbyScreen     = document.getElementById('lobby-screen') as HTMLElement;
  const gameScreen      = document.getElementById('game-screen') as HTMLElement;
  const playerNameInput = document.getElementById('player-name') as HTMLInputElement;
  const avatarSelect    = document.getElementById('avatar-select') as HTMLSelectElement;
  const avatarPreview   = document.getElementById('avatar-preview') as HTMLCanvasElement;
  const serverList      = document.getElementById('server-list') as HTMLElement;
  const lobbyStatus     = document.getElementById('lobby-status') as HTMLElement;
  const lobbyUpdated    = document.getElementById('lobby-updated') as HTMLElement;

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

  // ── Stats panel ──────────────────────────────────────────────────
  function updateStats(hp: number, maxHp: number, power: number, maxPower: number, xp: number, level: number): void {
    const hpBar  = document.getElementById('hp-bar');
    const mpBar  = document.getElementById('mp-bar');
    const hpText = document.getElementById('hp-text');
    const mpText = document.getElementById('mp-text');
    const xpLine = document.getElementById('xp-line');
    if (hpBar)  hpBar.style.width  = `${Math.max(0, (hp / maxHp) * 100)}%`;
    if (mpBar)  mpBar.style.width  = `${Math.max(0, (power / maxPower) * 100)}%`;
    if (hpText) hpText.textContent = `${hp}/${maxHp}`;
    if (mpText) mpText.textContent = `${power}/${maxPower}`;
    if (xpLine) xpLine.textContent = `Lvl ${level} · XP: ${xp}`;
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

  function appendReport(text: string): void {
    const line = document.createElement('div');
    line.className = 'chat-msg chat-report';
    line.textContent = `* ${text}`;
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
  chatInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') sendChat();
    else if (e.key === 'Escape') chatInput.blur();
  });

  window.addEventListener('keydown', (e) => {
    if (gameScreen.style.display === 'none') return;
    const tag = (document.activeElement as HTMLElement)?.tagName;
    if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;
    if (e.key === 't') { e.preventDefault(); chatInput.focus(); }
  });

  // ── State ─────────────────────────────────────────────────────────
  let currentGame: Game | null = null;
  let currentNetwork: GameNetwork | null = null;
  let currentMode: ColorMode = 'dark';
  let isJoining = false;
  let lobbyRefreshTimer: ReturnType<typeof setInterval> | null = null;
  let lobbyWatcher: WebSocket | null = null;

  function showTitle(): void {
    titleScreen.style.display = 'flex';
    lobbyScreen.style.display = 'none';
    gameScreen.style.display = 'none';
  }

  function hideTitle(): void {
    titleScreen.style.display = 'none';
  }

  function startLobbyWatcher(): void {
    if (lobbyWatcher) return;
    lobbyWatcher = watchGames(
      (games) => renderServerList(games),
      () => {
        // WebSocket failed — fall back to polling
        lobbyWatcher = null;
        if (!lobbyRefreshTimer) {
          lobbyRefreshTimer = setInterval(() => { void refreshServerList(); }, 20000);
        }
      },
    );
    lobbyWatcher.addEventListener('close', () => {
      lobbyWatcher = null;
    });
  }

  function stopLobbyWatcher(): void {
    if (lobbyWatcher) { lobbyWatcher.close(); lobbyWatcher = null; }
    if (lobbyRefreshTimer) { clearInterval(lobbyRefreshTimer); lobbyRefreshTimer = null; }
  }

  function showLobby(): void {
    lobbyScreen.style.display = 'flex';
    gameScreen.style.display = 'none';
    startLobbyWatcher();
  }

  function showGame(): void {
    lobbyScreen.style.display = 'none';
    gameScreen.style.display = 'flex';
    stopLobbyWatcher();
  }

  function setInputsDisabled(disabled: boolean): void {
    playerNameInput.disabled = disabled;
    avatarSelect.disabled = disabled;
    serverList.querySelectorAll<HTMLButtonElement>('.join-btn').forEach((btn) => {
      btn.disabled = disabled;
    });
  }

  // ── Lobby ─────────────────────────────────────────────────────────
  let lastGames: GameInfo[] = [];

  function updateJoinButtons(): void {
    const selected = avatarSelect.value;
    for (const btn of serverList.querySelectorAll<HTMLButtonElement>('.join-btn')) {
      const taken = (btn.dataset.avatars ?? '').split(',');
      const full = btn.dataset.full === 'true';
      btn.disabled = full || taken.includes(selected);
    }
  }

  function renderServerList(games: GameInfo[]): void {
    lastGames = games;
    serverList.innerHTML = '';
    const time = new Date().toLocaleTimeString(undefined, { hour: 'numeric', minute: '2-digit' });
    lobbyUpdated.textContent = `updated ${time}`;
    if (games.length === 0) {
      lobbyStatus.textContent = 'No active servers found.';
      return;
    }
    lobbyStatus.textContent = '';
    const header = document.createElement('div');
    header.className = 'server-header';
    header.innerHTML = `
      <span class="server-map">Map</span>
      <span class="server-avatars-hdr">Players</span>
      <span class="server-count-hdr">Count</span>
      <span class="server-teams-hdr">Teams</span>
      <span class="server-rooms-hdr">Rooms</span>
      <span class="server-join-hdr"></span>
    `;
    serverList.appendChild(header);
    for (const game of games) {
      const row = document.createElement('div');
      row.className = 'server-row';
      const avatarKeys = (game.avatars ?? []).map((a) => a.avatar).join(',');
      const full = game.players >= game.maxPlayers;
      const teamsVal = (game.teams ?? 0) === 0 ? 'FFA' : String(game.teams);
      row.innerHTML = `
        <span class="server-map">${game.title ?? game.mapName}</span>
        <span class="server-avatars"></span>
        <span class="server-players">${game.players}/${game.maxPlayers}</span>
        <span class="server-teams">${teamsVal}</span>
        <span class="server-rooms">${game.rooms ?? '?'}</span>
        <button class="join-btn" data-host="${game.host}" data-port="${game.port}" data-avatars="${avatarKeys}" data-full="${full}">Join</button>
      `;
      const avatarStrip = row.querySelector<HTMLElement>('.server-avatars')!;
      for (const entry of (game.avatars ?? [])) {
        const c = document.createElement('canvas');
        c.width = 32;
        c.height = 32;
        void drawAvatarOnCanvas(c, entry.avatar);
        c.addEventListener('mouseenter', (e) => showTooltip(`<div class="tip-name">${entry.name}</div>`, e.clientX, e.clientY));
        c.addEventListener('mousemove',  (e) => moveTooltip(e.clientX, e.clientY));
        c.addEventListener('mouseleave', () => hideTooltip());
        avatarStrip.appendChild(c);
      }
      row.querySelector<HTMLButtonElement>('.join-btn')!.addEventListener('click', () => joinServer(game));
      serverList.appendChild(row);
    }
    updateJoinButtons();
  }

  async function refreshServerList(): Promise<void> {
    let games: GameInfo[];
    try {
      games = await fetchGames();
    } catch (err) {
      lobbyStatus.textContent = `Error: ${err}`;
      return;
    }
    renderServerList(games);
  }

  async function joinServer(gameInfo: GameInfo): Promise<void> {
    if (isJoining) return;
    isJoining = true;
    setInputsDisabled(true);
    lobbyStatus.textContent = `Connecting to ${gameInfo.mapName}\u2026`;

    const playerName = playerNameInput.value.trim() || 'player';

    try {
      const { mapData, objFile } = await loadMap(gameInfo.mapName);

      // Set up inventory object references for the panel
      invObjects = objFile.objects;
      invObjset = mapData.map.objfilename.replace(/\.obj$/, '');

      const network = new GameNetwork(`ws://${gameInfo.host}:${gameInfo.port}/ws`);
      currentNetwork = network;
      invNetwork = network;

      network.onRejected = (msg) => {
        setInputsDisabled(false);
        lobbyStatus.textContent = `Rejected: ${msg.msg}`;
        isJoining = false;
      };

      network.onMessage = (msg) => {
        if (msg.to === 'all') {
          appendChat(msg.name, msg.text);
        } else if (msg.from === 0) {
          // Private GM message (warnings like "too heavy", "hands full")
          appendReport(msg.text);
        }
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
        invNetwork = null;
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

      network.onInventory = (msg) => {
        void updateInventoryPanel(msg);
        game.setHands(msg.leftHand, msg.rightHand);
      };

      network.onYourStats = (msg) => {
        updateStats(msg.hp, msg.maxHp, msg.power, msg.maxPower, msg.xp, msg.level);
      };

      network.onReport = (msg) => {
        appendReport(msg.text);
      };

      network.onYouDied = (msg) => {
        appendReport(`You were slain by ${msg.killerName}. Respawning…`);
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
    invNetwork = null;
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
    updateJoinButtons();
  });


  showTitle();
  void refreshServerList(); // start fetching in background during title
  await runTitleScreen(titleCanvas);
  hideTitle();
  showLobby();
}

export function setHandItems(left: ImageData | null, right: ImageData | null): void {
  setHandItem('left', left);
  setHandItem('right', right);
}

main().catch((err) => {
  const el = document.getElementById('lobby-status') ?? document.getElementById('status');
  if (el) el.textContent = `Fatal: ${err}`;
  console.error(err);
});
