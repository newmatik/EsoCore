<template>
  <div class="min-h-screen bg-(--ui-bg)">
    <!-- Header -->
    <div class="bg-(--ui-bg-elevated) shadow">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        <div class="flex items-center gap-4">
          <UButton leading-icon="i-heroicons-arrow-left" variant="ghost" to="/devices" />
          <div class="min-w-0">
            <h1 class="text-2xl font-bold text-(--ui-text-highlighted)">
              {{ device?.serial_number || 'Loading...' }}
            </h1>
            <p class="text-(--ui-text-muted)">
              {{ device?.model }} &middot; {{ device?.site_name }}
            </p>
          </div>
          <UBadge v-if="device" :color="statusColor(device.status)" variant="subtle" size="lg" class="ml-auto">
            {{ device.status }}
          </UBadge>
        </div>
      </div>
    </div>

    <div v-if="device" class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8 space-y-8">
      <!-- Device Info Cards -->
      <div class="grid grid-cols-1 md:grid-cols-3 gap-6">
        <UCard>
          <template #header>
            <h3 class="text-sm font-medium text-(--ui-text-muted)">Device Info</h3>
          </template>
          <dl class="space-y-2 text-sm">
            <div class="flex justify-between">
              <dt class="text-(--ui-text-muted)">Serial</dt>
              <dd class="font-mono text-(--ui-text-highlighted)">{{ device.serial_number }}</dd>
            </div>
            <div class="flex justify-between">
              <dt class="text-(--ui-text-muted)">Model</dt>
              <dd class="text-(--ui-text-highlighted)">{{ device.model }}</dd>
            </div>
            <div class="flex justify-between">
              <dt class="text-(--ui-text-muted)">Firmware</dt>
              <dd class="font-mono text-(--ui-text-highlighted)">v{{ device.firmware_version }}</dd>
            </div>
            <div class="flex justify-between">
              <dt class="text-(--ui-text-muted)">Last Seen</dt>
              <dd class="text-(--ui-text-highlighted)">{{ device.last_seen ? formatTimeAgo(device.last_seen) : 'Never' }}</dd>
            </div>
          </dl>
        </UCard>

        <UCard>
          <template #header>
            <h3 class="text-sm font-medium text-(--ui-text-muted)">Sampling Rates</h3>
          </template>
          <dl v-if="device.configuration" class="space-y-2 text-sm">
            <div v-for="(rate, key) in device.configuration.sampling_rates" :key="key" class="flex justify-between">
              <dt class="text-(--ui-text-muted)">{{ key }}</dt>
              <dd class="font-mono text-(--ui-text-highlighted)">{{ rate }} Hz</dd>
            </div>
          </dl>
          <p v-else class="text-sm text-(--ui-text-muted)">No configuration</p>
        </UCard>

        <UCard>
          <template #header>
            <h3 class="text-sm font-medium text-(--ui-text-muted)">Thresholds</h3>
          </template>
          <dl v-if="device.configuration?.thresholds" class="space-y-2 text-sm">
            <div v-for="(val, key) in device.configuration.thresholds" :key="key" class="flex justify-between">
              <dt class="text-(--ui-text-muted)">{{ formatThresholdKey(String(key)) }}</dt>
              <dd class="font-mono text-(--ui-text-highlighted)">{{ val }}</dd>
            </div>
          </dl>
          <p v-else class="text-sm text-(--ui-text-muted)">No thresholds configured</p>
        </UCard>
      </div>

      <!-- Telemetry Points -->
      <UCard>
        <template #header>
          <h3 class="text-lg font-medium text-(--ui-text-highlighted)">Recent Telemetry</h3>
        </template>
        <div class="overflow-x-auto">
          <table class="min-w-full divide-y divide-(--ui-border)">
            <thead>
              <tr>
                <th class="px-4 py-2 text-left text-xs font-medium text-(--ui-text-muted) uppercase">Metric</th>
                <th class="px-4 py-2 text-left text-xs font-medium text-(--ui-text-muted) uppercase">Value</th>
                <th class="px-4 py-2 text-left text-xs font-medium text-(--ui-text-muted) uppercase">Unit</th>
                <th class="px-4 py-2 text-left text-xs font-medium text-(--ui-text-muted) uppercase">Timestamp</th>
              </tr>
            </thead>
            <tbody class="divide-y divide-(--ui-border)">
              <tr v-for="point in telemetryPoints" :key="point.id">
                <td class="px-4 py-2 text-sm font-medium text-(--ui-text-highlighted)">{{ point.metric }}</td>
                <td class="px-4 py-2 text-sm font-mono text-(--ui-text-highlighted)">{{ point.value }}</td>
                <td class="px-4 py-2 text-sm text-(--ui-text-muted)">{{ point.unit }}</td>
                <td class="px-4 py-2 text-sm text-(--ui-text-muted)">{{ formatDateTime(point.timestamp) }}</td>
              </tr>
              <tr v-if="telemetryPoints.length === 0">
                <td colspan="4" class="px-4 py-8 text-center text-(--ui-text-muted)">No telemetry data available.</td>
              </tr>
            </tbody>
          </table>
        </div>
      </UCard>

      <!-- Events -->
      <UCard>
        <template #header>
          <h3 class="text-lg font-medium text-(--ui-text-highlighted)">Event History</h3>
        </template>
        <div class="space-y-3">
          <div
            v-for="event in deviceEvents"
            :key="event.id"
            class="flex items-center justify-between p-3 bg-(--ui-bg-accented) rounded-lg"
          >
            <div class="flex items-center gap-3">
              <div class="w-2.5 h-2.5 rounded-full flex-shrink-0" :class="severityDot(event.severity)" />
              <div>
                <p class="text-sm font-medium text-(--ui-text-highlighted)">{{ event.description }}</p>
                <p class="text-xs text-(--ui-text-muted)">{{ event.event_type }} &middot; {{ formatTimeAgo(event.created_at) }}</p>
              </div>
            </div>
            <UBadge :color="eventStatusColor(event.status)" variant="subtle" size="sm">
              {{ event.status }}
            </UBadge>
          </div>
          <div v-if="deviceEvents.length === 0" class="text-center py-8">
            <p class="text-(--ui-text-muted)">No events recorded.</p>
          </div>
        </div>
      </UCard>
    </div>

    <div v-else class="max-w-7xl mx-auto px-4 py-16 text-center">
      <div v-if="errorMsg" class="max-w-md mx-auto">
        <UCard>
          <div class="flex items-center gap-3 text-red-600 dark:text-red-400">
            <UIcon name="i-heroicons-exclamation-triangle" class="w-5 h-5 flex-shrink-0" />
            <div>
              <p class="text-sm font-medium">Failed to load device</p>
              <p class="text-xs mt-0.5">{{ errorMsg }}</p>
            </div>
          </div>
        </UCard>
      </div>
      <p v-else class="text-(--ui-text-muted)">Loading device details...</p>
    </div>
  </div>
</template>

<script setup lang="ts">
import { useApi } from '../../composables/useApi'

definePageMeta({
  layout: 'authenticated',
  middleware: ['auth'],
})

interface DeviceConfig {
  sampling_rates: Record<string, number>
  thresholds: Record<string, number>
  ntp_servers: string[]
  endpoints: Record<string, string>
}

interface DeviceDetail {
  id: string
  serial_number: string
  model: string
  firmware_version: string
  site: number
  site_name: string
  status: string
  last_seen: string | null
  tags: Record<string, string>
  configuration: DeviceConfig | null
}

interface TelemetryPoint {
  id: number
  metric: string
  value: number
  unit: string
  timestamp: string
}

interface DeviceEvent {
  id: string
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

const route = useRoute()
const api = useApi()
const deviceId = route.params.id as string

const device = ref<DeviceDetail | null>(null)
const telemetryPoints = ref<TelemetryPoint[]>([])
const deviceEvents = ref<DeviceEvent[]>([])
const errorMsg = ref('')

function statusColor(status: string): string {
  const map: Record<string, string> = { active: 'success', inactive: 'neutral', maintenance: 'warning', offline: 'error' }
  return map[status] || 'neutral'
}

function severityDot(severity: string) {
  const map: Record<string, string> = { low: 'bg-green-500', medium: 'bg-yellow-500', high: 'bg-orange-500', critical: 'bg-red-500' }
  return map[severity] || 'bg-gray-400'
}

function eventStatusColor(status: string): string {
  const map: Record<string, string> = { active: 'error', acknowledged: 'info', resolved: 'success', suppressed: 'neutral' }
  return map[status] || 'neutral'
}

function formatTimeAgo(dateStr: string) {
  const diff = Date.now() - new Date(dateStr).getTime()
  const minutes = Math.floor(diff / 60000)
  if (minutes < 1) return 'just now'
  if (minutes < 60) return `${minutes}m ago`
  const hours = Math.floor(minutes / 60)
  if (hours < 24) return `${hours}h ago`
  return `${Math.floor(hours / 24)}d ago`
}

function formatDateTime(dateStr: string) {
  return new Date(dateStr).toLocaleString('en-US', {
    month: 'short', day: 'numeric', hour: '2-digit', minute: '2-digit', second: '2-digit',
  })
}

function formatThresholdKey(key: string) {
  return key.replace(/_/g, ' ').replace(/\b\w/g, c => c.toUpperCase())
}

async function fetchDeviceData() {
  errorMsg.value = ''
  try {
    const [deviceData, telemetryData, eventsData] = await Promise.all([
      api.get<DeviceDetail>(`/devices/devices/${deviceId}/`),
      api.get<PaginatedResponse<TelemetryPoint>>('/telemetry/points/', { device: deviceId, ordering: '-timestamp', page_size: 20 }),
      api.get<PaginatedResponse<DeviceEvent>>('/events/events/', { device: deviceId, ordering: '-created_at', page_size: 10 }),
    ])
    device.value = deviceData
    telemetryPoints.value = telemetryData.results || []
    deviceEvents.value = eventsData.results || []
    useSeoMeta({ title: `${deviceData.serial_number} - EsoCore Portal` })
  }
  catch (error: unknown) {
    const err = error as { data?: { detail?: string }; message?: string }
    errorMsg.value = err?.data?.detail || err?.message || 'Failed to load device data'
    console.error('Failed to fetch device:', error)
  }
}

onMounted(() => { fetchDeviceData() })
</script>
