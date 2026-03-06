// PM2 process file for production deployment.
// Edit PUBLIC_WS_URL and LOBBY_URL to match your domain before deploying.
// Start with: pm2 start ecosystem.config.js

const DOMAIN = 'griljor.example.com'; // TODO: replace with your domain
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
    // Add more maps here following the same pattern.
    // Each map needs a unique PORT and a matching /ws/<mapname> block in nginx-example.conf.
  ],
};
