import { useAuthStore } from '../stores/auth'

export default defineNuxtPlugin(async () => {
  const authStore = useAuthStore()

  // Restore token from cookie
  authStore.initFromCookie()

  // If we have a token, fetch the user profile
  if (authStore.token && !authStore.user) {
    await authStore.fetchUser()
  }
})
