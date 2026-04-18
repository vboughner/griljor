# Static Blog on the Griljor VPS (astro-modular + nginx + GitHub Actions)

This guide sets up an [astro-modular](https://github.com/davidvkimball/astro-modular) blog on the same Hetzner VPS that runs Griljor, served under a separate domain, with automatic deploys on push to GitHub.

astro-modular is a feature-rich Astro blog theme with Obsidian vault integration, 16+ built-in color themes, a command palette with search, content graph visualization, MDX support, and more — all shipping minimal JavaScript by default.

## Prerequisites

- The Griljor VPS is already running (nginx, certbot, PM2)
- You have a domain for the blog with DNS pointing to the VPS IP (A record)
- The blog lives in its own GitHub repository (`vboughner/ai-blog`)
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
          node-version: 24
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
- On every push to `main`, GitHub Actions checks out the repo, installs deps, builds the site, and rsyncs the `dist/` output directory to `/var/www/blog/` on the VPS
- The `--delete` flag removes files from the VPS that no longer exist in the build output, keeping the deployment clean
- Build happens in CI (free 2000 min/month on GitHub Actions), so the VPS does no build work
- astro-modular's `npm run build` script automatically runs preprocessing steps (image sync, alias processing, graph data generation) before the Astro build

---

## 5. Set up astro-modular (local setup)

On your local machine, clone or scaffold the astro-modular template into a fresh repository:

```sh
npx create-astro-modular ai-blog
cd ai-blog
npm install
```

Alternatively, you can clone the template directly:

```sh
git clone https://github.com/davidvkimball/astro-modular.git ai-blog
cd ai-blog
rm -rf .git
npm install
```

> **Note:** astro-modular's upstream repo uses pnpm, but it works fine with npm. If you see a `pnpm-lock.yaml` file after cloning, delete it — npm will generate its own `package-lock.json` on install.

### Configure the site

Edit `src/config.ts` — this is the central configuration file:

```ts
export const siteConfig: SiteConfig = {
  title: 'My Blog',
  description: 'A blog about things',
  author: 'Your Name',
  language: 'en',
  site: 'https://yourblog.com',
  // ... theme, fonts, features, etc.
};
```

Key settings in `src/config.ts`:
- **`site`** — your blog's URL (used for RSS, sitemap, Open Graph)
- **`theme`** — choose from 16+ built-in themes (minimal, nord, dracula, catppuccin, gruvbox, rose-pine, solarized, etc.)
- **`fonts`** — configure body, heading, and monospace font families
- **`tableOfContents`** — enable/disable and set depth
- **`commandPalette`** — search scope and keyboard shortcut
- **`deployment`** — leave unset or ignore; we deploy via rsync, not a platform adapter

### Project structure

```
ai-blog/
├── public/              # Static assets (images, favicon, etc.)
├── src/
│   ├── config.ts        # Central site configuration (title, theme, fonts, features)
│   ├── components/      # Astro components (Header, Footer, PostCard, CommandPalette, etc.)
│   ├── content/
│   │   ├── posts/       # Blog posts (Markdown/MDX)
│   │   ├── pages/       # Static pages
│   │   ├── projects/    # Project showcases
│   │   └── docs/        # Documentation pages
│   ├── layouts/         # Page layouts
│   ├── pages/           # File-based routing (index, posts/, projects/, etc.)
│   ├── themes/          # Built-in color themes
│   ├── styles/          # CSS / Tailwind styles
│   └── utils/           # Remark/rehype plugins for Obsidian compatibility
├── scripts/             # Build-time preprocessing (image sync, graph data, etc.)
├── astro.config.mjs
└── package.json
```

### Write a blog post

Create `src/content/posts/hello-world.md`:

```md
---
title: "Hello World"
description: "My first blog post"
pubDate: 2026-04-04
---

This is my first blog post built with astro-modular.
```

### Obsidian integration

astro-modular is designed to work as an Obsidian vault. The `src/content/` directory *is* the vault — you can open it directly in Obsidian and write posts there. Features that work out of the box:

- `[[Wikilinks]]` between posts
- `![[Embeds]]` for including content from other notes
- Callouts (`> [!note]`, `> [!warning]`, etc.)
- `%%Comments%%` (stripped from published output)
- Image sizing syntax
- Mermaid diagrams and LaTeX math (KaTeX)

### Use MDX for interactive components

Rename any post to `.mdx` to embed interactive components:

`src/content/posts/interactive-post.mdx`:

```mdx
---
title: "A Post with Interactive Stuff"
description: "Mixing markdown with components"
pubDate: 2026-04-05
---

Here's some regular markdown text.

import Counter from '../../components/Counter.tsx';

And here's an interactive counter:

<Counter client:load />

The `client:load` directive tells Astro to hydrate this component
in the browser. Everything else on this page ships as zero JS.
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

### Preview locally

```sh
npm run dev
# Open http://localhost:5000
```

### Push to deploy

```sh
git init
git add -A
git commit -m "Initial blog setup"
git remote add origin https://github.com/vboughner/ai-blog.git
git push -u origin main
```

The GitHub Actions workflow triggers automatically and deploys to your VPS.

---

## 6. Create fully custom pages

Astro uses file-based routing. Any `.astro` file in `src/pages/` becomes a route:

- `src/pages/about.astro` → `yourblog.com/about`
- `src/pages/projects.astro` → `yourblog.com/projects`

astro-modular includes several page types out of the box (posts, projects, docs), but you can add entirely custom pages too:

```astro
---
// src/pages/playground.astro
import Layout from '../layouts/Base.astro';
import MyWidget from '../components/MyWidget.tsx';
---
<Layout title="Playground">
  <MyWidget client:load />
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
└── vboughner/ai-blog (blog repo, astro-modular)
    └── .github/workflows/deploy.yml → builds site + rsyncs to VPS
```

---

## Maintenance notes

- **Adding HTTPS for the blog does not affect griljor.com** — certbot manages each domain's certificate independently
- **Node.js version**: astro-modular requires Node.js 24+. Pin the same version in the GitHub Actions workflow and on the VPS to avoid build drift
- **DNS**: add an A record for `yourblog.com` pointing to the same VPS IP, using Cloudflare DNS-only (grey cloud) or your registrar's DNS
- **If you later want `www.yourblog.com` too**: add it to the nginx `server_name` line (`server_name yourblog.com www.yourblog.com;`) and re-run `sudo certbot --nginx -d yourblog.com -d www.yourblog.com`
- **Updating astro-modular**: run `npm run update` to pull the latest theme framework files while preserving your content and configuration
