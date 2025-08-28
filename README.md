# EdgeSentinel – Monorepo (Docs, Hardware, Firmware, Software)

This repository contains the public documentation website and the source for hardware, firmware, and supporting software for EdgeSentinel.

## Local development

From the project root, start any static server:

```sh
npx serve -l 3000
# or any static server (python3 -m http.server 3000)
```

Open `http://localhost:3000/website/` (landing) and `http://localhost:3000/website/docs/` (docs).

## Temporary access protection

The site can be temporarily protected with a simple client-side access code prompt for preview deployments. To enable or change this during development, edit `website/assets/site.js` (`ACCESS_CODE`). The unlock persists for the browser tab (`sessionStorage`) and is intended only to keep previews private while iterating. Remove or disable before production.

## Deployment (GitHub Pages)

Deployed from `website/` on the `main` branch with a custom domain.

1. Push to `main`.
2. In GitHub → Settings → Pages → Build and deployment:
   - Source: Deploy from branch
   - Branch: `main`
   - Folder: `/website`
3. Set custom domain to `www.edgesentinel.de` (CNAME is in `website/CNAME`). Ensure DNS points to GitHub Pages.

Deep links under the docs app work via `website/docs/404.html` fallback routing.

## Structure

- `website/` – Public website (published folder)
  - `index.html` – Landing page
  - `404.html`, `.nojekyll`, `CNAME` – GitHub Pages support files
  - `assets/` – Shared styles and scripts used by the site and docs app
    - `theme.css` – Theme
    - `site.js` – Shared header/footer and temporary access gate
    - `docs-app.js` – Docs viewer: sidebar, clean routing, search, section submenus
  - `docs/` – Documentation content and shell
    - `index.html` – Docs shell
    - `data/` – Public data used by the docs (BOM CSV copies for rendering)
    - `404.html` – Docs deep-link fallback
- `hardware/` – Hardware sources
  - `bom/` – Canonical Bill of Materials CSVs (author/edit here)
  - `altium/` – PCB projects (add here)
  - `mechanical/` – STEP/drawings (add here)
- `firmware/` – Device and board-support firmware
  - `stm32/`, `common/`, etc.
- `software/` – Host-side tools/services
- `tools/` – Utility scripts (e.g., sync, build, release)

Notes on BOMs:
- The canonical BOM CSVs live in `hardware/bom/`.
- The docs app reads CSVs from `website/docs/data/` for public rendering.
- After editing CSVs in `hardware/bom/`, copy them to `website/docs/data/` to update the site. We can add a small sync script in `tools/` if desired.

## Open source

Website source: [github.com/newmatik/edgesentinel-web](https://github.com/newmatik/edgesentinel-web)

## Protocols

The hardware requirements and BOM include options for industrial fieldbus protocols:

- Modbus RTU (RS‑485, isolated) and Modbus TCP (Ethernet)
- PROFIBUS DP (DB‑9, switchable termination)
- PROFINET Device (over Ethernet)
