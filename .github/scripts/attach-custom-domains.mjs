// Attach hostnames to a Worker as custom domains (idempotent), for deploy-website.yml.
// The first run moves each hostname from its old target (GitHub Pages) to the Worker. It uses the
// endpoint `wrangler deploy` uses for custom domains (PUT .../workers/scripts/<service>/domains/records),
// because the public PUT /workers/domains refuses a hostname that still has DNS records (error 100117)
// and has no override flag. override_existing_dns_record replaces the Pages A/CNAME records,
// override_existing_origin takes a hostname another Worker holds, and override_scope makes the list
// below the Worker's complete set of custom domains. Hostnames already on the Worker are left alone.
// Env: CLOUDFLARE_API_TOKEN, CLOUDFLARE_ACCOUNT_ID, ZONE_ID, SERVICE.
const { CLOUDFLARE_API_TOKEN: token, CLOUDFLARE_ACCOUNT_ID: account, ZONE_ID: zone, SERVICE: service } = process.env
const hosts = process.argv.slice(2)
if (!token || !account || !zone || !service || hosts.length === 0) {
  console.error('usage: attach-custom-domains.mjs <hostname>... (env CLOUDFLARE_API_TOKEN, CLOUDFLARE_ACCOUNT_ID, ZONE_ID, SERVICE)')
  process.exit(2)
}
const base = `https://api.cloudflare.com/client/v4/accounts/${account}/workers`
async function call(url, init = {}) {
  const response = await fetch(url, {
    ...init,
    headers: { Authorization: `Bearer ${token}`, 'Content-Type': 'application/json', ...init.headers },
  })
  const body = await response.json()
  if (!response.ok || !body.success) throw new Error(`${init.method ?? 'GET'} ${url}: ${response.status} ${JSON.stringify(body.errors)}`)
  return body.result
}
const current = new Map()
for (const hostname of hosts) {
  current.set(hostname, (await call(`${base}/domains?hostname=${encodeURIComponent(hostname)}`))[0]?.service ?? '')
}
if (hosts.every((hostname) => current.get(hostname) === service)) {
  console.log(`${hosts.join(', ')} already on ${service}`)
  process.exit(0)
}
await call(`${base}/scripts/${service}/domains/records`, {
  method: 'PUT',
  body: JSON.stringify({
    override_scope: true,
    override_existing_origin: true,
    override_existing_dns_record: true,
    origins: hosts.map((hostname) => ({ hostname, zone_id: zone })),
  }),
})
for (const hostname of hosts) {
  console.log(`${hostname} attached to ${service} (was: ${current.get(hostname) || 'not a Worker custom domain'})`)
}
