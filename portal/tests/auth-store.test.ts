import { describe, expect, it } from 'vitest'
import { useAuthStore } from '../app/stores/auth'
import { API_BASE_URL, nuxt } from './setup'

const user = {
  id: 1,
  username: 'jdoe',
  email: 'jdoe@example.test',
  first_name: 'Jane',
  last_name: 'Doe',
}

describe('auth store', () => {
  describe('login', () => {
    it('stores token and user and keeps the token in a cookie', async () => {
      nuxt.$fetch.mockResolvedValue({ token: 'abc123', user })
      const store = useAuthStore()

      await store.login('jdoe@example.test', 'secret')

      expect(nuxt.$fetch).toHaveBeenCalledWith('/auth/login/', {
        baseURL: API_BASE_URL,
        method: 'POST',
        body: { email: 'jdoe@example.test', password: 'secret' },
      })
      expect(store.token).toBe('abc123')
      expect(store.user).toEqual(user)
      expect(store.isAuthenticated).toBe(true)
      expect(nuxt.cookie('auth_token').value).toBe('abc123')
    })

    it('writes the cookie as a week-long, same-site cookie that is secure over HTTPS', async () => {
      nuxt.$fetch.mockResolvedValue({ token: 'abc123', user })

      await useAuthStore().login('jdoe@example.test', 'secret')

      expect(nuxt.cookieOptions.get('auth_token')).toEqual({
        maxAge: 60 * 60 * 24 * 7,
        secure: true,
        sameSite: 'strict',
      })
    })

    it('stays signed out when the server rejects the credentials', async () => {
      const error = { statusCode: 400, data: { detail: 'Invalid credentials.' } }
      nuxt.$fetch.mockRejectedValue(error)
      const store = useAuthStore()

      await expect(store.login('jdoe@example.test', 'wrong')).rejects.toBe(error)

      expect(store.isAuthenticated).toBe(false)
      expect(store.user).toBeNull()
      expect(nuxt.cookie('auth_token').value).toBeNull()
    })
  })

  describe('logout', () => {
    it('revokes the token on the server and clears the local session', async () => {
      nuxt.$fetch.mockResolvedValue(undefined)
      const store = useAuthStore()
      store.token = 'abc123'
      store.user = user
      nuxt.cookie('auth_token').value = 'abc123'

      await store.logout()

      expect(nuxt.$fetch).toHaveBeenCalledWith('/auth/logout/', {
        baseURL: API_BASE_URL,
        method: 'POST',
        headers: { Authorization: 'Token abc123' },
      })
      expect(store.token).toBeNull()
      expect(store.user).toBeNull()
      expect(nuxt.cookie('auth_token').value).toBeNull()
    })

    it('clears the local session even when the server call fails', async () => {
      nuxt.$fetch.mockRejectedValue(new Error('Network down'))
      const store = useAuthStore()
      store.token = 'abc123'
      nuxt.cookie('auth_token').value = 'abc123'

      await store.logout()

      expect(store.token).toBeNull()
      expect(nuxt.cookie('auth_token').value).toBeNull()
    })

    it('does not call the server without a token', async () => {
      await useAuthStore().logout()

      expect(nuxt.$fetch).not.toHaveBeenCalled()
    })
  })

  describe('fetchUser', () => {
    it('loads the profile of the signed-in user', async () => {
      nuxt.$fetch.mockResolvedValue(user)
      const store = useAuthStore()
      store.token = 'abc123'

      await store.fetchUser()

      expect(nuxt.$fetch).toHaveBeenCalledWith('/auth/me/', {
        baseURL: API_BASE_URL,
        headers: { Authorization: 'Token abc123' },
      })
      expect(store.user).toEqual(user)
    })

    it('signs out when the stored token is no longer accepted', async () => {
      nuxt.$fetch.mockRejectedValue({ statusCode: 401 })
      const store = useAuthStore()
      store.token = 'expired'
      nuxt.cookie('auth_token').value = 'expired'

      await store.fetchUser()

      expect(store.token).toBeNull()
      expect(store.user).toBeNull()
      expect(nuxt.cookie('auth_token').value).toBeNull()
    })

    it('does nothing without a token', async () => {
      await useAuthStore().fetchUser()

      expect(nuxt.$fetch).not.toHaveBeenCalled()
    })
  })

  describe('initFromCookie', () => {
    it('restores the token from the cookie', () => {
      nuxt.cookie('auth_token').value = 'abc123'
      const store = useAuthStore()

      store.initFromCookie()

      expect(store.token).toBe('abc123')
      expect(store.isAuthenticated).toBe(true)
    })

    it('stays signed out without a cookie', () => {
      const store = useAuthStore()

      store.initFromCookie()

      expect(store.isAuthenticated).toBe(false)
    })
  })

  describe('displayName and initials', () => {
    it('are empty while no user is loaded', () => {
      const store = useAuthStore()

      expect(store.displayName).toBe('')
      expect(store.initials).toBe('')
    })

    it('use first and last name when both are set', () => {
      const store = useAuthStore()
      store.user = user

      expect(store.displayName).toBe('Jane Doe')
      expect(store.initials).toBe('JD')
    })

    it('use the first name alone when there is no last name', () => {
      const store = useAuthStore()
      store.user = { ...user, last_name: '' }

      expect(store.displayName).toBe('Jane')
      expect(store.initials).toBe('JD')
    })

    it('fall back to the username', () => {
      const store = useAuthStore()
      store.user = { ...user, first_name: '', last_name: '' }

      expect(store.displayName).toBe('jdoe')
      expect(store.initials).toBe('JD')
    })
  })
})
