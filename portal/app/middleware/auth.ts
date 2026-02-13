import { useAuthStore } from '../stores/auth'

export default defineNuxtRouteMiddleware(to => {
  if (import.meta.server) return

  const authStore = useAuthStore()

  // Restore from cookie if not yet loaded
  if (!authStore.token) {
    authStore.initFromCookie()
  }

  if (!authStore.isAuthenticated && to.path !== '/login') {
    return navigateTo('/login')
  }
})
