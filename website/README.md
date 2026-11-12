# EsoCore Website

This is the official website for EsoCore - an open-source industrial IoT platform for comprehensive monitoring of factory machinery and equipment.

The website is built with [Nuxt.js](https://nuxt.com) and deployed as static assets on a Cloudflare Worker at
[www.esocore.com](https://www.esocore.com).

## Features

- **Homepage**: Product overview with animated typewriter effect and hero image
- **Documentation**: Complete technical documentation with search and navigation
- **Static Generation**: Pre-rendered for optimal performance and SEO
- **BOM Integration**: Dynamic Bill of Materials widgets for hardware components
- **Responsive Design**: Works on desktop and mobile devices

## Development

### Prerequisites

- Node.js 24 (CI and `.nvmrc` use 24.18.0)
- pnpm 8 or later

### Setup

Install dependencies:

```bash
pnpm install
```

### Development Server

Start the development server on `http://localhost:3081`:

```bash
pnpm dev
```

### Building for Production

Generate the static site:

```bash
pnpm generate
```

The static files will be generated in `.output/public/` and can be deployed to any static hosting service.

### Preview Production Build

Preview the generated static site locally:

```bash
pnpm preview
```

## Project Structure

```text
website/
├── components/         # Vue components (header, footer, etc.)
├── content/            # Markdown documentation files
├── layouts/            # Nuxt.js layouts
├── pages/              # Nuxt.js pages and routing
├── public/             # Static assets (images, CSS, BOM data)
│   ├── bom/            # Synced BOM CSV files
│   └── docs/           # Documentation assets
├── server/             # Server routes and plugins
│   ├── api/            # API routes for serving documentation
│   └── plugins/        # Server plugins (trailing-slash redirect)
├── utils/              # Utility modules (docs-config, etc.)
└── nuxt.config.ts      # Nuxt.js configuration
```

## Documentation

Documentation content is stored as Markdown files in the `content/` directory and served dynamically through API routes. The documentation includes:

- Hardware specifications
- API documentation
- Development guides
- Business information

## BOM Data Synchronization

BOM (Bill of Materials) data is synchronized from the hardware designs using the `sync_bom_data.py` script in the `tools/` directory:

```bash
# From the project root
python3 tools/sync_bom_data.py
```

This copies CSV files from `hardware/` to `website/public/bom/` for use in the documentation.

## Deployment

`.github/workflows/deploy-website.yml` deploys the site when changes to `website/` reach `main`, and an hourly check
deploys `main` when an automatic (Dependabot) merge did not start the workflow. The workflow:

1. Installs dependencies and generates the static site (`pnpm generate` writes `.output/public`)
2. Runs `wrangler deploy` for the Worker `esocore-website` (`wrangler.jsonc`, Newmatik Cloudflare account), with the
   routes `www.esocore.com/*` and `esocore.com/*`. The routes sit on the zone's existing proxied DNS records, which
   still name GitHub Pages but are never contacted; Cloudflare refuses Workers custom domains while those records exist
3. Checks that the site, its headers and the apex redirect answer

`worker/index.js` redirects `esocore.com` to `https://www.esocore.com` and hands every other request to the static
assets. `public/_headers` sets `Referrer-Policy`, `X-Frame-Options`, the CSP `frame-ancestors` and long caching for
`/_nuxt/*`; HSTS and `X-Content-Type-Options` come from the zone settings. Preview the production build locally with
`pnpm generate && pnpm exec wrangler dev`.

## Contributing

1. Make changes to the appropriate files
2. Test locally with `pnpm dev`
3. Generate and test the static build with `pnpm generate && pnpm preview`
4. Commit and push to trigger automatic deployment

For documentation updates, edit the Markdown files in `content/` and they will be automatically included in the next build.
