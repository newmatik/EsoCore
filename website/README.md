# EsoCore Website

This is the official website for EsoCore - an open-source industrial IoT platform for comprehensive monitoring of factory machinery and equipment.

The website is built with [Nuxt.js](https://nuxt.com) and deployed as a static site to GitHub Pages at [www.esocore.com](https://www.esocore.com).

## Features

- **Homepage**: Product overview with animated typewriter effect and hero image
- **Documentation**: Complete technical documentation with search and navigation
- **Static Generation**: Pre-rendered for optimal performance and SEO
- **BOM Integration**: Dynamic Bill of Materials widgets for hardware components
- **Responsive Design**: Works on desktop and mobile devices

## Development

### Prerequisites

- Node.js 18 or later
- pnpm package manager

### Setup

Install dependencies:

```bash
pnpm install
```

### Development Server

Start the development server on `http://localhost:3000`:

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
├── components/          # Vue components (header, footer, etc.)
├── content/            # Markdown documentation files
├── layouts/            # Nuxt.js layouts
├── pages/              # Nuxt.js pages and routing
├── public/             # Static assets (images, CSS, BOM data)
├── server/             # API routes for serving documentation
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

This copies CSV files from `hardware/` to `website/public/docs/data/` for use in the documentation.

## Deployment

The website is automatically deployed to GitHub Pages via GitHub Actions when changes are pushed to the `main` branch. The workflow:

1. Sets up Node.js and pnpm
2. Installs dependencies
3. Generates the static site
4. Deploys to GitHub Pages

## Contributing

1. Make changes to the appropriate files
2. Test locally with `pnpm dev`
3. Generate and test the static build with `pnpm generate && pnpm preview`
4. Commit and push to trigger automatic deployment

For documentation updates, edit the Markdown files in `content/` and they will be automatically included in the next build.
