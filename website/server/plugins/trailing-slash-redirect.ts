// CRITICAL: This plugin handles trailing slash redirects for the documentation site.
// It ensures that URLs with trailing slashes redirect to clean URLs without trailing slashes,
// maintaining SEO consistency and preventing duplicate content issues.
// 
// IMPORTANT: This plugin is DISABLED during development and static generation to prevent
// redirect loops. It only runs in production environments where GitHub Pages _redirects
// file might not be sufficient.
//
// DO NOT REMOVE OR MODIFY without understanding the full routing system including:
// - nuxt.config.ts route generation
// - _redirects file for GitHub Pages
// - Static site generation requirements
export default defineNitroPlugin((nitroApp) => {
  // DISABLED: This plugin is currently disabled to prevent redirect loops
  // The _redirects file handles trailing slash normalization for static hosting
  // Uncomment only if needed for specific hosting environments that don't support _redirects
  
  /*
  nitroApp.hooks.hook('request', (event) => {
    const url = event.node.req.url || ''

    // Skip if it's a static asset, API request, or Nuxt internal routes
    if (url.includes('.') || url.startsWith('/api/') || url.startsWith('/_nuxt/') || url.startsWith('/__nuxt_error')) {
      return
    }

    // Skip during development and static generation to prevent redirect loops
    if (process.env.NODE_ENV === 'development' || process.env.NUXT_COMMAND === 'generate') {
      return
    }

    // Check if the URL has a trailing slash (but is not the root path)
    if (url.endsWith('/') && url !== '/') {
      // Remove the trailing slash for canonical URL
      const newUrl = url.slice(0, -1)

      // Redirect to the canonical URL without trailing slash
      event.node.res.writeHead(301, {
        Location: newUrl,
        'Cache-Control': 'public, max-age=31536000' // Cache redirects for 1 year
      })
      event.node.res.end()
    }
  })
  */
})
