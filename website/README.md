# EdgeSentinel Website

This directory contains the static public website and the documentation app.

## Local development

From the repository root, start any static server and open these paths:

```sh
npx serve -l 3000
# or: python3 -m http.server 3000
```

- Landing: <http://localhost:3000/website/>
- Docs: <http://localhost:3000/website/docs/>

## Temporary access protection

For preview deployments only, a client-side access code prompt can be enabled.
To change the code, edit `website/assets/site.js` (`ACCESS_CODE`). The unlock
is stored in `sessionStorage` and applies to the current browser tab only.

## Deployment (GitHub Pages)

- Settings → Pages → Build and deployment:
  - Source: GitHub Actions
- Workflow: `.github/workflows/pages.yml` uploads `website/` and deploys on pushes to `main`.
- Optional: trigger manually via “Run workflow”.
- Custom domain: `www.edgesentinel.de` (`website/CNAME` is included in the artifact)
- Docs deep links are handled by `website/docs/404.html`

## Structure

- `index.html` – Landing page
- `404.html`, `.nojekyll`, `CNAME` – Pages support
- `assets/`
  - `theme.css` – Theme
  - `site.js` – Shared header/footer and access gate
  - `docs-app.js` – Docs viewer (sidebar, routing, search)
- `docs/`
  - `index.html` – Docs shell
  - `data/` – Public CSVs used by the docs (BOM copies)
  - `*.md` – Documentation content
  - `404.html` – Docs deep-link fallback

Note: The canonical BOM CSVs live in `hardware/bom/` at the repository root.
Copy them to `website/docs/data/` to publish updates on the site.
