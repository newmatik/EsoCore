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

  return { request, get, post, put, patch, del }
}
