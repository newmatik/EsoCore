// esocore-website Worker: serves the prerendered site (.output/public) as static assets.
// It runs before the assets (run_worker_first in wrangler.jsonc) only to send the apex
// domain to www, which GitHub Pages used to do. Every other request goes to the assets,
// where public/_headers sets the security and cache headers.
const CANONICAL_HOST = 'www.esocore.com'

export default {
  async fetch(request, env) {
    const url = new URL(request.url)
    if (url.hostname === 'esocore.com') {
      url.hostname = CANONICAL_HOST
      url.protocol = 'https:'
      url.port = ''
      return Response.redirect(url.toString(), 301)
    }
    return env.ASSETS.fetch(request)
  },
}
