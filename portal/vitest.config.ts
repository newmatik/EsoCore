import { defineConfig } from 'vitest/config'

// Unit tests run in plain Node without booting Nuxt. The Nuxt auto-imports the
// code under test relies on are stubbed in tests/setup.ts, and the tests run as
// the client side of the app (the auth cookie is only handled there).
export default defineConfig({
  define: {
    'import.meta.client': 'true',
    'import.meta.server': 'false',
  },
  test: {
    environment: 'node',
    include: ['tests/**/*.test.ts'],
    setupFiles: ['tests/setup.ts'],
    restoreMocks: true,
  },
})
