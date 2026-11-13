/**
 * API client composable for communicating with the Django REST server.
 *
 * Wraps Nuxt's $fetch with:
 * - Automatic base URL from runtime config
 * - Auth token injection from the auth store
 * - 401 handling (redirect to login)
 */
import type { FetchOptions } from 'ofetch'
import { useAuthStore } from '../stores/auth'

type HttpMethod = 'GET' | 'POST' | 'PUT' | 'PATCH' | 'DELETE' | 'HEAD' | 'OPTIONS'
// eslint-disable-next-line @typescript-eslint/no-explicit-any
type RequestBody = Record<string, any> | BodyInit | null | undefined

/** One page of a list endpoint. */
export interface PaginatedResponse<T> {
  count: number
  next?: string | null
  previous?: string | null
  results: T[]
}

/** Largest page the API serves (`max_page_size` of the server's pagination). */
const MAX_PAGE_SIZE = 500
/** Most rows a list page loads; beyond this it says that it shows only a part. */
export const LIST_LIMIT = 2000

export function useApi() {
  const config = useRuntimeConfig()
  const baseURL = config.public.apiBaseUrl as string

  async function request<T = unknown>(path: string, options: FetchOptions = {}): Promise<T> {
    const authStore = useAuthStore()

    const headers: Record<string, string> = {
      ...((options.headers as Record<string, string>) || {}),
    }

    if (authStore.token) {
      headers['Authorization'] = `Token ${authStore.token}`
    }

    // Only set Content-Type for requests with body
    if (options.method && !['GET', 'HEAD'].includes(options.method as string)) {
      if (!headers['Content-Type']) {
        headers['Content-Type'] = 'application/json'
      }
    }

    try {
      return await $fetch<T>(path, {
        baseURL,
        ...options,
        method: options.method as HttpMethod,
        headers,
      })
    } catch (error: unknown) {
      const fetchError = error as { statusCode?: number; status?: number; data?: unknown }
      if (fetchError.statusCode === 401 || fetchError.status === 401) {
        authStore.clearAuth()
        await navigateTo('/login')
      }
      throw error
    }
  }

  function get<T = unknown>(path: string, query?: Record<string, unknown>) {
    return request<T>(path, { method: 'GET', query })
  }

  function post<T = unknown>(path: string, body?: RequestBody) {
    return request<T>(path, { method: 'POST', body })
  }

  function put<T = unknown>(path: string, body?: RequestBody) {
    return request<T>(path, { method: 'PUT', body })
  }

  function patch<T = unknown>(path: string, body?: RequestBody) {
    return request<T>(path, { method: 'PATCH', body })
  }

  function del<T = unknown>(path: string) {
    return request<T>(path, { method: 'DELETE' })
  }

  /**
   * Reads a paginated list completely by following its pages, up to `limit`
   * rows. `total` is the number of rows the server has; it is larger than
   * `items.length` when the limit cut the list short.
   */
  async function getAll<T = unknown>(
    path: string,
    query: Record<string, unknown> = {},
    limit = LIST_LIMIT
  ): Promise<{ items: T[]; total: number }> {
    const items: T[] = []
    let total = 0

    for (let page = 1; items.length < limit; page++) {
      const data = await get<PaginatedResponse<T>>(path, {
        ...query,
        page,
        page_size: Math.min(MAX_PAGE_SIZE, limit),
      })
      const results = data.results || []
      items.push(...results)
      total = data.count ?? items.length
      if (!data.next || results.length === 0) break
    }

    const shown = items.slice(0, limit)
    return { items: shown, total: Math.max(total, shown.length) }
  }

  return { request, get, getAll, post, put, patch, del }
}
