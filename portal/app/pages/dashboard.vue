<template>
  <div class="min-h-screen bg-gray-50 dark:bg-gray-900">
    <!-- Header -->
    <div class="bg-white dark:bg-gray-800 shadow">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        <div class="flex items-center justify-between">
          <div>
            <h1 class="text-2xl font-bold text-gray-900 dark:text-white">Dashboard</h1>
            <p class="text-gray-500 dark:text-gray-400">Monitor your IoT devices and systems</p>
          </div>
          <div class="flex items-center space-x-4">
            <UButton icon="i-heroicons-arrow-path" :loading="refreshing" @click="refreshData">
              Refresh
            </UButton>
            <UButton icon="i-heroicons-cog-6-tooth" variant="outline" to="/settings">
              Settings
            </UButton>
          </div>
        </div>
      </div>
    </div>

    <!-- Main Content -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
      <!-- Stats Cards -->
      <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
        <div class="dashboard-card">
          <div class="flex items-center">
            <div class="flex-shrink-0">
              <div
                class="w-8 h-8 bg-blue-100 dark:bg-blue-900 rounded-full flex items-center justify-center"
              >
                <svg
                  class="w-5 h-5 text-blue-600 dark:text-blue-400"
                  fill="none"
                  stroke="currentColor"
                  viewBox="0 0 24 24"
                >
                  <path
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    stroke-width="2"
                    d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z"
                  />
                </svg>
              </div>
            </div>
            <div class="ml-4">
              <p class="text-sm font-medium text-gray-500 dark:text-gray-400">Total Devices</p>
              <p class="text-2xl font-semibold text-gray-900 dark:text-white">
                {{ stats.totalDevices }}
              </p>
            </div>
          </div>
        </div>

        <div class="dashboard-card">
          <div class="flex items-center">
            <div class="flex-shrink-0">
              <div
                class="w-8 h-8 bg-green-100 dark:bg-green-900 rounded-full flex items-center justify-center"
              >
                <svg
                  class="w-5 h-5 text-green-600 dark:text-green-400"
                  fill="none"
                  stroke="currentColor"
                  viewBox="0 0 24 24"
                >
                  <path
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    stroke-width="2"
                    d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"
                  />
                </svg>
              </div>
            </div>
            <div class="ml-4">
              <p class="text-sm font-medium text-gray-500 dark:text-gray-400">Online Devices</p>
              <p class="text-2xl font-semibold text-gray-900 dark:text-white">
                {{ stats.onlineDevices }}
              </p>
            </div>
          </div>
        </div>

        <div class="dashboard-card">
          <div class="flex items-center">
            <div class="flex-shrink-0">
              <div
                class="w-8 h-8 bg-yellow-100 dark:bg-yellow-900 rounded-full flex items-center justify-center"
              >
                <svg
                  class="w-5 h-5 text-yellow-600 dark:text-yellow-400"
                  fill="none"
                  stroke="currentColor"
                  viewBox="0 0 24 24"
                >
                  <path
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    stroke-width="2"
                    d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-2.5L13.732 4c-.77-.833-1.964-.833-2.732 0L3.732 16.5c-.77.833.192 2.5 1.732 2.5z"
                  />
                </svg>
              </div>
            </div>
            <div class="ml-4">
              <p class="text-sm font-medium text-gray-500 dark:text-gray-400">Active Alerts</p>
              <p class="text-2xl font-semibold text-gray-900 dark:text-white">
                {{ stats.activeAlerts }}
              </p>
            </div>
          </div>
        </div>

        <div class="dashboard-card">
          <div class="flex items-center">
            <div class="flex-shrink-0">
              <div
                class="w-8 h-8 bg-purple-100 dark:bg-purple-900 rounded-full flex items-center justify-center"
              >
                <svg
                  class="w-5 h-5 text-purple-600 dark:text-purple-400"
                  fill="none"
                  stroke="currentColor"
                  viewBox="0 0 24 24"
                >
                  <path
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    stroke-width="2"
                    d="M19 11H5m14 0a2 2 0 012 2v6a2 2 0 01-2 2H5a2 2 0 01-2-2v-6a2 2 0 012-2m14 0V9a2 2 0 00-2-2M5 11V9a2 2 0 012-2m0 0V5a2 2 0 012-2h6a2 2 0 012 2v2M7 7h10"
                  />
                </svg>
              </div>
            </div>
            <div class="ml-4">
              <p class="text-sm font-medium text-gray-500 dark:text-gray-400">Assets Monitored</p>
              <p class="text-2xl font-semibold text-gray-900 dark:text-white">
                {{ stats.totalAssets }}
              </p>
            </div>
          </div>
        </div>
      </div>

      <!-- Charts and Recent Activity -->
      <div class="grid grid-cols-1 lg:grid-cols-2 gap-8">
        <!-- Device Status Chart -->
        <div class="dashboard-card">
          <h3 class="text-lg font-medium text-gray-900 dark:text-white mb-4">
            Device Status Overview
          </h3>
          <div class="chart-container">
            <canvas ref="deviceStatusChart" />
          </div>
        </div>

        <!-- Recent Alerts -->
        <div class="dashboard-card">
          <div class="flex items-center justify-between mb-4">
            <h3 class="text-lg font-medium text-gray-900 dark:text-white">Recent Alerts</h3>
            <UButton variant="link" to="/alerts" class="text-sm"> View all </UButton>
          </div>
          <div class="space-y-3">
            <div
              v-for="alert in recentAlerts"
              :key="alert.id"
              class="flex items-center justify-between p-3 bg-gray-50 dark:bg-gray-700 rounded-lg"
            >
              <div class="flex items-center space-x-3">
                <div
                  class="w-2 h-2 rounded-full"
                  :class="{
                    'bg-green-500': alert.severity === 'low',
                    'bg-yellow-500': alert.severity === 'medium',
                    'bg-orange-500': alert.severity === 'high',
                    'bg-red-500': alert.severity === 'critical',
                  }"
                />
                <div>
                  <p class="text-sm font-medium text-gray-900 dark:text-white">
                    {{ alert.description }}
                  </p>
                  <p class="text-xs text-gray-500 dark:text-gray-400">
                    {{ formatDate(alert.created_at) }}
                  </p>
                </div>
              </div>
              <UButton size="xs" variant="outline" @click="acknowledgeAlert(alert.id)">
                Acknowledge
              </UButton>
            </div>
            <div v-if="recentAlerts.length === 0" class="text-center py-8">
              <p class="text-gray-500 dark:text-gray-400">No recent alerts</p>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue'

// Page meta
import auth from '~/middleware/auth'

definePageMeta({
  layout: 'authenticated',
  middleware: [auth],
})

// Reactive data
const refreshing = ref(false)
const deviceStatusChart = ref<HTMLCanvasElement>()

// Mock data - replace with real API calls
const stats = ref({
  totalDevices: 0,
  onlineDevices: 0,
  activeAlerts: 0,
  totalAssets: 0,
})

const recentAlerts = ref([
  {
    id: 1,
    description: 'Device offline: Sensor-001',
    severity: 'high',
    created_at: new Date(Date.now() - 1000 * 60 * 30), // 30 minutes ago
  },
  {
    id: 2,
    description: 'High vibration detected on CNC-001',
    severity: 'medium',
    created_at: new Date(Date.now() - 1000 * 60 * 60 * 2), // 2 hours ago
  },
])

// Methods
const refreshData = async () => {
  refreshing.value = true
  try {
    // TODO: Implement data refresh from API
    await new Promise(resolve => setTimeout(resolve, 1000))
  } finally {
    refreshing.value = false
  }
}

const acknowledgeAlert = async (alertId: number) => {
  // TODO: Implement alert acknowledgment
  console.log('Acknowledging alert:', alertId)
}

const formatDate = (date: Date) => {
  return new Intl.RelativeTimeFormat('en', { numeric: 'auto' }).format(
    Math.round((date.getTime() - Date.now()) / (1000 * 60)),
    'minute'
  )
}

// Initialize chart
onMounted(() => {
  if (deviceStatusChart.value) {
    // TODO: Initialize Chart.js for device status visualization
  }
})

// Fetch initial data
onMounted(async () => {
  // TODO: Fetch dashboard data from API
  stats.value = {
    totalDevices: 24,
    onlineDevices: 22,
    activeAlerts: 3,
    totalAssets: 18,
  }
})
</script>
