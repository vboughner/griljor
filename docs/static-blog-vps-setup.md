# Static Blog on the Griljor VPS (Hugo + nginx + GitHub Actions)

This guide sets up a Hugo static blog on the same Hetzner VPS that runs Griljor, served under a separate domain, with automatic deploys on push to GitHub.

## Prerequisites

- The Griljor VPS is already running (nginx, certbot, PM2)
- You have a domain for the blog with DNS pointing to the VPS IP (A record)
- The blog lives in its own GitHub repository

---

## 1. Install Hugo on the VPS

SSH into the VPS as the `griljor` user:

```sh
# Download the latest Hugo extended edition (check https://github.com/gohugoio/hugo/releases for current version)
HUGO_VERSION="0.147.0"
wget -O /tmp/hugo.deb "https://github.com/gohugoio/hugo/releases/download/v${HUGO_VERSION}/hugo_extended_${HUGO_VERSION}_linux-amd64.deb"
sudo dpkg -i /tmp/hugo.deb
rm /tmp/hugo.deb
hugo version
```

> **Note:** Hugo is only needed on the VPS if you want to build there. The GitHub Actions approach below builds in CI and only copies the output, so this step is optional. It's still handy for debugging.

---

## 2. Create the blog directory on the VPS

```sh
sudo mkdir -p /var/www/blog
sudo chown griljor:griljor /var/www/blog
```

Using `/var/www/blog` instead of a home directory path keeps the blog cleanly separated from the game code and avoids the `chmod o+x` chain needed for home directory serving.

---

## 3. Add an nginx server block for the blog

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

## 4. Set up SSH deploy key for GitHub Actions

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

## 5. GitHub Actions workflow for auto-deploy

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
        with:
          submodules: true  # Hugo themes are often git submodules

      - name: Setup Hugo
        uses: peaceiris/actions-hugo@v3
        with:
          hugo-version: "0.147.0"
          extended: true

      - name: Build
        run: hugo --minify

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
            public/ \
            griljor@${DEPLOY_HOST}:/var/www/blog/
```

How it works:
- On every push to `main`, GitHub Actions checks out the repo, builds with Hugo, and rsyncs the `public/` output directory to `/var/www/blog/` on the VPS
- The `--delete` flag removes files from the VPS that no longer exist in the build output, keeping the deployment clean
- Build happens in CI (free 2000 min/month on GitHub Actions), so the VPS does no build work

---

## 6. Create a Hugo blog (local setup)

On your local machine, create a new Hugo site in a fresh repository:

```sh
hugo new site myblog
cd myblog
git init
```

Pick a theme. For example, [PaperMod](https://github.com/adityatelange/hugo-PaperMod) is a popular, clean blog theme:

```sh
git submodule add https://github.com/adityatelange/hugo-PaperMod.git themes/PaperMod
```

Edit `hugo.toml`:

```toml
baseURL = "https://yourblog.com/"
languageCode = "en-us"
title = "My Blog"
theme = "PaperMod"

[params]
  defaultTheme = "auto"
  showReadingTime = true
  showShareButtons = false
  showPostNavLinks = true

[outputs]
  home = ["HTML", "RSS", "JSON"]
```

Create your first post:

```sh
hugo new posts/hello-world.md
```

Edit `content/posts/hello-world.md` — set `draft: false` when ready to publish.

Preview locally:

```sh
hugo server -D
# Open http://localhost:1313
```

When it looks good:

```sh
git add -A
git commit -m "Initial blog setup"
git remote add origin https://github.com/yourusername/myblog.git
git push -u origin main
```

The GitHub Actions workflow triggers automatically and deploys to your VPS.

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
└── yourusername/myblog (blog repo)
    └── .github/workflows/deploy.yml → builds Hugo + rsyncs to VPS
```

---

## Maintenance notes

- **Adding HTTPS for the blog does not affect griljor.com** — certbot manages each domain's certificate independently
- **Hugo version**: pin the same version in `hugo.toml` (`hugo.version`) and in the GitHub Actions workflow to avoid build drift
- **Theme updates**: `cd themes/PaperMod && git pull origin master`, then commit the submodule change
- **DNS**: add an A record for `yourblog.com` pointing to the same VPS IP, using Cloudflare DNS-only (grey cloud) or your registrar's DNS
- **If you later want `www.yourblog.com` too**: add it to the nginx `server_name` line (`server_name yourblog.com www.yourblog.com;`) and re-run `sudo certbot --nginx -d yourblog.com -d www.yourblog.com`
