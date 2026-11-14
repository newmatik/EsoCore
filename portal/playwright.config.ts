import { defineConfig, devices } from '@playwright/test'

import { alertCount, apiBase, apiPort, portalBase, portalPort, user } from './e2e/env'

// Browser smoke test of the built portal (`pnpm build` first) against the
// Django API with the seed data. See e2e/smoke.spec.ts and the portal README.
export default defineConfig({
  testDir: './e2e',
  forbidOnly: !!process.env.CI,
  retries: process.env.CI ? 1 : 0,
  workers: 1,
  reporter: process.env.CI ? [['list'], ['html', { open: 'never' }]] : 'list',
  use: {
    ...devices['Desktop Chrome'],
    baseURL: portalBase,
    trace: 'retain-on-failure',
  },
  webServer: [
    {
      command: 'node e2e/start-api.mjs',
      url: `${apiBase}/`,
      timeout: 120_000,
      reuseExistingServer: false,
      env: {
        E2E_API_PORT: String(apiPort),
        E2E_PORTAL_ORIGIN: portalBase,
        E2E_USERNAME: user.username,
        E2E_EMAIL: user.email,
        E2E_PASSWORD: user.password,
        E2E_ALERTS: String(alertCount),
      },
    },
    {
      command: 'node .output/server/index.mjs',
      url: `${portalBase}/login`,
      timeout: 60_000,
      reuseExistingServer: false,
      env: {
        HOST: '127.0.0.1',
        PORT: String(portalPort),
        NUXT_PUBLIC_API_BASE_URL: apiBase,
      },
    },
  ],
})
