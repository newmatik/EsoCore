# EdgeSentinel Website & Docs

This repository contains the static website and documentation viewer for EdgeSentinel.

## Local development

From the project root, start a static server:

```sh
npx serve -l 3000
# or any static server (python3 -m http.server 3000)
```

Open `http://localhost:3000/` (docs at `/docs`).

## Temporary access protection

The site is temporarily protected with a simple client-side access code prompt. Use this code to unlock a session:

- Access code: `55619`

The unlock persists for the browser tab (sessionStorage) and is intended only to keep previews private while we iterate.

## Deployment (GitHub Pages)

Deployed from the `main` branch with a custom domain.

1. Push to `main`.
2. Settings → Pages → Deploy from branch (`main`, root).
3. Set custom domain to `www.edgesentinel.de` (CNAME provided). Ensure DNS points to GitHub Pages.

Deep links to `/docs/<slug>` work via `404.html` redirects in the root and `/docs`.

## Structure

- `index.html` – Landing page
- `docs/` – Markdown docs and docs shell (`index.html`)
- `assets/` – Shared styles and scripts
  - `theme.css` – Dark theme (Nuxt-like green accent)
  - `site.js` – Shared header/footer and temporary access gate
  - `docs-app.js` – Docs viewer: sidebar, clean routing, search, section submenus
- `CNAME`, `.nojekyll`, `404.html`, `docs/404.html` – GitHub Pages support

## Open source

Website source: https://github.com/newmatik/edgesentinel-web
