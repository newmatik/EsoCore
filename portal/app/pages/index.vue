<template>
  <div class="min-h-screen bg-(--ui-bg)">
    <!-- Hero Section -->
    <div class="bg-(--ui-bg-elevated) shadow">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-24">
        <div class="text-center">
          <h1 class="text-4xl font-bold text-(--ui-text-highlighted) sm:text-6xl">
            {{ fullName }}
          </h1>
          <p class="mt-3 text-xl text-(--ui-text-muted) sm:mt-5 sm:text-2xl">
            {{ portalTitle }}
          </p>
          <div class="mt-8">
            <UButton to="/dashboard" size="xl"> Go to Dashboard </UButton>
          </div>
        </div>
      </div>
    </div>

    <!-- Features Section -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-16">
      <div class="text-center mb-12">
        <h2 class="text-3xl font-bold text-(--ui-text-highlighted)">
          Powerful IoT Monitoring Features
        </h2>
        <p class="mt-4 text-lg text-(--ui-text-muted)">
          Monitor, manage, and maintain your IoT devices with real-time insights
        </p>
      </div>

      <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8">
        <UCard v-for="feature in features" :key="feature.title">
          <div class="flex items-center gap-3 mb-4">
            <div
              class="w-10 h-10 rounded-lg flex items-center justify-center"
              :class="feature.iconBg"
            >
              <UIcon :name="feature.icon" class="w-5 h-5" :class="feature.iconColor" />
            </div>
            <h3 class="text-lg font-semibold text-(--ui-text-highlighted)">
              {{ feature.title }}
            </h3>
          </div>
          <p class="text-(--ui-text-muted)">
            {{ feature.description }}
          </p>
        </UCard>
      </div>
    </div>

    <!-- Footer -->
    <footer class="bg-(--ui-bg-elevated) border-t border-(--ui-border)">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div class="text-center text-(--ui-text-muted)">
          <p>&copy; 2025 EsoCore. Built with Nuxt.js &amp; Django REST Framework.</p>
        </div>
      </div>
    </footer>
  </div>
</template>

<script setup lang="ts">
import { useBranding } from '../composables/useBranding'

definePageMeta({
  layout: 'default',
})

useSeoMeta({
  title: 'EsoCore Portal',
  description: 'Monitor and manage your IoT devices with a powerful monitoring platform.',
})

useHead({
  meta: [
    {
      name: 'keywords',
      content: 'IoT, monitoring, devices, sensors, telemetry, alerts',
    },
  ],
})

const { fullName, portalTitle } = useBranding()

const features = [
  {
    icon: 'i-heroicons-chart-bar',
    iconBg: 'bg-blue-100 dark:bg-blue-900/40',
    iconColor: 'text-blue-600 dark:text-blue-400',
    title: 'Real-time Monitoring',
    description:
      'Monitor device status, telemetry data, and system health in real-time with live dashboards and alerts.',
  },
  {
    icon: 'i-heroicons-cog-6-tooth',
    iconBg: 'bg-green-100 dark:bg-green-900/40',
    iconColor: 'text-green-600 dark:text-green-400',
    title: 'Device Management',
    description:
      'Easily provision, configure, and manage your IoT devices with centralized device management tools.',
  },
  {
    icon: 'i-heroicons-bell-alert',
    iconBg: 'bg-yellow-100 dark:bg-yellow-900/40',
    iconColor: 'text-yellow-600 dark:text-yellow-400',
    title: 'Alert System',
    description:
      'Get notified of issues before they become problems with intelligent alerting and notification systems.',
  },
]
</script>
