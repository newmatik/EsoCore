export default defineNitroPlugin((nitroApp) => {
  nitroApp.hooks.hook('request', (event) => {
    const url = event.node.req.url || ''

    // Skip if it's a static asset or API request
    if (url.includes('.') || url.startsWith('/api/') || url.startsWith('/_nuxt/')) {
      return
    }

    // Check if the URL has a trailing slash (but is not the root path)
    if (url.endsWith('/') && url !== '/') {
      // Remove the trailing slash
      const newUrl = url.slice(0, -1)

      // Redirect to the canonical URL without trailing slash
      event.node.res.writeHead(301, {
        Location: newUrl
      })
      event.node.res.end()
    }
  })
})
