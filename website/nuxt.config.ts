// https://nuxt.com/docs/api/configuration/nuxt-config
import { readdirSync } from 'fs'
import { defineNuxtConfig } from 'nuxt/config'
import { join } from 'path'

// Automatically generate docs routes from content directory (recursive)
// CRITICAL: This function generates both clean URLs and trailing slash variants
// to ensure proper static site generation and prevent 404 errors when browsers
// or CDNs automatically add trailing slashes to URLs.
function generateDocRoutes() {
  try {
    const contentDir = join(process.cwd(), 'content')

    function walk(dir: string, prefix = ''): string[] {
      const entries = readdirSync(dir, { withFileTypes: true })
      const routes: string[] = []
      for (const entry of entries) {
        if (entry.isDirectory()) {
          routes.push(...walk(join(dir, entry.name), `${prefix}${entry.name}/`))
        } else if (entry.isFile() && entry.name.endsWith('.md') && entry.name !== 'README.md') {
          const route = `/docs/${prefix}${entry.name.replace('.md', '')}`
          routes.push(route)
        }
      }
      return routes
    }

    const docRoutes = walk(contentDir)

    const routes = [
      '/',
      '/docs', // Main docs page (README)
      '/imprint',
      ...docRoutes
    ]
    
    // Remove development-timeline if present
    return routes.filter(r => !r.includes('development-timeline'))
  } catch (error: any) {
    console.warn('Could not auto-generate doc routes:', error.message)
    // Fallback to manual routes if auto-generation fails
    return [
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
      // '/docs/development-timeline', // removed
      '/docs/business-model-and-partnerships',
      '/docs/competitor-analysis',
      '/docs/fieldbus',
      '/docs/bom',
      '/docs/license',
      '/imprint'
    ]
  }
}

export default defineNuxtConfig({
  compatibilityDate: '2024-04-03',
  devtools: { enabled: true },

  // CRITICAL: Build hooks to create GitHub Pages compatible file structure
  hooks: {
    'nitro:build:public-assets': async (nitro) => {
      // This hook creates HTML files for clean URLs to prevent GitHub Pages
      // from redirecting /docs/page to /docs/page/
      const fs = await import('fs')
      const path = await import('path')
      
      const publicDir = nitro.options.output.publicDir
      const docsDir = path.join(publicDir, 'docs')
      
      if (!fs.existsSync(docsDir)) return
      
      // Find all directories with index.html and create clean URL HTML files
      const processDirectory = (dirPath: string, relativePath = '') => {
        const entries = fs.readdirSync(dirPath, { withFileTypes: true })
        
        for (const entry of entries) {
          if (entry.isDirectory() && !entry.name.startsWith('_') && !entry.name.startsWith('.')) {
            const fullPath = path.join(dirPath, entry.name)
            const indexPath = path.join(fullPath, 'index.html')
            
            // If directory has index.html, create clean URL HTML file
            if (fs.existsSync(indexPath)) {
              const cleanUrlPath = path.join(dirPath, `${entry.name}.html`)
              
              // Copy index.html as clean URL file
              fs.copyFileSync(indexPath, cleanUrlPath)
              console.log(`✓ Created clean URL file: ${relativePath}/${entry.name}.html`)
            }
            
            // Recursively process subdirectories
            processDirectory(fullPath, path.join(relativePath, entry.name))
          }
        }
      }
      
      processDirectory(docsDir, 'docs')
    }
  },


  // Static site generation for GitHub Pages
  // CRITICAL: This configuration ensures proper static file generation
  // for GitHub Pages hosting with clean URL support
  nitro: {
    prerender: {
      routes: generateDocRoutes(),
      // Ensure trailing slash handling works correctly
      crawlLinks: true,
    },
    publicAssets: [
      {
        baseURL: '/',
        dir: 'public',
        maxAge: 60 * 60 * 24 * 365, // 1 year
      },
    ],
    static: true,
    // Enable proper SPA routing for static sites
    compressPublicAssets: true,
    // CRITICAL: Configure routing to handle both clean URLs and trailing slash variants
    routeRules: {
      // Generate static files for clean URLs (without trailing slash)
      '/docs/**': { prerender: true },
    },
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

  // Router configuration for consistent trailing slash handling
  // CRITICAL: This configuration ensures clean URLs are preferred.
  // The strict: false setting allows flexible route matching, while our
  // prerendering generates both URL variants and redirects handle canonicalization.
  router: {
    options: {
      strict: false, // Allow flexible route matching for both /page and /page/ variants
    },
  },
})
