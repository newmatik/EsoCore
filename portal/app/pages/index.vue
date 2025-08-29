<template>
  <div class="min-h-screen bg-gray-50 dark:bg-gray-900">
    <!-- Hero Section -->
    <div class="bg-white dark:bg-gray-800 shadow">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-24">
        <div class="text-center">
          <h1 class="text-4xl font-bold text-gray-900 dark:text-white sm:text-6xl">
            {{ fullName }}
          </h1>
          <p class="mt-3 text-xl text-gray-500 dark:text-gray-400 sm:mt-5 sm:text-2xl">
            {{ portalTitle }}
          </p>
          <div class="mt-8">
            <NuxtLink
              to="/dashboard"
              class="inline-flex items-center px-6 py-3 border border-transparent text-base font-medium rounded-md text-white bg-primary-500 hover:bg-primary-600 transition-colors"
            >
              Go to Dashboard
            </NuxtLink>
          </div>
        </div>
      </div>
    </div>

    <!-- Features Section -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-16">
      <div class="text-center mb-12">
        <h2 class="text-3xl font-bold text-gray-900 dark:text-white">
          Powerful IoT Monitoring Features
        </h2>
        <p class="mt-4 text-lg text-gray-500 dark:text-gray-400">
          Monitor, manage, and maintain your IoT devices with real-time insights
        </p>
      </div>

      <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8">
        <div class="dashboard-card">
          <div class="flex items-center mb-4">
            <div class="flex-shrink-0">
              <svg class="h-8 w-8 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z" />
              </svg>
            </div>
            <h3 class="ml-3 text-lg font-medium text-gray-900 dark:text-white">
              Real-time Monitoring
            </h3>
          </div>
          <p class="text-gray-500 dark:text-gray-400">
            Monitor device status, telemetry data, and system health in real-time with live dashboards and alerts.
          </p>
        </div>

        <div class="dashboard-card">
          <div class="flex items-center mb-4">
            <div class="flex-shrink-0">
              <svg class="h-8 w-8 text-green-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M10.325 4.317c.426-1.756 2.924-1.756 3.35 0a1.724 1.724 0 002.573 1.066c1.543-.94 3.31.826 2.37 2.37a1.724 1.724 0 001.065 2.572c1.756.426 1.756 2.924 0 3.35a1.724 1.724 0 00-1.066 2.573c.94 1.543-.826 3.31-2.37 2.37a1.724 1.724 0 00-2.572 1.065c-.426 1.756-2.924 1.756-3.35 0a1.724 1.724 0 00-2.573-1.066c-1.543.94-3.31-.826-2.37-2.37a1.724 1.724 0 00-1.065-2.572c-1.756-.426-1.756-2.924 0-3.35a1.724 1.724 0 001.066-2.573c-.94-1.543.826-3.31 2.37-2.37.996.608 2.296.07 2.572-1.065z" />
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" />
              </svg>
            </div>
            <h3 class="ml-3 text-lg font-medium text-gray-900 dark:text-white">
              Device Management
            </h3>
          </div>
          <p class="text-gray-500 dark:text-gray-400">
            Easily provision, configure, and manage your IoT devices with centralized device management tools.
          </p>
        </div>

        <div class="dashboard-card">
          <div class="flex items-center mb-4">
            <div class="flex-shrink-0">
              <svg class="h-8 w-8 text-yellow-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-2.5L13.732 4c-.77-.833-1.964-.833-2.732 0L3.732 16.5c-.77.833.192 2.5 1.732 2.5z" />
              </svg>
            </div>
            <h3 class="ml-3 text-lg font-medium text-gray-900 dark:text-white">
              Alert System
            </h3>
          </div>
          <p class="text-gray-500 dark:text-gray-400">
            Get notified of issues before they become problems with intelligent alerting and notification systems.
          </p>
        </div>
      </div>
    </div>

    <!-- Footer -->
    <footer class="bg-white dark:bg-gray-800 border-t border-gray-200 dark:border-gray-700">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div class="text-center text-gray-500 dark:text-gray-400">
          <p>&copy; 2025 EsoCore. Built with Nuxt.js & Django REST Framework.</p>
        </div>
      </div>
    </footer>
  </div>
</template>

<script setup lang="ts">
// Page meta
definePageMeta({
  layout: 'default'
})

// SEO meta
useSeoMeta({
  title: 'EsoCore Portal',
  description: 'Monitor and manage your IoT devices with a powerful monitoring platform.',
  keywords: 'IoT, monitoring, devices, sensors, telemetry, alerts'
})

import { useBranding } from '../composables/useBranding'
const { fullName, portalTitle } = useBranding()
</script>
