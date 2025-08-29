# EsoCore Portal (Nuxt.js Frontend)

Modern Vue.js frontend application for the EsoCore IoT monitoring system, built with Nuxt.js.

## Features

- **Dashboard**: Real-time device monitoring and telemetry visualization
- **Device Management**: Device provisioning, configuration, and status monitoring
- **Asset Tracking**: Equipment monitoring and maintenance scheduling
- **Alert Management**: Real-time alerts and notification management
- **User Management**: Role-based access control and user profiles
- **Responsive Design**: Mobile-first responsive web interface

## Technology Stack

- **Framework**: Nuxt.js 4.0 (Vue.js 3)
- **UI Library**: Nuxt UI (Tailwind CSS + Headless UI)
- **Charts**: Chart.js or ApexCharts for data visualization
- **State Management**: Pinia for Vue state management
- **HTTP Client**: Axios for API communication
- **Authentication**: JWT-based authentication
- **Real-time**: WebSocket support for live updates

## Quick Start

### Prerequisites

- Node.js 22+
- pnpm (recommended) or npm/yarn

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

### Nuxt Configuration

The main configuration is in `nuxt.config.ts`. Key settings include:

- **SSR/SSG**: Universal rendering for SEO and performance
- **API Proxy**: Development proxy to Django backend
- **CSS**: Tailwind CSS integration
- **Modules**: UI components, charts, and utilities

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
├── app/                          # Nuxt app directory
│   ├── app.vue                   # Root component
│   ├── layouts/                  # Layout components
│   │   ├── default.vue          # Default layout
│   │   └── auth.vue             # Authentication layout
│   ├── pages/                   # Page components (file-based routing)
│   │   ├── index.vue            # Home page
│   │   ├── dashboard.vue        # Main dashboard
│   │   ├── devices/             # Device management pages
│   │   ├── assets/              # Asset management pages
│   │   ├── alerts/              # Alert management pages
│   │   └── users/               # User management pages
│   ├── components/              # Reusable components
│   │   ├── ui/                  # UI components
│   │   ├── charts/              # Chart components
│   │   └── forms/               # Form components
│   ├── composables/             # Vue composables
│   │   ├── useApi.ts            # API communication
│   │   ├── useAuth.ts           # Authentication
│   │   └── useWebSocket.ts      # WebSocket connection
│   ├── plugins/                 # Nuxt plugins
│   │   ├── api.client.ts        # API client setup
│   │   └── charts.client.ts     # Chart library setup
│   └── middleware/              # Route middleware
│       └── auth.ts              # Authentication middleware
├── public/                      # Static assets
├── server/                      # Server-side code (if needed)
├── nuxt.config.ts               # Nuxt configuration
├── tailwind.config.js           # Tailwind CSS configuration
└── package.json                 # Dependencies and scripts
```

## API Integration

### Authentication

The portal integrates with Django's authentication system:

```typescript
// Login
const { login } = useAuth()
await login({ username, password })

// API calls with automatic token handling
const { data } = await $api('/devices/')
```

### Real-time Updates

WebSocket integration for live data:

```typescript
const { connect, subscribe } = useWebSocket()

// Connect to WebSocket
connect()

// Subscribe to device updates
subscribe('devices', (data) => {
  // Handle real-time updates
})
```

## Development

### Code Style

```bash
# Lint code
pnpm run lint

# Format code
pnpm run format

# Type check
pnpm run typecheck
```

### Testing

```bash
# Unit tests
pnpm run test

# E2E tests
pnpm run test:e2e
```

### Storybook (Component Development)

```bash
# Start Storybook
pnpm run storybook
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

### Static Site Generation

For static hosting:

```bash
npm run generate
```

This creates a `dist/` folder with static files ready for deployment.

## Key Features Implementation

### Dashboard

- Real-time device status overview
- Telemetry data visualization with interactive charts
- Recent alerts and notifications
- Quick actions for common tasks

### Device Management

- Device listing with filtering and search
- Device details and configuration
- Firmware update management
- Device provisioning workflow

### Alert System

- Alert dashboard with severity-based filtering
- Alert acknowledgment and resolution
- Notification preferences
- Escalation management

### User Management

- User profiles and role management
- Site and device access control
- Customizable dashboards
- Notification settings

## Contributing

1. Follow Vue.js and Nuxt.js best practices
2. Use TypeScript for type safety
3. Write tests for new features
4. Follow the established component structure
5. Use meaningful commit messages

## License

Licensed under the Apache License, Version 2.0.
