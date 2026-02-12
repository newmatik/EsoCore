<template>
  <div class="min-h-screen bg-(--ui-bg)">
    <!-- Header -->
    <div class="bg-(--ui-bg-elevated) shadow">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        <div class="flex items-center justify-between">
          <div>
            <h1 class="text-2xl font-bold text-(--ui-text-highlighted)">Devices</h1>
            <p class="text-(--ui-text-muted)">Manage and monitor your IoT edge devices</p>
          </div>
          <UButton leading-icon="i-heroicons-arrow-path" :loading="loading" @click="fetchDevices">
            Refresh
          </UButton>
        </div>
      </div>
    </div>

    <!-- Filters -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 pt-6">
      <div class="flex flex-wrap gap-3">
        <USelect v-model="statusFilter" :items="statusOptions" placeholder="All statuses" class="w-40" />
        <USelect v-model="siteFilter" :items="siteOptions" placeholder="All sites" class="w-48" />
      </div>
    </div>

    <!-- Content -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
      <!-- Error -->
      <UCard v-if="errorMsg" class="mb-6">
        <div class="flex items-center gap-3 text-red-600 dark:text-red-400">
          <UIcon name="i-heroicons-exclamation-triangle" class="w-5 h-5 flex-shrink-0" />
          <div>
            <p class="text-sm font-medium">Failed to load devices</p>
            <p class="text-xs mt-0.5">{{ errorMsg }}</p>
          </div>
        </div>
      </UCard>

      <!-- Loading -->
      <div v-if="loading && devices.length === 0" class="text-center py-16">
        <UIcon name="i-heroicons-arrow-path" class="w-6 h-6 animate-spin text-(--ui-text-muted) mx-auto" />
        <p class="mt-3 text-(--ui-text-muted)">Loading devices...</p>
      </div>

      <!-- Device Table -->
      <UCard v-else>
        <div class="overflow-x-auto">
          <table class="min-w-full divide-y divide-(--ui-border)">
            <thead>
              <tr>
                <th class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider">Device</th>
                <th class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider">Site</th>
                <th class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider">Status</th>
                <th class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider">Firmware</th>
                <th class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider">Last Seen</th>
                <th class="px-6 py-3" />
              </tr>
            </thead>
            <tbody class="divide-y divide-(--ui-border)">
              <tr
                v-for="device in filteredDevices"
                :key="device.id"
                class="hover:bg-(--ui-bg-accented) transition-colors"
              >
                <td class="px-6 py-4">
                  <p class="text-sm font-medium text-(--ui-text-highlighted)">{{ device.serial_number }}</p>
                  <p class="text-xs text-(--ui-text-muted)">{{ device.model }}</p>
                </td>
                <td class="px-6 py-4 text-sm text-(--ui-text)">{{ device.site_name }}</td>
                <td class="px-6 py-4">
                  <UBadge :color="statusColor(device.status)" variant="subtle" size="sm">
                    {{ device.status }}
                  </UBadge>
                </td>
                <td class="px-6 py-4 text-sm font-mono text-(--ui-text)">v{{ device.firmware_version }}</td>
                <td class="px-6 py-4 text-sm text-(--ui-text-muted)">
                  {{ device.last_seen ? formatTimeAgo(device.last_seen) : 'Never' }}
                </td>
                <td class="px-6 py-4 text-right">
                  <UButton size="xs" variant="ghost" :to="`/devices/${device.id}`">Details</UButton>
                </td>
              </tr>
              <tr v-if="filteredDevices.length === 0 && !loading">
                <td colspan="6" class="px-6 py-12 text-center text-(--ui-text-muted)">No devices found.</td>
              </tr>
            </tbody>
          </table>
        </div>
      </UCard>
    </div>
  </div>
</template>

<script setup lang="ts">
import { useApi } from '../../composables/useApi'

definePageMeta({
  layout: 'authenticated',
  middleware: ['auth'],
})

useSeoMeta({ title: 'Devices - EsoCore Portal' })

interface DeviceItem {
  id: string
  serial_number: string
  model: string
  firmware_version: string
  site: number
  site_name: string
  status: string
  last_seen: string | null
  tags: Record<string, string>
}

interface SiteItem {
  id: number
  name: string
}

interface PaginatedResponse<T> {
  count: number
  results: T[]
}

const api = useApi()
const loading = ref(false)
const errorMsg = ref('')
const devices = ref<DeviceItem[]>([])
const sites = ref<SiteItem[]>([])
const statusFilter = ref('all')
const siteFilter = ref('all')

const statusOptions = [
  { label: 'All statuses', value: 'all' },
  { label: 'Active', value: 'active' },
  { label: 'Inactive', value: 'inactive' },
  { label: 'Maintenance', value: 'maintenance' },
  { label: 'Offline', value: 'offline' },
]

const siteOptions = computed(() => [
  { label: 'All sites', value: 'all' },
  ...sites.value.map(s => ({ label: s.name, value: String(s.id) })),
])

const filteredDevices = computed(() => {
  let result = devices.value
  if (statusFilter.value && statusFilter.value !== 'all') {
    result = result.filter(d => d.status === statusFilter.value)
  }
  if (siteFilter.value && siteFilter.value !== 'all') {
    result = result.filter(d => String(d.site) === siteFilter.value)
  }
  return result
})

function statusColor(status: string): string {
  const map: Record<string, string> = {
    active: 'success',
    inactive: 'neutral',
    maintenance: 'warning',
    offline: 'error',
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

async function fetchDevices() {
  loading.value = true
  errorMsg.value = ''
  try {
    const [devicesData, sitesData] = await Promise.all([
      api.get<PaginatedResponse<DeviceItem>>('/devices/devices/'),
      api.get<PaginatedResponse<SiteItem>>('/devices/sites/'),
    ])
    devices.value = devicesData.results || []
    sites.value = sitesData.results || []
  }
  catch (error: unknown) {
    const err = error as { message?: string; statusCode?: number; data?: { detail?: string } }
    errorMsg.value = err?.data?.detail || err?.message || 'Unknown error'
    console.error('Failed to fetch devices:', error)
  }
  finally {
    loading.value = false
  }
}

onMounted(() => {
  fetchDevices()
})
</script>
