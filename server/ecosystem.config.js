// PM2 process file for production deployment.
// Edit PUBLIC_WS_URL and LOBBY_URL to match your domain before deploying.
// Start with: pm2 start ecosystem.config.js

const DOMAIN = 'griljor.com';
const LOBBY  = 'http://localhost:3000';

module.exports = {
  apps: [
    {
      name:   'lobby',
      script: 'dist/lobby.js',
      env:    { PORT: 3000 },
    },
    {
      name:   'battle',
      script: 'dist/main.js',
      env:    { MAP: 'battle', PORT: 3001, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/battle` },
    },
    {
      name:   'castle',
      script: 'dist/main.js',
      env:    { MAP: 'castle', PORT: 3002, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/castle` },
    },
    {
      name:   'paradise',
      script: 'dist/main.js',
      env:    { MAP: 'paradise', PORT: 3003, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/paradise` },
    },
    {
      name:   'flag',
      script: 'dist/main.js',
      env:    { MAP: 'flag', PORT: 3004, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/flag` },
    },
    {
      name:   'ring',
      script: 'dist/main.js',
      env:    { MAP: 'ring', PORT: 3005, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/ring` },
    },
    {
      name:   'ivarr',
      script: 'dist/main.js',
      env:    { MAP: 'ivarr', PORT: 3006, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/ivarr` },
    },
    {
      name:   'playtesters',
      script: 'dist/main.js',
      env:    { MAP: 'playtesters', PORT: 3007, LOBBY_URL: LOBBY, PUBLIC_WS_URL: `wss://${DOMAIN}/ws/playtesters` },
    },
    // Add more maps here following the same pattern.
    // Each map needs a unique PORT and a matching /ws/<mapname> block in nginx-example.conf.
  ],
};
