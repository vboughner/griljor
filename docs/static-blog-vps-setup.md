# Static Blog on the Griljor VPS (Astro + nginx + GitHub Actions)

This guide sets up an Astro static blog on the same Hetzner VPS that runs Griljor, served under a separate domain, with automatic deploys on push to GitHub.

Astro is ideal for a blog that's mostly static content but where you want the freedom to drop in React components, interactive widgets, or fully custom pages whenever you want. It ships zero JavaScript by default and only hydrates the interactive pieces you explicitly opt into.

## Prerequisites

- The Griljor VPS is already running (nginx, certbot, PM2)
- You have a domain for the blog with DNS pointing to the VPS IP (A record)
- The blog lives in its own GitHub repository
- Node.js is already installed on the VPS (required by Griljor)

---

## 1. Create the blog directory on the VPS

SSH into the VPS as the `griljor` user:

```sh
sudo mkdir -p /var/www/blog
sudo chown griljor:griljor /var/www/blog
```

Using `/var/www/blog` instead of a home directory path keeps the blog cleanly separated from the game code and avoids the `chmod o+x` chain needed for home directory serving.

---

## 2. Add an nginx server block for the blog

Create `/etc/nginx/sites-available/blog`:

```nginx
server {
    server_name yourblog.com;

    root /var/www/blog;
    index index.html;

    # Serve static files, fall back to 404
    location / {
        try_files $uri $uri/ =404;
    }

    # Cache static assets
    location ~* \.(css|js|jpg|jpeg|png|gif|ico|svg|woff|woff2|ttf|eot)$ {
        expires 30d;
        add_header Cache-Control "public, immutable";
    }

    listen 80;
}
```

Enable it:

```sh
sudo ln -s /etc/nginx/sites-available/blog /etc/nginx/sites-enabled/blog
sudo nginx -t && sudo systemctl reload nginx
```

Then get HTTPS:

```sh
sudo certbot --nginx -d yourblog.com
```

Certbot will modify the server block to add SSL lines and an HTTP-to-HTTPS redirect, just like it did for `griljor.com`.

> **Replace `yourblog.com`** with your actual blog domain throughout this guide.

---

## 3. Set up SSH deploy key for GitHub Actions

On your **local machine** (not the VPS), generate a dedicated deploy key:

```sh
ssh-keygen -t ed25519 -C "github-actions-blog-deploy" -f ~/.ssh/blog_deploy_key -N ""
```

Then:

1. **Copy the public key to the VPS:**
   ```sh
   ssh-copy-id -i ~/.ssh/blog_deploy_key.pub griljor@<VPS_IP>
   ```

2. **Add the private key as a GitHub Actions secret** in your blog repository:
   - Go to your blog repo on GitHub → Settings → Secrets and variables → Actions
   - Click "New repository secret"
   - Name: `DEPLOY_SSH_KEY`
   - Value: paste the contents of `~/.ssh/blog_deploy_key`

3. **Add the VPS IP as another secret:**
   - Name: `DEPLOY_HOST`
   - Value: your VPS IP address

---

## 4. GitHub Actions workflow for auto-deploy

In your blog repository, create `.github/workflows/deploy.yml`:

```yaml
name: Deploy to VPS

on:
  push:
    branches: [main]

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: 20
          cache: npm

      - name: Install dependencies
        run: npm ci

      - name: Build
        run: npm run build

      - name: Deploy via rsync
        env:
          DEPLOY_SSH_KEY: ${{ secrets.DEPLOY_SSH_KEY }}
          DEPLOY_HOST: ${{ secrets.DEPLOY_HOST }}
        run: |
          mkdir -p ~/.ssh
          echo "$DEPLOY_SSH_KEY" > ~/.ssh/deploy_key
          chmod 600 ~/.ssh/deploy_key
          ssh-keyscan -H "$DEPLOY_HOST" >> ~/.ssh/known_hosts 2>/dev/null

          rsync -avz --delete \
            -e "ssh -i ~/.ssh/deploy_key" \
            dist/ \
            griljor@${DEPLOY_HOST}:/var/www/blog/
```

How it works:
- On every push to `main`, GitHub Actions checks out the repo, installs deps, runs `astro build`, and rsyncs the `dist/` output directory to `/var/www/blog/` on the VPS
- The `--delete` flag removes files from the VPS that no longer exist in the build output, keeping the deployment clean
- Build happens in CI (free 2000 min/month on GitHub Actions), so the VPS does no build work

---

## 5. Create an Astro blog (local setup)

On your local machine, create a new Astro site in a fresh repository:

```sh
npm create astro@latest myblog
cd myblog
```

The CLI wizard will ask you a few questions. Recommended choices:
- Template: **"blog"** (gives you a working blog with markdown support out of the box)
- TypeScript: **Yes** (strict)
- Install dependencies: **Yes**

### Add React support

This lets you use React components anywhere in your Astro pages and blog posts:

```sh
npx astro add react
```

This installs `@astrojs/react`, `react`, and `react-dom`, and updates your `astro.config.mjs` automatically.

### Configure the site URL

Edit `astro.config.mjs`:

```js
import { defineConfig } from 'astro/config';
import react from '@astrojs/react';
import mdx from '@astrojs/mdx';
import sitemap from '@astrojs/sitemap';

export default defineConfig({
  site: 'https://yourblog.com',
  integrations: [mdx(), sitemap(), react()],
});
```

### Project structure

The blog template gives you:

```
myblog/
├── public/              # Static assets (images, favicon, etc.)
├── src/
│   ├── components/      # Astro and React components
│   ├── content/
│   │   └── blog/        # Markdown/MDX blog posts
│   ├── layouts/         # Page layouts
│   ├── pages/           # File-based routing
│   │   ├── index.astro  # Home page
│   │   └── blog/        # Blog listing and post pages
│   └── styles/          # Global styles
├── astro.config.mjs
└── package.json
```

### Write a blog post

Create `src/content/blog/hello-world.md`:

```md
---
title: "Hello World"
description: "My first blog post"
pubDate: "Apr 04 2026"
---

This is my first blog post built with Astro.
```

### Use React components in posts (MDX)

Rename any post to `.mdx` to use components inside it:

`src/content/blog/interactive-post.mdx`:

```mdx
---
title: "A Post with Interactive Stuff"
description: "Mixing markdown with React components"
pubDate: "Apr 05 2026"
---

Here's some regular markdown text.

import Counter from '../../components/Counter.tsx';

And here's an interactive React counter:

<Counter client:load />

The `client:load` directive tells Astro to hydrate this component
in the browser. Everything else on this page ships as zero JS.
```

`src/components/Counter.tsx`:

```tsx
import { useState } from 'react';

export default function Counter() {
  const [count, setCount] = useState(0);
  return (
    <button onClick={() => setCount(count + 1)}>
      Clicks: {count}
    </button>
  );
}
```

### Astro's hydration directives

These control when (and whether) a component's JavaScript loads in the browser:

| Directive | Behavior |
|-----------|----------|
| `client:load` | Hydrates immediately on page load |
| `client:idle` | Hydrates once the browser is idle (good for below-the-fold) |
| `client:visible` | Hydrates when the component scrolls into view |
| `client:only="react"` | Renders only on the client (skip server render) |
| *(no directive)* | Renders to static HTML, ships zero JS |

This is what makes Astro great for blogs: most of the page is static HTML, and you only pay the JS cost for the pieces that actually need interactivity.

### Preview locally

```sh
npm run dev
# Open http://localhost:4321
```

### Push to deploy

```sh
git init
git add -A
git commit -m "Initial blog setup"
git remote add origin https://github.com/yourusername/myblog.git
git push -u origin main
```

The GitHub Actions workflow triggers automatically and deploys to your VPS.

---

## 6. Create fully custom pages

Astro uses file-based routing. Any `.astro` or `.tsx` file in `src/pages/` becomes a route:

- `src/pages/about.astro` → `yourblog.com/about`
- `src/pages/projects.astro` → `yourblog.com/projects`

An Astro page can be entirely custom — no blog layout required:

```astro
---
// src/pages/playground.astro
import Layout from '../layouts/Base.astro';
import MyReactApp from '../components/MyReactApp.tsx';
---
<Layout title="Playground">
  <MyReactApp client:load />
</Layout>
```

This lets you have a standard blog alongside completely bespoke interactive pages, all in one site.

---

## Summary of what lives where

```
VPS (Hetzner CX22)
├── nginx (port 80/443)
│   ├── griljor.com → /home/griljor/griljor/client/dist/ + proxy to Node.js
│   └── yourblog.com → /var/www/blog/ (pure static)
├── PM2 → lobby + game servers (unchanged)
└── /var/www/blog/ ← rsync target from GitHub Actions

GitHub
├── vboughner/griljor (game repo, unchanged)
└── yourusername/myblog (blog repo, Astro)
    └── .github/workflows/deploy.yml → builds Astro + rsyncs to VPS
```

---

## Maintenance notes

- **Adding HTTPS for the blog does not affect griljor.com** — certbot manages each domain's certificate independently
- **Node.js version**: pin the same version in the GitHub Actions workflow and on the VPS to avoid build drift
- **DNS**: add an A record for `yourblog.com` pointing to the same VPS IP, using Cloudflare DNS-only (grey cloud) or your registrar's DNS
- **If you later want `www.yourblog.com` too**: add it to the nginx `server_name` line (`server_name yourblog.com www.yourblog.com;`) and re-run `sudo certbot --nginx -d yourblog.com -d www.yourblog.com`
- **Updating Astro**: run `npx @astrojs/upgrade` in your blog repo to update Astro and its integrations together
