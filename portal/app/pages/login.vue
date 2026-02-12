<template>
  <div class="min-h-screen flex items-center justify-center bg-(--ui-bg) px-4">
    <div class="w-full max-w-sm">
      <!-- Logo / Branding -->
      <div class="text-center mb-8">
        <img src="/icon.svg" alt="EsoCore" class="h-12 w-12 mx-auto mb-4">
        <h1 class="text-2xl font-bold text-(--ui-text-highlighted)">
          {{ portalTitle }}
        </h1>
        <p class="mt-2 text-(--ui-text-muted)">
          Sign in to your account
        </p>
      </div>

      <!-- Login Form -->
      <UCard>
        <form @submit.prevent="handleLogin" class="space-y-5">
          <!-- Error message -->
          <div
            v-if="errorMessage"
            class="p-3 rounded-lg bg-red-50 dark:bg-red-900/20 text-red-700 dark:text-red-400 text-sm"
          >
            {{ errorMessage }}
          </div>

          <!-- Email -->
          <div>
            <label for="email" class="block text-sm font-medium text-(--ui-text) mb-1.5">
              Email
            </label>
            <UInput
              id="email"
              v-model="email"
              type="email"
              required
              autocomplete="email"
              placeholder="admin@esocore.local"
              size="lg"
              class="w-full"
            />
          </div>

          <!-- Password -->
          <div>
            <label for="password" class="block text-sm font-medium text-(--ui-text) mb-1.5">
              Password
            </label>
            <UInput
              id="password"
              v-model="password"
              type="password"
              required
              autocomplete="current-password"
              placeholder="Enter your password"
              size="lg"
              class="w-full"
            />
          </div>

          <!-- Submit -->
          <UButton
            type="submit"
            :loading="loading"
            block
            size="lg"
          >
            Sign in
          </UButton>
        </form>
      </UCard>

      <!-- Footer hint -->
      <p class="mt-6 text-center text-xs text-(--ui-text-dimmed)">
        Demo credentials: admin@esocore.local / admin
      </p>
    </div>
  </div>
</template>

<script setup lang="ts">
import { useAuthStore } from '../stores/auth'
import { useBranding } from '../composables/useBranding'

definePageMeta({
  layout: 'default',
})

useSeoMeta({
  title: 'Login - EsoCore Portal',
})

const { portalTitle } = useBranding()
const authStore = useAuthStore()

const email = ref('')
const password = ref('')
const loading = ref(false)
const errorMessage = ref('')

if (import.meta.client && authStore.isAuthenticated) {
  navigateTo('/dashboard')
}

async function handleLogin() {
  loading.value = true
  errorMessage.value = ''

  try {
    await authStore.login(email.value, password.value)
    await navigateTo('/dashboard')
  }
  catch (error: unknown) {
    const fetchError = error as { data?: { detail?: string } }
    errorMessage.value = fetchError?.data?.detail || 'Login failed. Please check your credentials.'
  }
  finally {
    loading.value = false
  }
}
</script>
