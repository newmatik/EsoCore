// Attach hostnames to a Worker as custom domains (idempotent), for deploy-website.yml.
// The first run moves each hostname from its old target (GitHub Pages) to the Worker:
// override_existing_dns_record replaces the existing DNS records. A hostname already on
// the Worker is left alone. Env: CLOUDFLARE_API_TOKEN, CLOUDFLARE_ACCOUNT_ID, ZONE_ID, SERVICE.
const { CLOUDFLARE_API_TOKEN: token, CLOUDFLARE_ACCOUNT_ID: account, ZONE_ID: zone, SERVICE: service } = process.env
const hosts = process.argv.slice(2)
if (!token || !account || !zone || !service || hosts.length === 0) {
  console.error('usage: attach-custom-domains.mjs <hostname>... (env CLOUDFLARE_API_TOKEN, CLOUDFLARE_ACCOUNT_ID, ZONE_ID, SERVICE)')
  process.exit(2)
}
const api = `https://api.cloudflare.com/client/v4/accounts/${account}/workers/domains`
async function call(url, init = {}) {
  const response = await fetch(url, {
    ...init,
    headers: { Authorization: `Bearer ${token}`, 'Content-Type': 'application/json', ...init.headers },
  })
  const body = await response.json()
  if (!response.ok || !body.success) throw new Error(`${init.method ?? 'GET'} ${url}: ${response.status} ${JSON.stringify(body.errors)}`)
  return body.result
}
for (const hostname of hosts) {
  const current = (await call(`${api}?hostname=${encodeURIComponent(hostname)}`))[0]?.service ?? ''
  if (current === service) {
    console.log(`${hostname} already on ${service}`)
    continue
  }
  await call(api, {
    method: 'PUT',
    body: JSON.stringify({ hostname, service, zone_id: zone, environment: 'production', override_existing_dns_record: true }),
  })
  console.log(`${hostname} attached to ${service} (was: ${current || 'not a Worker custom domain'})`)
}
