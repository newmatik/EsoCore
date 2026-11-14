// Settings shared by playwright.config.ts and the smoke test. Ports default
// to values outside the development ports (8000, 3080), so the suite can run
// while the dev servers are up.
export const apiPort = Number(process.env.E2E_API_PORT || 18000)
export const portalPort = Number(process.env.E2E_PORTAL_PORT || 13080)
export const apiBase = `http://127.0.0.1:${apiPort}/api`
export const portalBase = `http://127.0.0.1:${portalPort}`

// The smoke test signs in with a user of its own, created by e2e/fixtures.py.
export const user = {
  username: 'smoke-test',
  email: 'smoke-test@esocore.local',
  password: 'smoke-test-password',
}

// More alerts than the API's default page of 100 rows (#129).
export const alertCount = 130
