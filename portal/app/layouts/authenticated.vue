<template>
  <div class="min-h-screen bg-gray-50 dark:bg-gray-900">
    <header class="bg-white dark:bg-gray-800 border-b border-gray-200 dark:border-gray-700">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4 flex items-center justify-between">
        <div class="flex items-center gap-2">
          <img src="/icon.svg" alt="Portal Icon" class="h-6 w-6" >
          <h1 class="text-lg font-semibold text-gray-900 dark:text-white">
            {{ portalTitle }}
          </h1>
        </div>
        <nav class="text-sm text-gray-600 dark:text-gray-300 flex items-center gap-4">
          <NuxtLink to="/dashboard" class="mr-4 hover:underline">Dashboard</NuxtLink>
          <NuxtLink to="/devices" class="mr-4 hover:underline">Devices</NuxtLink>
          <NuxtLink to="/alerts" class="mr-4 hover:underline">Alerts</NuxtLink>
          <NuxtLink to="/assets" class="hover:underline">Assets</NuxtLink>
          <ThemeToggle />
        </nav>
      </div>
    </header>
    <main>
      <slot />
    </main>
  </div>
</template>

<script setup lang="ts">
import { useBranding } from '../composables/useBranding'

const { portalTitle } = useBranding()
useHead({ title: portalTitle })
</script>
