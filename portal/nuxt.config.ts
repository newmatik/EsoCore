import { mkdirSync, writeFileSync, existsSync } from 'node:fs'
import { resolve } from 'node:path'

// https://nuxt.com/docs/api/configuration/nuxt-config
export default defineNuxtConfig({
  compatibilityDate: '2025-01-01',
  devtools: { enabled: true },

  // Workaround for Nuxt 4.2.x missing client.precomputed.mjs (nuxt/nuxt#33579)
  hooks: {
    'vite:serverCreated'() {
      const dir = resolve('.nuxt', 'dist', 'server')
      const file = resolve(dir, 'client.precomputed.mjs')
      if (!existsSync(file)) {
        mkdirSync(dir, { recursive: true })
        writeFileSync(file, 'export default {}')
      }
    },
  },

  // Modules
  modules: ['@nuxt/ui', '@pinia/nuxt'],

  // CSS
  css: ['~/assets/css/main.css'],

  // Environment variables
  runtimeConfig: {
    public: {
      apiBaseUrl: process.env.NUXT_PUBLIC_API_BASE_URL || 'http://localhost:8000/api',
      wsBaseUrl: process.env.NUXT_PUBLIC_WS_BASE_URL || 'ws://localhost:8000/ws',
      authRedirectUrl: process.env.NUXT_PUBLIC_AUTH_REDIRECT_URL || '/dashboard',
      appName: process.env.NUXT_PUBLIC_APP_NAME || 'EsoCore',
      appVersion: process.env.NUXT_PUBLIC_APP_VERSION || '1.0.0',
      brand: process.env.NUXT_PUBLIC_BRAND || 'Newmatik',
      product: process.env.NUXT_PUBLIC_PRODUCT || 'EsoCore',
      displayBrand: (process.env.NUXT_PUBLIC_DISPLAY_BRAND || 'true') === 'true',
    },
  },

  // Color mode (handled by @nuxt/ui)
  colorMode: {
    preference: 'dark',
  },

  // Build configuration
  nitro: {
    preset: 'node-server',
  },

  // TypeScript
  typescript: {
    strict: true,
    typeCheck: false,
  },

  // Vue configuration
  vue: {
    compilerOptions: {
      isCustomElement: tag => tag.startsWith('ui-'),
    },
  },

  // Development server
  devServer: {
    port: 3000,
    host: '0.0.0.0',
  },

  // SSR
  ssr: true,

  // Route rules for SPA-like behavior on certain routes
  routeRules: {
    '/dashboard': { ssr: false },
    '/devices/**': { ssr: false },
    '/assets/**': { ssr: false },
    '/alerts/**': { ssr: false },
  },

  // Head configuration
  app: {
    head: {
      title: 'EsoCore Portal',
      meta: [
        { charset: 'utf-8' },
        { name: 'viewport', content: 'width=device-width, initial-scale=1' },
        { name: 'description', content: 'EsoCore IoT Device Monitoring and Management Portal' },
      ],
      link: [
        { rel: 'icon', type: 'image/png', href: '/icon.png' },
        { rel: 'icon', type: 'image/svg+xml', href: '/icon.svg' },
        { rel: 'apple-touch-icon', href: '/icon.png' },
      ],
    },
  },
})
