/**
 * Stand-ins for the Nuxt auto-imports used by the code under test.
 *
 * `nuxt` holds the mocks so each test can set return values and inspect calls;
 * they are reset before every test.
 */
import { beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { computed, ref, type Ref } from 'vue'

export const API_BASE_URL = 'https://api.example.test/api'

const cookies = new Map<string, Ref<string | null>>()

export const nuxt = {
  $fetch: vi.fn(),
  navigateTo: vi.fn(),
  /** Options passed to useCookie, by cookie name (last call wins). */
  cookieOptions: new Map<string, unknown>(),
  publicConfig: {} as Record<string, unknown>,
  cookie(name: string): Ref<string | null> {
    if (!cookies.has(name)) cookies.set(name, ref<string | null>(null))
    return cookies.get(name)!
  },
}

beforeEach(() => {
  cookies.clear()
  nuxt.cookieOptions.clear()
  nuxt.$fetch.mockReset()
  nuxt.navigateTo.mockReset()
  nuxt.publicConfig = { apiBaseUrl: API_BASE_URL }

  setActivePinia(createPinia())

  vi.stubGlobal('$fetch', nuxt.$fetch)
  vi.stubGlobal('navigateTo', nuxt.navigateTo)
  vi.stubGlobal('useRuntimeConfig', () => ({ public: nuxt.publicConfig }))
  vi.stubGlobal('useCookie', (name: string, options?: unknown) => {
    if (options) nuxt.cookieOptions.set(name, options)
    return nuxt.cookie(name)
  })
  vi.stubGlobal('location', { protocol: 'https:' })
})

// Needed while a module loads, so they cannot wait for beforeEach.
vi.stubGlobal('defineNuxtRouteMiddleware', (middleware: unknown) => middleware)
vi.stubGlobal('computed', computed)
