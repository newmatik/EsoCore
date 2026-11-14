// Browser smoke test of the portal: sign-in, the dashboard counters, the full
// lists (more than one API page of alerts, #129), a device detail page, and no
// sideways scrolling at phone width (#138). Run with `pnpm build && pnpm test:e2e`.
import { expect, test, type APIRequestContext, type Page } from '@playwright/test'

import { alertCount, apiBase, user } from './env'

async function apiToken(request: APIRequestContext) {
  const response = await request.post(`${apiBase}/auth/login/`, {
    data: { username: user.username, password: user.password },
  })
  expect(response.ok()).toBe(true)
  return ((await response.json()) as { token: string }).token
}

async function apiGet<T>(request: APIRequestContext, token: string, path: string) {
  const response = await request.get(`${apiBase}${path}`, {
    headers: { Authorization: `Token ${token}` },
  })
  expect(response.ok()).toBe(true)
  return (await response.json()) as T
}

async function apiCount(request: APIRequestContext, token: string, path: string) {
  return (await apiGet<{ count: number }>(request, token, path)).count
}

/** Collects uncaught page errors and failed or unsuccessful API requests. */
function watchProblems(page: Page) {
  const problems: string[] = []
  page.on('pageerror', error => problems.push(`page error: ${error.message}`))
  page.on('requestfailed', request => {
    if (request.url().startsWith(apiBase)) {
      problems.push(`failed request: ${request.url()} ${request.failure()?.errorText}`)
    }
  })
  page.on('response', response => {
    if (response.url().startsWith(apiBase) && response.status() >= 400) {
      problems.push(`HTTP ${response.status()}: ${response.url()}`)
    }
  })
  return problems
}

async function signIn(page: Page) {
  await page.goto('/login')
  // Fill the form only after hydration, so the submit runs the app's handler.
  await page.waitForLoadState('networkidle')
  await page.locator('#email').fill(user.email)
  await page.locator('#password').fill(user.password)
  await page.getByRole('button', { name: 'Sign in' }).click()
  await expect(page).toHaveURL(/\/dashboard$/)
}

async function horizontalOverflow(page: Page) {
  return page.evaluate(
    () => document.documentElement.scrollWidth - document.documentElement.clientWidth
  )
}

test('sign in, dashboard, full lists and a device page', async ({ page, request }) => {
  const problems = watchProblems(page)
  const token = await apiToken(request)

  await signIn(page)

  const summary = await apiGet<Record<string, number>>(request, token, '/dashboard/summary/')
  const expected = [
    summary.total_devices,
    summary.online_devices,
    summary.active_alerts,
    summary.total_assets,
  ].map(String)
  await expect(page.getByTestId('stat-value')).toHaveText(expected)

  const lists = [
    { path: '/devices', testId: 'device-row', api: '/devices/devices/' },
    { path: '/alerts', testId: 'alert-row', api: '/events/events/' },
    { path: '/assets', testId: 'asset-card', api: '/assets/assets/' },
  ]
  for (const list of lists) {
    const count = await apiCount(request, token, list.api)
    expect(count, `${list.api} has rows`).toBeGreaterThan(0)
    await page.goto(list.path)
    await expect(page.getByTestId(list.testId), `${list.path} shows every row`).toHaveCount(count)
  }
  expect(await apiCount(request, token, '/events/events/')).toBeGreaterThanOrEqual(alertCount)

  await page.goto('/devices')
  const firstDevice = page.getByTestId('device-row').first()
  const serial = (await firstDevice.locator('td p').first().innerText()).trim()
  await firstDevice.getByRole('link', { name: 'Details' }).click()
  await expect(page).toHaveURL(/\/devices\/[0-9a-f-]+$/)
  await expect(page.getByRole('heading', { level: 1 })).toHaveText(serial)
  await expect(page.getByRole('link', { name: 'Back to devices' })).toBeVisible()

  expect(problems).toEqual([])
})

test.describe('phone width', () => {
  test.use({ viewport: { width: 390, height: 844 } })

  test('no page is wider than the screen', async ({ page }) => {
    const problems = watchProblems(page)
    await signIn(page)

    await page.goto('/devices')
    const deviceLink = await page
      .getByRole('link', { name: 'Details' })
      .first()
      .getAttribute('href')
    expect(deviceLink).toBeTruthy()

    for (const path of ['/dashboard', '/devices', deviceLink!, '/alerts', '/assets']) {
      await page.goto(path)
      await page.waitForLoadState('networkidle')
      expect(await horizontalOverflow(page), `${path} scrolls sideways`).toBe(0)
    }

    // The navigation stays reachable through the menu button.
    await page.getByRole('button', { name: 'Open navigation' }).click()
    await page.locator('#mobile-nav').getByRole('link', { name: 'Alerts' }).click()
    await expect(page).toHaveURL(/\/alerts$/)

    expect(problems).toEqual([])
  })
})
