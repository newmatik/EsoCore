<template>
  <div class="min-h-screen bg-(--ui-bg)">
    <!-- Header -->
    <div class="bg-(--ui-bg-elevated) shadow">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        <div class="flex items-center justify-between">
          <div>
            <h1 class="text-2xl font-bold text-(--ui-text-highlighted)">Dashboard</h1>
            <p class="text-(--ui-text-muted)">Monitor your IoT devices and systems</p>
          </div>
          <UButton leading-icon="i-heroicons-arrow-path" :loading="refreshing" @click="refreshData">
            Refresh
          </UButton>
        </div>
      </div>
    </div>

    <!-- Main Content -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
      <!-- Error -->
      <UCard v-if="errorMsg" class="mb-6">
        <div class="flex items-center gap-3 text-red-600 dark:text-red-400">
          <UIcon name="i-heroicons-exclamation-triangle" class="w-5 h-5 flex-shrink-0" />
          <div>
            <p class="text-sm font-medium">Failed to load dashboard data</p>
            <p class="text-xs mt-0.5">{{ errorMsg }}</p>
          </div>
        </div>
      </UCard>

      <!-- Stats Cards -->
      <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
        <UCard v-for="stat in statsCards" :key="stat.label">
          <div class="flex items-center">
            <div
              class="w-10 h-10 rounded-lg flex items-center justify-center flex-shrink-0"
              :class="stat.iconBg"
            >
              <UIcon :name="stat.icon" class="w-5 h-5" :class="stat.iconColor" />
            </div>
            <div class="ml-4">
              <p class="text-sm font-medium text-(--ui-text-muted)">{{ stat.label }}</p>
              <p class="text-2xl font-semibold text-(--ui-text-highlighted)">{{ stat.value }}</p>
            </div>
          </div>
        </UCard>
      </div>

      <!-- Charts and Recent Activity -->
      <div class="grid grid-cols-1 lg:grid-cols-2 gap-8">
        <!-- Device Status Chart -->
        <UCard>
          <template #header>
            <h3 class="text-lg font-medium text-(--ui-text-highlighted)">
              Device Status Overview
            </h3>
          </template>
          <div class="flex items-center justify-center" style="height: 260px;">
            <Doughnut v-if="chartReady" :data="chartData" :options="chartOptions" />
            <p v-else class="text-(--ui-text-muted)">Loading chart...</p>
          </div>
        </UCard>

        <!-- Recent Alerts -->
        <UCard>
          <template #header>
            <div class="flex items-center justify-between">
              <h3 class="text-lg font-medium text-(--ui-text-highlighted)">Recent Alerts</h3>
              <UButton variant="link" to="/alerts" size="sm">
                View all
              </UButton>
            </div>
          </template>
          <div class="space-y-3">
            <div
              v-for="event in recentEvents"
              :key="event.id"
              class="flex items-center justify-between p-3 bg-(--ui-bg-accented) rounded-lg"
            >
              <div class="flex items-center gap-3 min-w-0">
                <div
                  class="w-2.5 h-2.5 rounded-full flex-shrink-0"
                  :class="severityDotClass(event.severity)"
                />
                <div class="min-w-0">
                  <p class="text-sm font-medium text-(--ui-text-highlighted) truncate">
                    {{ event.description }}
                  </p>
                  <p class="text-xs text-(--ui-text-muted)">
                    {{ event.event_type }} &middot; {{ formatTimeAgo(event.created_at) }}
                  </p>
                </div>
              </div>
              <UButton
                v-if="event.status === 'active'"
                size="xs"
                variant="outline"
                class="ml-2 flex-shrink-0"
                @click="acknowledgeEvent(event.id)"
              >
                Ack
              </UButton>
              <UBadge v-else :color="statusBadgeColor(event.status)" variant="subtle" size="sm" class="ml-2 flex-shrink-0">
                {{ event.status }}
              </UBadge>
            </div>
            <div v-if="recentEvents.length === 0" class="text-center py-8">
              <p class="text-(--ui-text-muted)">No recent alerts</p>
            </div>
          </div>
        </UCard>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { Doughnut } from 'vue-chartjs'
import { Chart as ChartJS, ArcElement, Tooltip, Legend } from 'chart.js'
import { useApi } from '../composables/useApi'

ChartJS.register(ArcElement, Tooltip, Legend)

definePageMeta({
  layout: 'authenticated',
  middleware: ['auth'],
})

interface DashboardSummary {
  total_devices: number
  online_devices: number
  offline_devices: number
  maintenance_devices: number
  active_alerts: number
  total_assets: number
  total_sites: number
  device_status_breakdown: Record<string, number>
  alert_severity_breakdown: Record<string, number>
}

interface SystemEvent {
  id: string
  device: string
  event_type: string
  severity: string
  description: string
  status: string
  created_at: string
}

interface PaginatedResponse<T> {
  count: number
  results: T[]
}

const api = useApi()
const toast = useToast()
const refreshing = ref(false)
const errorMsg = ref('')

const summary = ref<DashboardSummary>({
  total_devices: 0,
  online_devices: 0,
  offline_devices: 0,
  maintenance_devices: 0,
  active_alerts: 0,
  total_assets: 0,
  total_sites: 0,
  device_status_breakdown: {},
  alert_severity_breakdown: {},
})

const recentEvents = ref<SystemEvent[]>([])
const chartReady = ref(false)

const statsCards = computed(() => [
  {
    icon: 'i-heroicons-cpu-chip',
    iconBg: 'bg-blue-100 dark:bg-blue-900/40',
    iconColor: 'text-blue-600 dark:text-blue-400',
    label: 'Total Devices',
    value: summary.value.total_devices,
  },
  {
    icon: 'i-heroicons-signal',
    iconBg: 'bg-green-100 dark:bg-green-900/40',
    iconColor: 'text-green-600 dark:text-green-400',
    label: 'Online',
    value: summary.value.online_devices,
  },
  {
    icon: 'i-heroicons-bell-alert',
    iconBg: 'bg-yellow-100 dark:bg-yellow-900/40',
    iconColor: 'text-yellow-600 dark:text-yellow-400',
    label: 'Active Alerts',
    value: summary.value.active_alerts,
  },
  {
    icon: 'i-heroicons-wrench-screwdriver',
    iconBg: 'bg-purple-100 dark:bg-purple-900/40',
    iconColor: 'text-purple-600 dark:text-purple-400',
    label: 'Assets',
    value: summary.value.total_assets,
  },
])

const chartData = computed(() => {
  const breakdown = summary.value.device_status_breakdown
  return {
    labels: ['Active', 'Inactive', 'Maintenance', 'Offline'],
    datasets: [
      {
        data: [
          breakdown.active || 0,
          breakdown.inactive || 0,
          breakdown.maintenance || 0,
          breakdown.offline || 0,
        ],
        backgroundColor: ['#10b981', '#6b7280', '#f59e0b', '#ef4444'],
        borderColor: ['#059669', '#4b5563', '#d97706', '#dc2626'],
        borderWidth: 2,
      },
    ],
  }
})

const chartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  plugins: {
    legend: {
      position: 'bottom' as const,
      labels: {
        color: '#9ca3af',
        padding: 16,
        usePointStyle: true,
      },
    },
  },
}

function severityDotClass(severity: string) {
  const map: Record<string, string> = {
    low: 'bg-green-500',
    medium: 'bg-yellow-500',
    high: 'bg-orange-500',
    critical: 'bg-red-500',
  }
  return map[severity] || 'bg-gray-400'
}

function statusBadgeColor(status: string): string {
  const map: Record<string, string> = {
    acknowledged: 'info',
    resolved: 'success',
    suppressed: 'neutral',
  }
  return map[status] || 'neutral'
}

function formatTimeAgo(dateStr: string) {
  const diff = Date.now() - new Date(dateStr).getTime()
  const minutes = Math.floor(diff / 60000)
  if (minutes < 1) return 'just now'
  if (minutes < 60) return `${minutes}m ago`
  const hours = Math.floor(minutes / 60)
  if (hours < 24) return `${hours}h ago`
  const days = Math.floor(hours / 24)
  return `${days}d ago`
}

async function fetchDashboardData() {
  errorMsg.value = ''
  try {
    const [summaryData, eventsData] = await Promise.all([
      api.get<DashboardSummary>('/dashboard/summary/'),
      api.get<PaginatedResponse<SystemEvent>>('/events/events/', {
        ordering: '-created_at',
        page_size: 8,
      }),
    ])
    summary.value = summaryData
    recentEvents.value = eventsData.results || []
    chartReady.value = true
  }
  catch (error: unknown) {
    const err = error as { data?: { detail?: string }; message?: string }
    errorMsg.value = err?.data?.detail || err?.message || 'Failed to load dashboard data'
    console.error('Failed to fetch dashboard data:', error)
  }
}

async function refreshData() {
  refreshing.value = true
  try {
    await fetchDashboardData()
  }
  finally {
    refreshing.value = false
  }
}

async function acknowledgeEvent(eventId: string) {
  try {
    await api.post(`/events/events/${eventId}/acknowledge/`)
    const event = recentEvents.value.find(e => e.id === eventId)
    if (event) event.status = 'acknowledged'
  }
  catch (error: unknown) {
    const err = error as { data?: { detail?: string }; message?: string }
    toast.add({
      title: 'Failed to acknowledge event',
      description: err?.data?.detail || err?.message || 'Unknown error',
      color: 'error',
    })
  }
}

onMounted(() => {
  fetchDashboardData()
})
</script>
