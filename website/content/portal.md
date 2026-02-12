# Portal

The EsoCore Portal is a web application that provides the user-facing interface for device monitoring, fleet management,
and data visualization. It connects to the Backend Server APIs and presents real-time dashboards, device management
tools, and alerting interfaces.

---

## Technology Stack

| Component | Technology | Version |
| --- | --- | --- |
| **Framework** | Nuxt 3 | 3.18 |
| **UI Library** | Nuxt UI (Tailwind CSS) | 2.17 |
| **State Management** | Pinia | 2.2 |
| **Charts** | Chart.js + vue-chartjs | 4.4 / 5.3 |
| **HTTP Client** | Axios | 1.7 |
| **Real-time** | socket.io-client (planned) | 4.8 |
| **Auth** | JWT (jwt-decode) | 4.0 |
| **Utilities** | VueUse | 10.11 |
| **Node.js** | 22+ | -- |
| **Package Manager** | pnpm | 9+ |

---

## Features

### Device Dashboard

The main dashboard provides an at-a-glance overview of the entire device fleet:

- Fleet health summary with device counts by status
- Recent telemetry data and trend indicators
- Active alerts and event notifications
- Quick navigation to individual device detail views

### Device Management

- Browse, search, and filter the device registry
- View individual device details: configuration, firmware version, connectivity status
- Monitor real-time sensor readings and historical telemetry charts
- Track device health metrics and uptime

### Asset Monitoring

- Associate devices with physical equipment and assets
- View per-asset dashboards combining data from multiple sensors
- Track maintenance history and upcoming schedules

### Alerting

- View and acknowledge active alerts
- Filter events by severity, type, and time range
- Event detail views with full context and suggested actions

---

## Application Structure

The portal code lives in the `portal/` directory:

```
portal/
├── app/
│   ├── app.vue                # Root application component
│   ├── layouts/
│   │   ├── default.vue        # Public layout
│   │   └── authenticated.vue  # Authenticated layout with sidebar
│   ├── pages/
│   │   ├── index.vue          # Landing/login page
│   │   └── dashboard.vue      # Main dashboard
│   ├── components/
│   │   └── ThemeToggle.vue    # Dark/light mode toggle
│   ├── composables/
│   │   └── useBranding.ts     # Dynamic branding configuration
│   ├── config/
│   │   └── branding.ts        # Brand name, colors, product name
│   ├── middleware/
│   │   └── auth.ts            # Route guard for authenticated pages
│   └── plugins/
│       └── force-dark.client.ts  # Dark mode initialization
├── assets/css/main.css        # Global styles
├── nuxt.config.ts             # Nuxt configuration
├── tailwind.config.js         # Tailwind CSS configuration
└── package.json
```

### Key Architecture Decisions

- **SSR with SPA fallback**: The portal uses server-side rendering by default, but authenticated routes
  (`/dashboard`, `/devices/**`, `/assets/**`, `/alerts/**`) are rendered client-side only for better interactivity.
- **Dark mode by default**: The color mode defaults to dark, matching industrial control room environments.
- **White-label ready**: Branding (product name, company name, colors) is configurable via environment variables
  and the `branding.ts` config, allowing partners to customize the portal for their customers.

---

## Local Development Setup

### Prerequisites

- Node.js 22 or later
- pnpm 9 or later
- The Backend Server running at `http://localhost:8000` (see [Backend Server](/docs/backend-server))

### Installation

```bash
cd portal

# Install dependencies
pnpm install

# Start the development server
pnpm dev
```

The portal will be available at `http://localhost:3000`.

### Environment Variables

| Variable | Default | Description |
| --- | --- | --- |
| `NUXT_PUBLIC_API_BASE_URL` | `http://localhost:8000/api` | Backend API base URL |
| `NUXT_PUBLIC_WS_BASE_URL` | `ws://localhost:8000/ws` | WebSocket URL for real-time updates |
| `NUXT_PUBLIC_AUTH_REDIRECT_URL` | `/dashboard` | Redirect after login |
| `NUXT_PUBLIC_APP_NAME` | `EsoCore` | Application display name |
| `NUXT_PUBLIC_BRAND` | `Newmatik` | Brand name |
| `NUXT_PUBLIC_PRODUCT` | `EsoCore` | Product name |
| `NUXT_PUBLIC_DISPLAY_BRAND` | `true` | Show/hide brand name in UI |

### Available Scripts

```bash
pnpm dev              # Start development server with hot reload
pnpm build            # Build for production
pnpm preview          # Preview production build locally
pnpm lint             # Run ESLint
pnpm lint:fix         # Auto-fix lint issues
pnpm format           # Format code with Prettier
pnpm format:check     # Check formatting
pnpm typecheck        # Run TypeScript type checking
```

### Building for Production

```bash
pnpm build
```

The portal is configured with the `node-server` Nitro preset, producing a Node.js server bundle suitable for
deployment behind a reverse proxy (Nginx, Caddy, etc.).

---

## Connecting to the Backend

The portal communicates with the Backend Server via REST APIs. In development, both services run locally:

| Service | URL | Purpose |
| --- | --- | --- |
| Backend Server | `http://localhost:8000` | Django REST API |
| Portal | `http://localhost:3000` | Nuxt.js frontend |

CORS is configured on the backend to allow requests from the portal origin. In production, both services are
typically served behind the same reverse proxy or domain to simplify CORS handling.

---

## Related Documentation

- [Backend Server](/docs/backend-server) -- The Django REST API that powers the portal
- [API Specification](/docs/api-specification) -- Detailed API endpoint documentation
- [Cloud Infrastructure](/docs/cloud-infrastructure) -- Production deployment and hosting
