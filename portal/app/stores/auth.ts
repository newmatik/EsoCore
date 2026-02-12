import { defineStore } from 'pinia'

interface UserInfo {
  id: number
  username: string
  email: string
  first_name: string
  last_name: string
  is_staff?: boolean
}

interface AuthState {
  token: string | null
  user: UserInfo | null
}

export const useAuthStore = defineStore('auth', {
  state: (): AuthState => ({
    token: null,
    user: null,
  }),

  getters: {
    isAuthenticated: (state): boolean => !!state.token,
    displayName: (state): string => {
      if (!state.user) return ''
      if (state.user.first_name) {
        return `${state.user.first_name} ${state.user.last_name}`.trim()
      }
      return state.user.username
    },
    initials: (state): string => {
      if (!state.user) return ''
      if (state.user.first_name && state.user.last_name) {
        return `${state.user.first_name[0]}${state.user.last_name[0]}`.toUpperCase()
      }
      return state.user.username.substring(0, 2).toUpperCase()
    },
  },

  actions: {
    async login(email: string, password: string) {
      const config = useRuntimeConfig()
      const baseURL = config.public.apiBaseUrl as string

      const data = await $fetch<{ token: string; user: UserInfo }>(
        '/auth/login/',
        {
          baseURL,
          method: 'POST',
          body: { email, password },
        },
      )

      this.token = data.token
      this.user = data.user
      this._persistToken()
    },

    async logout() {
      try {
        if (this.token) {
          const config = useRuntimeConfig()
          const baseURL = config.public.apiBaseUrl as string
          await $fetch('/auth/logout/', {
            baseURL,
            method: 'POST',
            headers: { Authorization: `Token ${this.token}` },
          })
        }
      }
      catch {
        // Ignore errors during logout
      }
      finally {
        this.clearAuth()
      }
    },

    async fetchUser() {
      if (!this.token) return
      try {
        const config = useRuntimeConfig()
        const baseURL = config.public.apiBaseUrl as string
        this.user = await $fetch<UserInfo>('/auth/me/', {
          baseURL,
          headers: { Authorization: `Token ${this.token}` },
        })
      }
      catch {
        this.clearAuth()
      }
    },

    clearAuth() {
      this.token = null
      this.user = null
      if (import.meta.client) {
        useCookie('auth_token').value = null
      }
    },

    /** Restore token from cookie on app init (client-side). */
    initFromCookie() {
      if (import.meta.client) {
        const cookie = useCookie('auth_token')
        if (cookie.value) {
          this.token = cookie.value
        }
      }
    },

    _persistToken() {
      if (import.meta.client) {
        const cookie = useCookie('auth_token', {
          maxAge: 60 * 60 * 24 * 7,
          secure: location.protocol === 'https:',
          sameSite: 'strict',
        })
        cookie.value = this.token
      }
    },
  },
})
