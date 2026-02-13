<template>
  <div class="min-h-screen bg-(--ui-bg)">
    <header class="bg-(--ui-bg-elevated) border-b border-(--ui-border)">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4 flex items-center justify-between">
        <NuxtLink to="/" class="flex items-center gap-2 hover:opacity-80 transition-opacity">
          <img src="/icon.svg" alt="Portal Icon" class="h-6 w-6" />
          <h1 class="text-lg font-semibold text-(--ui-text-highlighted)">{{ portalTitle }}</h1>
        </NuxtLink>
        <nav class="text-sm flex items-center gap-2">
          <NuxtLink
            to="/"
            class="px-3 py-1.5 rounded-md text-(--ui-text-muted) hover:bg-(--ui-bg-accented) hover:text-(--ui-text) transition-colors"
          >
            Home
          </NuxtLink>
          <NuxtLink
            to="/dashboard"
            class="px-3 py-1.5 rounded-md text-(--ui-text-muted) hover:bg-(--ui-bg-accented) hover:text-(--ui-text) transition-colors"
          >
            Dashboard
          </NuxtLink>
          <ClientOnly>
            <ThemeToggle />
          </ClientOnly>
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
