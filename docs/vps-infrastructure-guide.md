# VPS Infrastructure Guide

How to recreate the Hetzner VPS from scratch, including all domains, the game, the blog, and domain redirect. This is the "what we actually built" companion to [`deployment-plan.md`](deployment-plan.md) (game setup) and [`static-blog-vps-setup.md`](static-blog-vps-setup.md) (blog setup).

## Overview

A single Hetzner Cloud CX22 (2 vCPU, 4 GB RAM, Ubuntu 24.04) hosts everything:

```
VPS (5.78.75.71)
├── nginx (ports 80/443)
│   ├── griljor.com        → game frontend + WebSocket proxy to Node.js
│   ├── vanboughner.com    → static blog (/var/www/blog/)
│   └── hovercloud.com     → 301 redirect to vanboughner.com
├── PM2
│   ├── lobby (port 3000)
│   └── game servers (ports 3001–3007, one per map)
├── /home/griljor/griljor/       → game repo clone
└── /var/www/blog/               → blog build output (deployed via rsync from GitHub Actions)
```

---

## Domains

Four domains are managed. All use Cloudflare for DNS. Three are registered at Cloudflare Registrar (transferred from GoDaddy). One remains at GreenGeeks.

| Domain | Registrar | DNS | Points to | Purpose |
|--------|-----------|-----|-----------|---------|
| `griljor.com` | GreenGeeks | Cloudflare | VPS | Multiplayer game |
| `vanboughner.com` | Cloudflare | Cloudflare | VPS | Personal blog (Astro) |
| `hovercloud.com` | Cloudflare | Cloudflare | VPS | 301 redirect → vanboughner.com |
| `fellupon.com` | Cloudflare | Cloudflare | Parked (www → artistwebsites.com) | Email forwarding only (for now) |

### Cloudflare DNS settings

All A records use **DNS only** (grey cloud, not orange proxied). This is critical for griljor.com because Cloudflare's orange proxy breaks WebSocket connections. The other domains use grey cloud for consistency.

**griljor.com:**
| Type | Name | Value |
|------|------|-------|
| A | `@` | `5.78.75.71` |

**vanboughner.com:**
| Type | Name | Value |
|------|------|-------|
| A | `@` | `5.78.75.71` |
| CNAME | `www` | `vanboughner.com` |
| MX | `@` | `10 mx1.forwardemail.net.` |
| MX | `@` | `10 mx2.forwardemail.net.` |
| TXT | `@` | `v=spf1 a mx include:spf.forwardemail.net -all` |
| TXT | `@` | `forward-email=van.boughner@gmail.com` |

**hovercloud.com:**
| Type | Name | Value |
|------|------|-------|
| A | `@` | `5.78.75.71` |
| CNAME | `www` | `hovercloud.com` |
| MX | `@` | `10 mx1.forwardemail.net.` |
| MX | `@` | `10 mx2.forwardemail.net.` |
| TXT | `@` | `v=spf1 a mx include:spf.forwardemail.net -all` |
| TXT | `@` | `forward-email=van:van.boughner@gmail.com` |
| TXT | `@` | `forward-email=van13:van.boughner@gmail.com` |
| TXT | `@` | `forward-email=adult:van.boughner@gmail.com` |
| TXT | `@` | `forward-email=duckvandoge:van.boughner@gmail.com` |
| TXT | `@` | `forward-email=offers:van@hovercloud.com` |
| TXT | `@` | `forward-email=phillipandvan:van.boughner@gmail.com,phillipandvan:fellupon@gmail.com` |
| TXT | `@` | `forward-email=vanandphillip:van.boughner@gmail.com,vanandphillip:fellupon@gmail.com` |
| TXT | `@` | `forward-email=plants:van@hovercloud.com` |
| TXT | `@` | `forward-email=zombies:plants@hovercloud.com` |
| TXT | `@` | `forward-email=ground:zombies@hovercloud.com` |

**fellupon.com:**
| Type | Name | Value |
|------|------|-------|
| A | `@` | `216.185.148.181` (parked — currently unused) |
| CNAME | `www` | `custom.artistwebsites.com` |
| MX | `@` | `10 mx1.forwardemail.net.` |
| MX | `@` | `10 mx2.forwardemail.net.` |
| TXT | `@` | `v=spf1 a mx include:spf.forwardemail.net -all` |
| TXT | `@` | `forward-email=van:van.boughner@gmail.com` |
| TXT | `@` | `forward-email=phillip:fellupon@gmail.com` |
| TXT | `@` | `forward-email=phillipgomez:fellupon@gmail.com` |
| TXT | `@` | `forward-email=griljoy:fellupon@gmail.com` |
| TXT | `@` | `forward-email=phillipandvan:fellupon@gmail.com,phillipandvan:van.boughner@gmail.com` |
| TXT | `@` | `forward-email=vanandphillip:fellupon@gmail.com,vanandphillip:van.boughner@gmail.com` |

### Email forwarding

All three personal domains use [ForwardEmail.net](https://forwardemail.net) (free tier). Email forwarding is configured entirely via DNS records (MX + TXT). No server-side email software runs on the VPS.

- `anything@vanboughner.com` → `van.boughner@gmail.com`
- `van@hovercloud.com` → `van.boughner@gmail.com` (plus many other aliases — see TXT records above)
- `van@fellupon.com` → `van.boughner@gmail.com` (plus other aliases — see TXT records above)

---

## Domain registration migration notes

### Why Cloudflare Registrar

Cloudflare sells domain registrations at cost (~$10.44/year for `.com`), includes WHOIS privacy for free, and consolidates DNS + registration in one dashboard. GoDaddy charges $18–22/year with aggressive upselling.

### How the transfers were done

Three domains were transferred from GoDaddy to Cloudflare Registrar: `vanboughner.com`, `hovercloud.com`, `fellupon.com`. The process for each:

1. **At GoDaddy:** turn off Domain Lock, turn off WHOIS privacy, request the EPP/authorization code
2. **At Cloudflare:** Domain Registration → Transfer Domains → enter domain + EPP code → pay one year (~$10.44)
3. **Approve the transfer email** from GoDaddy (check spam folder)
4. Wait 1–3 days (up to 7 if the email isn't approved)

`griljor.com` remains registered at GreenGeeks with nameservers pointing to Cloudflare.

### DNS migration (before registration transfer)

DNS was migrated to Cloudflare independently of and prior to the registration transfer. The process for each domain:

1. **Backup existing DNS** by querying the current nameservers:
   ```sh
   for type in A AAAA CNAME MX TXT NS CAA; do
     echo "=== $type ==="
     dig @1.1.1.1 example.com $type +short
   done
   dig @1.1.1.1 www.example.com +short
   ```
2. **Add the site in Cloudflare** (free plan). Cloudflare auto-scans existing DNS records.
3. **Verify the scan caught everything** — compare against the backup, especially MX and TXT records for email. Cloudflare's scan can miss TXT records when there are many.
4. **Update A records** to point at the VPS IP (`5.78.75.71`), grey cloud (DNS only).
5. **Change nameservers** at the registrar from the old ones (e.g. `dns[1-4].p05.nsone.net` for Netlify, `ns1/ns2.mediatemple.net` for GoDaddy) to the Cloudflare-assigned ones.
6. **Verify propagation:** `dig @1.1.1.1 example.com NS +short` should show Cloudflare nameservers.
7. **Test email forwarding** — send a test email to confirm MX/TXT records are intact.

**GoDaddy-specific cleanup:** When importing DNS from GoDaddy, Cloudflare's scan may find GoDaddy artifacts that can be deleted:
- `autoconfig` CNAME → `autodiscover.accessdomain.com` (GoDaddy email auto-config)
- `_domainconnect` CNAME → `_domainconnect.gd.domaincontrol.com` (GoDaddy Domain Connect protocol)
- SRV records → `autodiscover.accessdomain.com` (Microsoft Autodiscover for GoDaddy email)

**Netlify-specific notes (vanboughner.com):** Netlify uses NS1 nameservers (`dns[1-4].p05.nsone.net`). After switching nameservers to Cloudflare, NS1 still responds with old records until the NS cache expires. If `dig` on your local machine shows old IPs while `dig @1.1.1.1` shows the new IP, your local resolver has cached stale NS records. Fix: flush macOS DNS cache (`sudo dscacheutil -flushcache && sudo killall -HUP mDNSResponder`) or set your Mac's DNS server to `1.1.1.1`.

**GoDaddy forwarding (hovercloud.com):** GoDaddy's "domain forwarding" feature uses AWS Global Accelerator IPs and is unreliable (returned `405 Not Allowed`). It was replaced with an nginx 301 redirect on the VPS. Before changing DNS, remove the forwarding configuration at GoDaddy first — otherwise GoDaddy may override your DNS records with its forwarding IPs.

---

## Recreating the VPS from scratch

Follow these steps in order. Steps 1–3 are from [`deployment-plan.md`](deployment-plan.md), repeated here for completeness.

### 1. Provision the VM

1. Hetzner Cloud → Add Server → Ubuntu 24.04 → CX22 → add SSH key → create
2. Note the public IP address
3. SSH in: `ssh root@<IP>`

### 2. Create the non-root user

```sh
adduser griljor
usermod -aG sudo griljor
rsync --archive --chown=griljor:griljor ~/.ssh /home/griljor
```

All subsequent steps use the `griljor` user.

### 3. Install dependencies

```sh
curl -fsSL https://deb.nodesource.com/setup_22.x | sudo -E bash -
sudo apt-get install -y nodejs nginx certbot python3-certbot-nginx git rsync
sudo npm install -g pm2
```

### 3b. Lock down the firewall

Only 22, 80 and 443 should ever be reachable from the internet. The Node
processes bind `127.0.0.1` (see `server/src/main.ts`), so nginx is the only
public entry point — this is the backstop for when something forgets to.

Preferred: a **Hetzner Cloud Firewall** (Cloud Console → Firewalls → Apply to
Server). It filters before traffic reaches the VM, so it still holds if a
process on the box binds the wrong interface.

| Direction | Protocol | Port | Source |
|---|---|---|---|
| Inbound | TCP | 22 | your IP, or `0.0.0.0/0` |
| Inbound | TCP | 80 | `0.0.0.0/0` |
| Inbound | TCP | 443 | `0.0.0.0/0` |

Everything else inbound: drop. Leave outbound unrestricted (certbot, apt, git).

Belt and braces, on the VM itself:

```sh
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow 22/tcp
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw enable
sudo ufw status verbose
```

Do not open 3000-3007. Those are the lobby and per-map game servers, and the
browser never contacts them directly — the client only ever talks to
`https://griljor.com`, and nginx proxies `/games`, `/reset`, `/watch` and
`/ws/<mapname>` to loopback.

### 4. Deploy the game (griljor.com)

Follow [`deployment-plan.md`](deployment-plan.md) sections "Clone and build", "Configure nginx", "HTTPS via Let's Encrypt", and "Start everything with PM2".

Summary:
```sh
git clone -b modern-rewrite https://github.com/vboughner/griljor.git ~/griljor
cd ~/griljor

cd server && npm install && npm run build && cd ..

cd client
echo "VITE_LOBBY_URL=https://griljor.com" > .env.production
npm install && npm run build && cd ..

sudo cp ~/griljor/server/nginx-example.conf /etc/nginx/sites-available/griljor
sudo sed -i 's|/home/ubuntu/|/home/griljor/|g' /etc/nginx/sites-available/griljor
sudo ln -s /etc/nginx/sites-available/griljor /etc/nginx/sites-enabled/griljor
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t && sudo systemctl reload nginx

chmod o+x /home/griljor
chmod o+x /home/griljor/griljor
chmod o+x /home/griljor/griljor/client
chmod -R o+r /home/griljor/griljor/client/dist

sudo certbot --nginx -d griljor.com

cd ~/griljor/server
pm2 start ecosystem.config.js
pm2 save
pm2 startup  # copy/paste the printed command
```

### 5. Deploy the blog (vanboughner.com)

Follow [`static-blog-vps-setup.md`](static-blog-vps-setup.md) for full details. Summary:

```sh
sudo mkdir -p /var/www/blog
sudo chown griljor:griljor /var/www/blog
```

Create `/etc/nginx/sites-available/blog`:

```nginx
server {
    server_name vanboughner.com www.vanboughner.com;

    root /var/www/blog;
    index index.html;

    location / {
        try_files $uri $uri/ =404;
    }

    location ~* \.(css|js|jpg|jpeg|png|gif|ico|svg|woff|woff2|ttf|eot)$ {
        expires 30d;
        add_header Cache-Control "public, immutable";
    }

    listen 80;
}
```

Enable, test, and get HTTPS:

```sh
sudo ln -s /etc/nginx/sites-available/blog /etc/nginx/sites-enabled/blog
sudo nginx -t && sudo systemctl reload nginx
sudo certbot --nginx -d vanboughner.com -d www.vanboughner.com
```

After certbot runs, verify that both `server_name` lines in the modified config include `www.vanboughner.com`. If certbot missed the `www`, add it manually to both server blocks (the HTTPS block and the HTTP redirect block).

The blog content is deployed automatically by GitHub Actions in the `vboughner/ai-blog` repo. On push to `main`, the workflow builds the Astro site and rsyncs `dist/` to `/var/www/blog/`. The deploy key must be authorized on the VPS — see [`static-blog-vps-setup.md`](static-blog-vps-setup.md) section 3.

### 6. Set up the redirect (hovercloud.com)

Create `/etc/nginx/sites-available/hovercloud-redirect`:

```nginx
server {
    server_name hovercloud.com www.hovercloud.com;
    return 301 https://vanboughner.com$request_uri;
    listen 80;
}
```

Enable, test, and get HTTPS:

```sh
sudo ln -s /etc/nginx/sites-available/hovercloud-redirect /etc/nginx/sites-enabled/
sudo nginx -t && sudo systemctl reload nginx
sudo certbot --nginx -d hovercloud.com -d www.hovercloud.com
```

After certbot, verify both `server_name` lines include `www.hovercloud.com`. The HTTPS server block should still have the `return 301` directive — certbot preserves it.

### 7. Update DNS

For each domain served by the VPS (griljor.com, vanboughner.com, hovercloud.com):

1. In Cloudflare DNS, set the A record for `@` to the new VPS IP
2. Set `www` as a CNAME to the apex domain (or a second A record)
3. Use DNS only (grey cloud)
4. Wait for propagation: `dig @1.1.1.1 example.com +short`

### 8. Verify everything

```sh
# Game
pm2 status
curl https://griljor.com/games

# Game/lobby ports must NOT be reachable from outside — every one should fail
for p in 3000 3001 3002 3003 3004 3005 3006 3007; do
  printf "%s: " "$p"; curl -s -o /dev/null -m 5 -w "%{http_code}\n" "http://<IP>:$p/"
done
# Expected: 000 for all eight (connection refused / filtered)

# And loopback should still work, from on the VM
sudo ss -lntp | grep -E ':300[0-7]'
# Expected: every line shows 127.0.0.1:300x, never 0.0.0.0:300x or *:300x

# Blog
curl -I https://vanboughner.com
curl -I https://www.vanboughner.com

# Redirect
curl -I https://hovercloud.com
# Should return: 301 → https://vanboughner.com/

# Certs
sudo certbot certificates
# Should list griljor.com, vanboughner.com, hovercloud.com

# Email (send test emails to verify ForwardEmail forwarding)
```

---

## nginx server blocks on the VPS

Three server blocks exist in `/etc/nginx/sites-available/`, each symlinked to `sites-enabled/`:

| File | Domain | Purpose |
|------|--------|---------|
| `griljor` | `griljor.com` | Game frontend + WebSocket proxy (see [`deployment-plan.md`](deployment-plan.md)) |
| `blog` | `vanboughner.com`, `www.vanboughner.com` | Static blog from `/var/www/blog/` |
| `hovercloud-redirect` | `hovercloud.com`, `www.hovercloud.com` | 301 redirect to `https://vanboughner.com` |

All three have been modified by certbot to include SSL configuration. Never overwrite a certbot-modified config by copying from the repo — use `sed` or manual edits instead.

---

## Let's Encrypt certificates

Three certificates are managed by certbot, each renewing automatically via systemd timer:

| Certificate | Domains covered |
|-------------|----------------|
| `griljor.com` | `griljor.com` |
| `vanboughner.com` | `vanboughner.com`, `www.vanboughner.com` |
| `hovercloud.com` | `hovercloud.com`, `www.hovercloud.com` |

Check status: `sudo certbot certificates`

Renewal happens automatically. To test: `sudo certbot renew --dry-run`

---

## Blog auto-deploy pipeline

The blog (`vboughner/ai-blog` on GitHub) deploys automatically on push to `main`:

1. GitHub Actions checks out the repo
2. Installs pnpm + Node.js 22
3. Runs `pnpm install --frozen-lockfile`
4. Runs `pnpm run build` (astro-modular preprocessing + Astro build → `dist/`)
5. Rsyncs `dist/` to `griljor@<VPS_IP>:/var/www/blog/` via SSH

**GitHub Actions secrets required in `vboughner/ai-blog`:**

| Secret | Value |
|--------|-------|
| `DEPLOY_SSH_KEY` | Private key of the deploy keypair |
| `DEPLOY_HOST` | VPS IP address |
| `DEPLOY_USER` | `griljor` |

**VPS requirement:** The deploy key's public half must be in `/home/griljor/.ssh/authorized_keys`.

To redeploy manually (without pushing to GitHub), SSH into the VPS and:

```sh
cd /tmp
git clone https://github.com/vboughner/ai-blog.git
cd ai-blog
npm install
npm run build
rsync -avz --delete dist/ /var/www/blog/
rm -rf /tmp/ai-blog
```

---

## Updating the game

```sh
ssh griljor@<VPS_IP>
cd ~/griljor && git pull
bash ~/griljor/scripts/rebuild-restart-production.sh
```

See [`deployment-plan.md`](deployment-plan.md) for map management, troubleshooting, and other operational details.

---

## Key lessons learned

- **certbot and www**: certbot's nginx installer sometimes fails to find a matching server block for `www.` if the `server_name` directive only lists the apex domain. Always include both names in `server_name` before running certbot, or add `www` manually after certbot modifies the config.
- **Cloudflare proxy (orange cloud) breaks WebSockets**: always use DNS only (grey cloud) for griljor.com. The other domains could use orange cloud but grey is simpler and consistent.
- **NS record caching**: after switching nameservers at a registrar, your local machine may cache the old NS records for hours. Public resolvers like `1.1.1.1` and `8.8.8.8` update faster. Use `dig @1.1.1.1` to verify, or set your Mac's DNS to `1.1.1.1`.
- **Service workers cause stale content**: if the old site (e.g. a Netlify/Gatsby blog) registered a service worker, browsers will keep serving the old content from cache even after DNS points elsewhere. Fix: Chrome DevTools → Application → Clear site data.
- **GoDaddy forwarding is unreliable**: their forwarding service (AWS Global Accelerator-backed) returned `405 Not Allowed`. Replaced with a simple nginx `return 301` on the VPS — faster, more reliable, proper HTTPS.
- **GoDaddy DNS artifacts to delete when migrating**: `_domainconnect` CNAME, `autoconfig` CNAME, and `autodiscover` SRV records are GoDaddy-specific and useless on Cloudflare.
- **Back up DNS before migrating**: query all record types with `dig` before changing nameservers. Cloudflare's scan catches most records but can miss TXT records when there are many (especially ForwardEmail aliases).
- **`netlify.toml` in astro-modular**: the blog template generates Netlify-specific config during build. This is harmless on the VPS (nginx ignores it). Add `netlify.toml` to `.gitignore` to keep `git status` clean.
