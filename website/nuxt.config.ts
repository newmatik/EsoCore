// https://nuxt.com/docs/api/configuration/nuxt-config
export default defineNuxtConfig({
  compatibilityDate: '2024-04-03',
  devtools: { enabled: true },

  // Static site generation for GitHub Pages
  nitro: {
    prerender: {
      routes: [
        '/',
        '/docs',
        '/docs/esocore-edge',
        '/docs/esocore-sensors',
        '/docs/edge-intelligence',
        '/docs/data-format-specification',
        '/docs/api-specification',
        '/docs/cloud-infrastructure',
        '/docs/testing-procedures',
        '/docs/development-environment',
        '/docs/development-timeline',
        '/docs/business-model-and-partnerships',
        '/docs/competitor-analysis',
        '/docs/fieldbus',
        '/docs/bom',
        '/docs/license',
        '/imprint'
      ],
      fallback: '404.html'
    },
    publicAssets: [
      {
        baseURL: '/',
        dir: 'public',
        maxAge: 60 * 60 * 24 * 365, // 1 year
      },
    ],
  },

  // App configuration
  app: {
    head: {
      htmlAttrs: {
        lang: 'en',
      },
      meta: [
        { charset: 'utf-8' },
        { name: 'viewport', content: 'width=device-width, initial-scale=1' },
        { name: 'color-scheme', content: 'dark' },
        { name: 'theme-color', content: '#0b0f14' },
        {
          name: 'description',
          content:
            'EsoCore brings edge-native intelligence, observability and control in a sleek, minimal, open-source package.',
        },
        { property: 'og:title', content: 'EsoCore' },
        {
          property: 'og:description',
          content: 'Edge-native intelligence, observability and control.',
        },
        { property: 'og:type', content: 'website' },
      ],
      link: [
        { rel: 'icon', href: '/icon.svg', type: 'image/svg+xml' },
        { rel: 'icon', href: '/icon.png', type: 'image/png' },
        { rel: 'preconnect', href: 'https://fonts.googleapis.com' },
        { rel: 'preconnect', href: 'https://fonts.gstatic.com', crossorigin: '' },
        {
          rel: 'stylesheet',
          href: 'https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&display=swap',
        },
        { rel: 'stylesheet', href: '/theme.css' },
      ],
    },
  },

  // CSS configuration
  css: [],

  // Modules
  modules: [],

  // Router configuration
  router: {
    options: {
      strict: false,
    },
  },
})
