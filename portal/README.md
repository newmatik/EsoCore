# EsoCore Portal

Modern Vue.js frontend application for the EsoCore IoT monitoring system, built with Nuxt.js.

## Features

- **Dashboard**: Device monitoring overview with telemetry stats and recent alerts
- **Device Management**: Device listing with filtering, detail views with telemetry history
- **Asset Tracking**: Equipment grid with status filtering
- **Alert Management**: Alerts with severity filtering, acknowledgment, and resolution
- **Authentication**: Token-based authentication with cookie persistence
- **Dark Mode**: Forced dark theme with toggle support
- **Branding**: Configurable brand and product names via environment variables

## Technology Stack

- **Framework**: Nuxt 4 (Vue 3)
- **UI Library**: Nuxt UI (Tailwind CSS 4)
- **State Management**: Pinia
- **HTTP Client**: Nuxt `$fetch` (ofetch) with custom `useApi` composable
- **Authentication**: DRF Token authentication with cookie persistence
- **Icons**: Lucide (via Iconify)

## Quick Start

### Prerequisites

- Node.js 22+
- pnpm 9+

### Installation

1. **Navigate to portal directory:**

   ```bash
   cd portal
   ```

2. **Install dependencies:**

   ```bash
   pnpm install
   ```

3. **Start development server:**

   ```bash
   pnpm run dev
   ```

The application will be available at `http://localhost:3000`

### Production Build

```bash
# Build for production
pnpm run build

# Preview production build
pnpm run preview

# Generate static site
pnpm run generate
```

## Configuration

### Environment Variables

Create a `.env` file in the portal directory:

```bash
# API Configuration
NUXT_PUBLIC_API_BASE_URL=http://localhost:8000/api
NUXT_PUBLIC_WS_BASE_URL=ws://localhost:8000/ws

# Authentication
NUXT_PUBLIC_AUTH_REDIRECT_URL=/dashboard

# App Configuration
NUXT_PUBLIC_APP_NAME=EsoCore
NUXT_PUBLIC_APP_VERSION=1.0.0

# Branding (optional)
# Set your brand and product names for UI titles and headers
# Example: Brand: Newmatik, Product: EsoCore => "EsoCore Portal"
NUXT_PUBLIC_BRAND=Newmatik
NUXT_PUBLIC_PRODUCT=EsoCore
```

### Branding

You can customize the displayed product name without code changes:

1. Set environment variables in your `.env` (or deployment env):

   ```bash
   NUXT_PUBLIC_BRAND=Newmatik
   NUXT_PUBLIC_PRODUCT=EsoCore
   ```

2. The UI will render headers and titles like: `EsoCore Portal`.

Defaults are defined in `app/config/branding.ts` and can be overridden at runtime via env vars.

## Project Structure

```text
portal/
├── app/
│   ├── app.vue                       # Root component
│   ├── assets/css/main.css           # Global CSS
│   ├── components/
│   │   └── ThemeToggle.vue           # Dark/light mode toggle
│   ├── composables/
│   │   ├── useApi.ts                 # API client ($fetch wrapper with auth)
│   │   └── useBranding.ts            # Brand/product name composable
│   ├── config/
│   │   └── branding.ts               # Branding defaults
│   ├── layouts/
│   │   ├── authenticated.vue         # Layout for authenticated pages
│   │   └── default.vue               # Default layout
│   ├── middleware/
│   │   └── auth.ts                   # Authentication route guard
│   ├── pages/
│   │   ├── index.vue                 # Landing page
│   │   ├── login.vue                 # Login page
│   │   ├── dashboard.vue             # Dashboard with stats and recent alerts
│   │   ├── alerts/index.vue          # Alert list with filtering and actions
│   │   ├── assets/index.vue          # Asset grid with status filtering
│   │   └── devices/
│   │       ├── index.vue             # Device list with site filtering
│   │       └── [id].vue              # Device detail with telemetry and events
│   ├── plugins/
│   │   ├── auth.client.ts            # Auth token restoration on startup
│   │   └── force-dark.client.ts      # Force dark mode on load
│   └── stores/
│       └── auth.ts                   # Auth state (token, user, login/logout)
├── public/                           # Static assets (icons, robots.txt)
├── app.config.ts                     # App-level configuration
├── eslint.config.js                  # ESLint configuration
├── nuxt.config.ts                    # Nuxt configuration
├── package.json                      # Dependencies and scripts
├── pnpm-lock.yaml                    # Lockfile
└── tsconfig.json                     # TypeScript configuration
```

## API Integration

The portal communicates with the Django REST backend through the `useApi` composable, which wraps
Nuxt's `$fetch` with automatic base URL resolution, auth token injection, and 401 redirect handling.

### Authentication

```typescript
const authStore = useAuthStore()
await authStore.login(email, password)

// Authenticated API calls
const api = useApi()
const devices = await api.get('/devices/devices/')
```

### API Endpoints Used

- **Auth**: `/auth/login/`, `/auth/logout/`, `/auth/me/`
- **Dashboard**: `/dashboard/summary/`
- **Devices**: `/devices/devices/`, `/devices/sites/`
- **Telemetry**: `/telemetry/points/`
- **Events**: `/events/events/`
- **Assets**: `/assets/assets/`

## Development

### Code Style

```bash
# Lint code
pnpm run lint

# Auto-fix lint issues
pnpm run lint:fix

# Format code
pnpm run format

# Check formatting
pnpm run format:check

# Type check
pnpm run typecheck
```

## Deployment

### Docker

```dockerfile
FROM node:22-alpine
WORKDIR /app
ENV NODE_ENV=production
# Use pnpm via Corepack (bundled with Node 22)
RUN corepack enable
COPY package.json pnpm-lock.yaml ./
RUN pnpm install --frozen-lockfile
COPY . .
RUN pnpm run build
EXPOSE 3000
CMD ["node", ".output/server/index.mjs"]
```

## Contributing

1. Follow Vue.js and Nuxt.js best practices
2. Use TypeScript for type safety
3. Follow the established component structure
4. Use meaningful commit messages

## License

Copyright 2026 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/content/license.md` for details.
