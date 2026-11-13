import { describe, expect, it } from 'vitest'
import { useApi } from '../app/composables/useApi'
import { useAuthStore } from '../app/stores/auth'
import { API_BASE_URL, nuxt } from './setup'

/** Path and options of the only $fetch call made. */
function fetchCall() {
  expect(nuxt.$fetch).toHaveBeenCalledTimes(1)
  const [path, options] = nuxt.$fetch.mock.calls[0]!
  return { path, options }
}

describe('useApi', () => {
  it('sends requests to the configured API base URL and returns the response', async () => {
    nuxt.$fetch.mockResolvedValue({ count: 0, results: [] })

    const result = await useApi().get('/devices/devices/', { page_size: 8 })

    expect(result).toEqual({ count: 0, results: [] })
    const { path, options } = fetchCall()
    expect(path).toBe('/devices/devices/')
    expect(options).toMatchObject({
      baseURL: API_BASE_URL,
      method: 'GET',
      query: { page_size: 8 },
    })
  })

  it('sends no Authorization header while signed out', async () => {
    nuxt.$fetch.mockResolvedValue({})

    await useApi().get('/dashboard/summary/')

    expect(fetchCall().options.headers).toEqual({})
  })

  it('sends the token of the signed-in user', async () => {
    nuxt.$fetch.mockResolvedValue({})
    useAuthStore().token = 'abc123'

    await useApi().get('/dashboard/summary/')

    expect(fetchCall().options.headers).toEqual({ Authorization: 'Token abc123' })
  })

  it.each([
    ['post', 'POST'],
    ['put', 'PUT'],
    ['patch', 'PATCH'],
  ] as const)('%s sends the body as JSON', async (helper, method) => {
    nuxt.$fetch.mockResolvedValue({})

    await useApi()[helper]('/events/events/7/', { status: 'resolved' })

    const { options } = fetchCall()
    expect(options.method).toBe(method)
    expect(options.body).toEqual({ status: 'resolved' })
    expect(options.headers).toEqual({ 'Content-Type': 'application/json' })
  })

  it('del sends a DELETE request', async () => {
    nuxt.$fetch.mockResolvedValue(undefined)

    await useApi().del('/devices/devices/7/')

    expect(fetchCall().options.method).toBe('DELETE')
  })

  it('keeps a Content-Type the caller set and adds its other headers', async () => {
    nuxt.$fetch.mockResolvedValue({})

    await useApi().request('/firmware/upload/', {
      method: 'POST',
      headers: { 'Content-Type': 'application/octet-stream', 'X-Request-Id': '42' },
    })

    expect(fetchCall().options.headers).toEqual({
      'Content-Type': 'application/octet-stream',
      'X-Request-Id': '42',
    })
  })

  describe('getAll', () => {
    /** Serves `total` numbered rows in pages, like the API's pagination. */
    function servePages(total: number) {
      nuxt.$fetch.mockImplementation(
        async (_path: string, options: { query: { page: number; page_size: number } }) => {
          const { page, page_size: pageSize } = options.query
          const start = (page - 1) * pageSize
          const results = Array.from(
            { length: Math.max(0, Math.min(pageSize, total - start)) },
            (_, index) => ({ id: start + index + 1 })
          )
          return { count: total, next: start + pageSize < total ? 'next-page-url' : null, results }
        }
      )
    }

    it('returns a list that fits one page with a single request', async () => {
      servePages(3)

      const { items, total } = await useApi().getAll('/devices/devices/')

      expect(items).toEqual([{ id: 1 }, { id: 2 }, { id: 3 }])
      expect(total).toBe(3)
      expect(nuxt.$fetch).toHaveBeenCalledTimes(1)
    })

    it('follows the pages until the list is complete', async () => {
      servePages(1200)

      const { items, total } = await useApi().getAll<{ id: number }>('/events/events/', {
        ordering: '-created_at',
      })

      expect(items).toHaveLength(1200)
      expect(items[0]).toEqual({ id: 1 })
      expect(items[1199]).toEqual({ id: 1200 })
      expect(total).toBe(1200)
      expect(nuxt.$fetch.mock.calls.map(([, options]) => options.query)).toEqual([
        { ordering: '-created_at', page: 1, page_size: 500 },
        { ordering: '-created_at', page: 2, page_size: 500 },
        { ordering: '-created_at', page: 3, page_size: 500 },
      ])
    })

    it('stops at the limit and reports how many rows exist', async () => {
      servePages(12345)

      const { items, total } = await useApi().getAll('/events/events/')

      expect(items).toHaveLength(2000)
      expect(total).toBe(12345)
      expect(nuxt.$fetch).toHaveBeenCalledTimes(4)
    })

    it('honours a smaller limit', async () => {
      servePages(40)

      const { items, total } = await useApi().getAll('/events/events/', {}, 10)

      expect(items).toHaveLength(10)
      expect(total).toBe(40)
      expect(nuxt.$fetch.mock.calls[0]![1].query).toEqual({ page: 1, page_size: 10 })
    })

    it('keeps a row once when an insert on the server shifts the pages', async () => {
      // Newest first: a new row (id 601) arrives after page 1 was read, so
      // page 2 starts with the row that ended page 1.
      const page1 = Array.from({ length: 500 }, (_, index) => ({ id: 600 - index }))
      const page2 = Array.from({ length: 101 }, (_, index) => ({ id: 101 - index }))
      nuxt.$fetch
        .mockResolvedValueOnce({ count: 600, next: 'next-page-url', results: page1 })
        .mockResolvedValueOnce({ count: 601, next: null, results: page2 })

      const { items, total } = await useApi().getAll<{ id: number }>('/events/events/')

      expect(items).toHaveLength(600)
      expect(new Set(items.map(item => item.id)).size).toBe(600)
      expect(items[499]).toEqual({ id: 101 })
      expect(items[500]).toEqual({ id: 100 })
      expect(total).toBe(601)
    })

    it('keeps rows without an id as they come', async () => {
      nuxt.$fetch.mockResolvedValue({
        count: 2,
        next: null,
        results: [{ name: 'a' }, { name: 'a' }],
      })

      const { items } = await useApi().getAll('/devices/sites/')

      expect(items).toEqual([{ name: 'a' }, { name: 'a' }])
    })

    it('returns an empty list', async () => {
      servePages(0)

      await expect(useApi().getAll('/assets/assets/')).resolves.toEqual({ items: [], total: 0 })
    })

    it('copes with a response that is not paginated as expected', async () => {
      nuxt.$fetch.mockResolvedValue({})

      await expect(useApi().getAll('/assets/assets/')).resolves.toEqual({ items: [], total: 0 })
      expect(nuxt.$fetch).toHaveBeenCalledTimes(1)
    })

    it('passes a failing page on', async () => {
      const error = { statusCode: 500 }
      nuxt.$fetch.mockRejectedValue(error)

      await expect(useApi().getAll('/assets/assets/')).rejects.toBe(error)
    })
  })

  it('signs the user out and opens the login page when the API answers 401', async () => {
    const error = { statusCode: 401, data: { detail: 'Invalid token.' } }
    nuxt.$fetch.mockRejectedValue(error)
    const authStore = useAuthStore()
    authStore.token = 'expired'
    nuxt.cookie('auth_token').value = 'expired'

    await expect(useApi().get('/devices/devices/')).rejects.toBe(error)

    expect(authStore.token).toBeNull()
    expect(nuxt.cookie('auth_token').value).toBeNull()
    expect(nuxt.navigateTo).toHaveBeenCalledWith('/login')
  })

  it('also recognises a 401 reported as `status`', async () => {
    nuxt.$fetch.mockRejectedValue({ status: 401 })
    const authStore = useAuthStore()
    authStore.token = 'expired'

    await expect(useApi().get('/devices/devices/')).rejects.toEqual({ status: 401 })

    expect(authStore.token).toBeNull()
    expect(nuxt.navigateTo).toHaveBeenCalledWith('/login')
  })

  it.each([403, 404, 500])('passes a %d on and keeps the user signed in', async statusCode => {
    const error = { statusCode, data: { detail: 'Nope.' } }
    nuxt.$fetch.mockRejectedValue(error)
    const authStore = useAuthStore()
    authStore.token = 'abc123'

    await expect(useApi().get('/devices/devices/')).rejects.toBe(error)

    expect(authStore.token).toBe('abc123')
    expect(nuxt.navigateTo).not.toHaveBeenCalled()
  })
})
