import { MapFile, ObjectFile, ObjDef, InventoryItem } from './types';
import { Game } from './game';
import { ColorMode } from './assets';
import { GameNetwork } from './network';
import { fetchGames, watchGames, GameInfo } from './lobby';
import { loadMaskedSprite, loadSprite } from './assets';
import { initMouseWidget, setHandItem } from './mouse-widget';
import { runTitleScreen, drawLogo } from './title';
import { showTooltip, hideTooltip, moveTooltip, buildItemHtml } from './tooltip';
import { formatAge } from './utils';

const TOMBSTONE_BIT = '/sprites/bitmaps/tombbit.png';
const TOMBSTONE_MASK = '/sprites/bitmaps/tombmask.png';

const AVATARS = [
  'aaron',
  'adriana',
  'albert',
  'aragorn',
  'avatar',
  'bh',
  'crescendo',
  'crom',
  'drustan',
  'duel',
  'eric',
  'gm',
  'mahatma',
  'mcelhoe',
  'mel',
  'mike',
  'mikey',
  'moronus',
  'ollie',
  'savaki',
  'spook',
  'stefan',
  'stinglai',
  'trevor',
  'van',
];

async function loadMap(name: string): Promise<{ mapData: MapFile; objFile: ObjectFile }> {
  const mapResp = await fetch(`/data/maps/${name}.json`);
  if (!mapResp.ok) throw new Error(`Failed to load map: ${mapResp.status}`);
  const mapData = (await mapResp.json()) as MapFile;

  const objName = mapData.map.objfilename.replace(/\.obj$/, '');
  const objResp = await fetch(`/data/objects/${objName}.json`);
  if (!objResp.ok) throw new Error(`Failed to load objects: ${objResp.status}`);
  const objFile = (await objResp.json()) as ObjectFile;

  return { mapData, objFile };
}

async function drawImageDataOnCanvas(
  canvas: HTMLCanvasElement,
  imgData: ImageData | null,
): Promise<void> {
  const ctx = canvas.getContext('2d')!;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  if (!imgData) return;
  const tmp = new OffscreenCanvas(imgData.width, imgData.height);
  tmp.getContext('2d')!.putImageData(imgData, 0, 0);
  ctx.drawImage(tmp, 0, 0, canvas.width, canvas.height);
}

async function drawAvatarOnCanvas(canvas: HTMLCanvasElement, avatarName: string): Promise<void> {
  const imgData = await loadMaskedSprite(
    `/sprites/facebits/${avatarName}bit.png`,
    `/sprites/facebits/${avatarName}mask.png`,
  );
  await drawImageDataOnCanvas(canvas, imgData);
}

const INV_SIZE = 35;

// Inventory panel state
let invObjects: ObjDef[] = [];
let invObjset = '';
let invNetwork: GameNetwork | null = null;

// Track current hand items for tooltips
let currentLeftHand: InventoryItem | null = null;
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
    { id: 'hand-left-canvas', getItem: () => currentLeftHand },
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
  await drawImageDataOnCanvas(canvas, await getItemImgData(item));
}

async function updateInventoryPanel(msg: {
  leftHand: InventoryItem | null;
  rightHand: InventoryItem | null;
  inventory: Array<InventoryItem | null>;
  currentWeight: number;
  maxWeight: number;
}): Promise<void> {
  // Track hand items for tooltips
  currentLeftHand = msg.leftHand;
  currentRightHand = msg.rightHand;

  // Update weight display and burden bar
  const weightEl = document.getElementById('inv-weight');
  const burdenFill = document.getElementById('burden-fill');
  if (weightEl) weightEl.textContent = `${msg.currentWeight}/${msg.maxWeight}`;
  if (burdenFill)
    burdenFill.style.width = `${Math.min(100, (msg.currentWeight / msg.maxWeight) * 100)}%`;

  // Update hand slot icons (fetched in parallel)
  const [leftImg, rightImg] = await Promise.all([
    msg.leftHand ? getItemImgData(msg.leftHand) : Promise.resolve(null),
    msg.rightHand ? getItemImgData(msg.rightHand) : Promise.resolve(null),
  ]);
  setHandItems(leftImg, rightImg);

  // Update hand slot charge counts for numbered weapons
  const handLeftCount = document.getElementById('hand-left-count');
  const handMidCount = document.getElementById('hand-middle-count');
  if (handLeftCount) {
    const obj = msg.leftHand ? invObjects[msg.leftHand.type] : null;
    handLeftCount.textContent = obj?.numbered && msg.leftHand ? String(msg.leftHand.quantity) : '';
  }
  if (handMidCount) {
    const obj = msg.rightHand ? invObjects[msg.rightHand.type] : null;
    handMidCount.textContent = obj?.numbered && msg.rightHand ? String(msg.rightHand.quantity) : '';
  }

  // Update hand slot click handlers (set once via data attr trick, easier to redo here)
  const handLeftCanvas = document.getElementById('hand-left-canvas') as HTMLCanvasElement;
  const handMiddleCanvas = document.getElementById('hand-middle-canvas') as HTMLCanvasElement;
  if (handLeftCanvas) {
    handLeftCanvas.onclick = () => {
      if (msg.leftHand && invNetwork) invNetwork.sendDrop('left');
    };
    handLeftCanvas.oncontextmenu = (e) => {
      e.preventDefault();
      if (msg.leftHand && invNetwork) invNetwork.sendDrop('left');
    };
  }
  if (handMiddleCanvas) {
    handMiddleCanvas.onclick = () => {
      if (msg.rightHand && invNetwork) invNetwork.sendDrop('right');
    };
    handMiddleCanvas.oncontextmenu = (e) => {
      e.preventDefault();
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
  const titleScreen = document.getElementById('title-screen') as HTMLElement;
  const titleCanvas = document.getElementById('title-canvas') as HTMLCanvasElement;

  // DOM refs — lobby
  const lobbyScreen = document.getElementById('lobby-screen') as HTMLElement;
  const gameScreen = document.getElementById('game-screen') as HTMLElement;
  const playerNameInput = document.getElementById('player-name') as HTMLInputElement;
  const avatarPreview = document.getElementById('avatar-preview') as HTMLCanvasElement;
  const avatarDropdown = document.getElementById('avatar-dropdown') as HTMLElement;
  const serverList = document.getElementById('server-list') as HTMLElement;
  const lobbyStatus = document.getElementById('lobby-status') as HTMLElement;

  // DOM refs — game
  const canvas = document.getElementById('game-canvas') as HTMLCanvasElement;
  const roomInfo = document.getElementById('room-label') as HTMLElement;
  const mapLabel = document.getElementById('map-label') as HTMLElement;
  const status = document.getElementById('status') as HTMLElement;
  const leaveBtn = document.getElementById('leave-btn') as HTMLButtonElement;
  const respawnBtn = document.getElementById('respawn-btn') as HTMLButtonElement;
  const chatLog = document.getElementById('chat-log') as HTMLElement;
  const chatInput = document.getElementById('chat-input') as HTMLInputElement;
  const chatSend = document.getElementById('chat-send') as HTMLButtonElement;
  const playerListEl = document.getElementById('player-list') as HTMLElement;
  const navBtns = {
    north: document.getElementById('btn-north') as HTMLButtonElement,
    east: document.getElementById('btn-east') as HTMLButtonElement,
    south: document.getElementById('btn-south') as HTMLButtonElement,
    west: document.getElementById('btn-west') as HTMLButtonElement,
  };

  // ── State ─────────────────────────────────────────────────────────
  let currentGame: Game | null = null;
  let currentNetwork: GameNetwork | null = null;
  let currentMode: ColorMode = 'dark';
  let isJoining = false;
  let lobbyRefreshTimer: ReturnType<typeof setInterval> | null = null;
  let lobbyWatcher: WebSocket | null = null;

  // ── Avatar picker ─────────────────────────────────────────────────
  let selectedAvatar = AVATARS[Math.floor(Math.random() * AVATARS.length)];
  let nameManuallyEdited = false;

  function setSelectedAvatar(name: string): void {
    selectedAvatar = name;
    void drawAvatarOnCanvas(avatarPreview, name);
    if (!nameManuallyEdited) playerNameInput.value = name;
    currentGame?.setAvatar(name);
    updateJoinButtons();
  }

  for (const name of AVATARS) {
    const c = document.createElement('canvas');
    c.width = 32;
    c.height = 32;
    c.title = name;
    void drawAvatarOnCanvas(c, name);
    c.addEventListener('click', (e) => {
      e.stopPropagation();
      setSelectedAvatar(name);
    });
    avatarDropdown.appendChild(c);
  }

  playerNameInput.addEventListener('input', () => {
    if (playerNameInput.value.trim() === '') {
      nameManuallyEdited = false;
    } else {
      nameManuallyEdited = true;
    }
  });

  setSelectedAvatar(selectedAvatar);
  playerNameInput.value = selectedAvatar;

  document.getElementById('random-avatar-btn')!.addEventListener('click', () => {
    const others = AVATARS.filter((a) => a !== selectedAvatar);
    setSelectedAvatar(others[Math.floor(Math.random() * others.length)]);
  });

  // ── Player list state ────────────────────────────────────────────
  interface PlayerEntry {
    id: number;
    name: string;
    avatar: string;
    kills: number;
    deaths: number;
    joinedAt: number;
    dead: boolean;
    hp: number;
    maxHp: number;
    row: HTMLElement;
    timeEl: HTMLElement;
    avatarCanvas: HTMLCanvasElement;
    hpFill: HTMLElement;
  }
  const playerMap = new Map<number, PlayerEntry>();

  async function setPlayerDeadDisplay(id: number, dead: boolean): Promise<void> {
    const p = playerMap.get(id);
    if (!p || p.dead === dead) return; // skip if unchanged
    p.dead = dead;
    if (dead) {
      await drawImageDataOnCanvas(
        p.avatarCanvas,
        await loadMaskedSprite(TOMBSTONE_BIT, TOMBSTONE_MASK),
      );
    } else {
      await drawAvatarOnCanvas(p.avatarCanvas, p.avatar);
    }
  }
  let playerTickInterval: ReturnType<typeof setInterval> | null = null;

  function renderPlayerList(): void {
    playerListEl.innerHTML = '';
    for (const p of playerMap.values()) {
      playerListEl.appendChild(p.row);
    }
  }

  async function addPlayerRow(
    id: number,
    name: string,
    avatar: string,
    kills: number,
    deaths: number,
    joinedAt: number,
  ): Promise<void> {
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

    const hpTrack = document.createElement('div');
    hpTrack.className = 'pl-hp-track';
    const hpFill = document.createElement('div');
    hpFill.className = 'pl-hp-fill';
    hpFill.style.width = '100%';
    hpFill.style.background = '#3a3';
    hpTrack.appendChild(hpFill);

    details.appendChild(nameEl);
    details.appendChild(hpTrack);
    details.appendChild(kdEl);
    details.appendChild(timeEl);
    row.appendChild(avatarCanvas);
    row.appendChild(details);

    playerMap.set(id, {
      id,
      name,
      avatar,
      kills,
      deaths,
      joinedAt,
      dead: false,
      hp: 100,
      maxHp: 100,
      row,
      timeEl,
      avatarCanvas,
      hpFill,
    });
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

  function hpBarColor(pct: number): string {
    return pct > 66 ? '#3a3' : pct > 33 ? '#aa3' : '#a33';
  }

  // ── Stats panel ──────────────────────────────────────────────────
  function updateStats(hp: number, maxHp: number): void {
    const hpBar = document.getElementById('hp-bar');
    const hpText = document.getElementById('hp-text');
    if (hpBar) {
      const pct = Math.max(0, (hp / maxHp) * 100);
      hpBar.style.width = `${pct}%`;
      hpBar.style.background = hpBarColor(pct);
    }
    if (hpText) hpText.textContent = `${hp}/${maxHp}`;
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
    if (e.key === 't') {
      e.preventDefault();
      chatInput.focus();
    }
    if (e.key === 'L') {
      e.preventDefault();
      void toggleMode();
    }
  });

  document.addEventListener('contextmenu', (e) => {
    if (gameScreen.style.display !== 'none') e.preventDefault();
  });

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
          lobbyRefreshTimer = setInterval(() => {
            void refreshServerList();
          }, 20000);
        }
      },
    );
    lobbyWatcher.addEventListener('close', () => {
      lobbyWatcher = null;
    });
  }

  function stopLobbyWatcher(): void {
    if (lobbyWatcher) {
      lobbyWatcher.close();
      lobbyWatcher = null;
    }
    if (lobbyRefreshTimer) {
      clearInterval(lobbyRefreshTimer);
      lobbyRefreshTimer = null;
    }
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
    avatarPreview.style.pointerEvents = disabled ? 'none' : '';
    serverList.querySelectorAll<HTMLButtonElement>('.join-btn').forEach((btn) => {
      btn.disabled = disabled;
    });
  }

  // ── Lobby ─────────────────────────────────────────────────────────
  function updateJoinButtons(): void {
    const selected = selectedAvatar;
    for (const btn of serverList.querySelectorAll<HTMLButtonElement>('.join-btn')) {
      const taken = (btn.dataset.avatars ?? '').split(',');
      const full = btn.dataset.full === 'true';
      const avatarTaken = taken.includes(selected);
      btn.disabled = full || avatarTaken;
      const isTeamBtn = btn.dataset.team !== undefined;
      let tipText: string | null = null;
      if (full) {
        tipText = isTeamBtn ? 'This team is full.' : 'This game is full.';
      } else if (avatarTaken) {
        tipText = 'Your avatar is already in use in this game. Pick a different one to join.';
      }
      if (tipText) {
        btn.onmouseenter = (e) => showTooltip(tipText, e.clientX, e.clientY);
        btn.onmousemove = (e) => moveTooltip(e.clientX, e.clientY);
        btn.onmouseleave = () => hideTooltip();
      } else {
        btn.onmouseenter = null;
        btn.onmousemove = null;
        btn.onmouseleave = null;
      }
    }
  }

  function appendAvatarCanvas(
    container: HTMLElement,
    avatarName: string,
    playerName: string,
  ): void {
    const c = document.createElement('canvas');
    c.width = 32;
    c.height = 32;
    void drawAvatarOnCanvas(c, avatarName);
    c.addEventListener('mouseenter', (e) =>
      showTooltip(`<div class="tip-name">${playerName}</div>`, e.clientX, e.clientY),
    );
    c.addEventListener('mousemove', (e) => moveTooltip(e.clientX, e.clientY));
    c.addEventListener('mouseleave', () => hideTooltip());
    container.appendChild(c);
  }

  function renderServerList(games: GameInfo[]): void {
    serverList.innerHTML = '';
    if (games.length === 0) {
      lobbyStatus.textContent = 'No active servers found.';
      return;
    }
    lobbyStatus.textContent = '';
    const header = document.createElement('div');
    header.className = 'server-header';
    header.innerHTML = `
      <span class="server-map">Map</span>
      <span class="server-rooms-hdr">Rooms</span>
      <span class="server-teams-hdr">Teams</span>
      <span class="server-count-hdr">Players</span>
      <span class="server-avatars-hdr">In Game</span>
      <span class="server-join-hdr"></span>
    `;
    serverList.appendChild(header);
    for (const game of games) {
      const row = document.createElement('div');
      row.className = 'server-row';
      const avatarKeys = (game.avatars ?? []).map((a) => a.avatar).join(',');
      const teamsVal = (game.teams ?? 0) === 0 ? 'FFA' : String(game.teams);

      const mapSpan = document.createElement('span');
      mapSpan.className = 'server-map';
      mapSpan.textContent = game.title ?? game.mapName;
      row.appendChild(mapSpan);

      if ((game.teams ?? 0) > 1) {
        // Multi-team: avatar lines | rooms | teams | per-team counts | join buttons
        const perTeamMax = Math.floor(game.maxPlayers / game.teams);
        const teamLines = document.createElement('div');
        teamLines.className = 'server-team-col server-team-lines';
        const countSpans: HTMLSpanElement[] = [];
        const joinBtns: HTMLButtonElement[] = [];
        for (let t = 1; t <= game.teams; t++) {
          const teamAvatars = (game.avatars ?? []).filter((a) => a.team === t);
          const teamCount = teamAvatars.length;
          const teamFull = teamCount >= perTeamMax;

          const line = document.createElement('div');
          line.className = 'server-team-line';

          const avatarStrip = document.createElement('span');
          avatarStrip.className = 'server-avatars';
          for (const entry of teamAvatars) {
            appendAvatarCanvas(avatarStrip, entry.avatar, entry.name);
          }

          line.appendChild(avatarStrip);
          teamLines.appendChild(line);

          const countSpan = document.createElement('span');
          countSpan.className = 'server-players';
          countSpan.textContent = `${teamCount}/${perTeamMax}`;
          countSpans.push(countSpan);

          const btn = document.createElement('button');
          btn.className = 'join-btn';
          btn.textContent = `Join Team ${t}`;
          btn.dataset.wsurl = game.wsUrl;
          btn.dataset.avatars = avatarKeys;
          btn.dataset.full = String(teamFull);
          btn.dataset.team = String(t);
          btn.addEventListener('click', () => joinServer(game, t));
          joinBtns.push(btn);
        }
        const roomsSpan = document.createElement('span');
        roomsSpan.className = 'server-rooms';
        roomsSpan.textContent = String(game.rooms ?? '?');
        const teamsSpan = document.createElement('span');
        teamsSpan.className = 'server-teams';
        teamsSpan.textContent = teamsVal;
        row.appendChild(roomsSpan);
        row.appendChild(teamsSpan);

        const countCol = document.createElement('div');
        countCol.className = 'server-team-col server-team-counts';
        for (const s of countSpans) countCol.appendChild(s);
        row.appendChild(countCol);

        row.appendChild(teamLines);

        const joinCol = document.createElement('div');
        joinCol.className = 'server-team-col server-join-btns';
        for (const btn of joinBtns) joinCol.appendChild(btn);
        row.appendChild(joinCol);
      } else {
        // Single-team: original column order (avatars, count, teams, rooms, join)
        const full = game.players >= game.maxPlayers;
        const avatarStrip = document.createElement('span');
        avatarStrip.className = 'server-avatars';
        for (const entry of game.avatars ?? []) {
          appendAvatarCanvas(avatarStrip, entry.avatar, entry.name);
        }
        const playersSpan = document.createElement('span');
        playersSpan.className = 'server-players';
        playersSpan.textContent = `${game.players}/${game.maxPlayers}`;

        const roomsSpan = document.createElement('span');
        roomsSpan.className = 'server-rooms';
        roomsSpan.textContent = String(game.rooms ?? '?');
        const teamsSpan = document.createElement('span');
        teamsSpan.className = 'server-teams';
        teamsSpan.textContent = teamsVal;

        const btn = document.createElement('button');
        btn.className = 'join-btn';
        btn.textContent = 'Join';
        btn.dataset.wsurl = game.wsUrl;
        btn.dataset.avatars = avatarKeys;
        btn.dataset.full = String(full);
        btn.addEventListener('click', () => joinServer(game));

        row.appendChild(roomsSpan);
        row.appendChild(teamsSpan);
        row.appendChild(playersSpan);
        row.appendChild(avatarStrip);
        row.appendChild(btn);
      }

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

  async function joinServer(gameInfo: GameInfo, team = 1): Promise<void> {
    if (isJoining) return;
    isJoining = true;
    setInputsDisabled(true);
    lobbyStatus.textContent = `Connecting to ${gameInfo.mapName}\u2026`;

    const playerName = playerNameInput.value.trim() || selectedAvatar;

    try {
      const { mapData, objFile } = await loadMap(gameInfo.mapName);

      // Set up inventory object references for the panel
      invObjects = objFile.objects;
      invObjset = mapData.map.objfilename.replace(/\.obj$/, '');
      mapLabel.textContent = gameInfo.title ?? gameInfo.mapName;

      const network = new GameNetwork(gameInfo.wsUrl);
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

      let localPlayerId = -1;

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
        void setPlayerDeadDisplay(msg.id, msg.dead);
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
        updateStats(msg.hp, msg.maxHp);
        game.setMyHp(msg.hp, msg.maxHp);
      };

      network.onPlayerHealth = (msg) => {
        if (msg.id === localPlayerId) {
          updateStats(msg.hp, msg.maxHp);
          game.setMyHp(msg.hp, msg.maxHp);
        }
        // Update HP bar in player list
        const entry = playerMap.get(msg.id);
        if (entry) {
          entry.hp = msg.hp;
          entry.maxHp = msg.maxHp;
          const pct = Math.max(0, (msg.hp / msg.maxHp) * 100);
          entry.hpFill.style.width = `${pct}%`;
          entry.hpFill.style.background = hpBarColor(pct);
        }
      };

      network.onReport = (msg) => {
        appendReport(msg.text);
      };

      network.onYouDied = (msg) => {
        appendReport(
          `You were slain by ${msg.killerName}. Respawning in ${Math.round(msg.deadForMs / 1000)} seconds…`,
        );
        cancelRespawn();
        respawnBtn.disabled = true;
        game.notifyDied();
        void setPlayerDeadDisplay(localPlayerId, true);
      };
      network.onYouRespawned = (msg) => {
        respawnBtn.disabled = false;
        void game.notifyRespawned(msg.room, msg.x, msg.y);
        void setPlayerDeadDisplay(localPlayerId, false);
      };

      network.onAccepted = (msg) => {
        localPlayerId = msg.id;
        showGame();
        startPlayerTick();
        status.textContent = `Connected as ${playerName} (id=${msg.id})`;
        game.setMyId(msg.id);
        // Server doesn't send PLAYER_INFO for the local player back to themselves
        void addPlayerRow(msg.id, playerName, selectedAvatar, 0, 0, Date.now());
        void game.goToRoom(msg.room, msg.x, msg.y);
      };

      await game.setAvatar(selectedAvatar);
      network.join(playerName, selectedAvatar, team);
    } catch (err) {
      lobbyStatus.textContent = `Error: ${err}`;
      setInputsDisabled(false);
      isJoining = false;
    }
  }

  async function toggleMode(): Promise<void> {
    currentMode = currentMode === 'dark' ? 'light' : 'dark';
    document.body.style.background = currentMode === 'dark' ? '#1a1a1a' : '#d0d0d0';
    document.body.style.color = currentMode === 'dark' ? '#ccc' : '#222';
    await currentGame?.setMode(currentMode);
  }

  // Returns a cancel function. When the countdown reaches zero, calls onConfirm().
  // beforeStart (optional) is called the moment the countdown begins.
  function makeCountdownButton(
    btn: HTMLButtonElement,
    idleLabel: string,
    activePrefix: string,
    onConfirm: () => void,
    beforeStart?: () => void,
  ): () => void {
    let timer: ReturnType<typeof setInterval> | null = null;
    function cancel(): void {
      if (timer !== null) {
        clearInterval(timer);
        timer = null;
      }
      btn.textContent = idleLabel;
    }
    btn.addEventListener('click', () => {
      if (timer !== null) {
        cancel();
        return;
      }
      beforeStart?.();
      let secs = 5;
      btn.textContent = `${activePrefix} ${secs}…`;
      timer = setInterval(() => {
        secs--;
        if (secs <= 0) {
          cancel();
          onConfirm();
          return;
        }
        btn.textContent = `${activePrefix} ${secs}…`;
      }, 1000);
    });
    return cancel;
  }

  let respawnWarned = false;

  // cancelLeave is a forward reference — valid because it's only called on click, after init.
  const cancelRespawn = makeCountdownButton(
    respawnBtn,
    'Respawn',
    'Respawn',
    () => currentNetwork?.sendVoluntaryRespawn(),
    () => {
      cancelLeave();
      if (!respawnWarned) {
        respawnWarned = true;
        appendReport('Warning: respawning will drop all of your carried items.');
      }
    },
  );

  function doLeave(): void {
    cancelRespawn();
    cancelLeave();
    respawnWarned = false;
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
  }

  const cancelLeave = makeCountdownButton(leaveBtn, 'Leave Game', 'Leaving', doLeave, () =>
    cancelRespawn(),
  );

  // Size the lobby logo canvas the same way the title screen sizes its top band
  const lobbyLogo = document.getElementById('lobby-logo') as HTMLCanvasElement;
  const logoH = Math.max(60, Math.floor(window.innerHeight / 4) - 30);
  lobbyLogo.width = 780;
  lobbyLogo.height = logoH;
  void drawLogo(lobbyLogo);

  // Half-size game screen logo
  const gameLogoCanvas = document.getElementById('game-logo') as HTMLCanvasElement;
  gameLogoCanvas.width = 400;
  gameLogoCanvas.height = 100;
  void drawLogo(gameLogoCanvas);

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
