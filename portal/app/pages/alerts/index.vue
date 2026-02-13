<template>
  <div class="min-h-screen bg-(--ui-bg)">
    <!-- Header -->
    <div class="bg-(--ui-bg-elevated) shadow">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        <div class="flex items-center justify-between">
          <div>
            <h1 class="text-2xl font-bold text-(--ui-text-highlighted)">Alerts</h1>
            <p class="text-(--ui-text-muted)">System events and alert management</p>
          </div>
          <UButton leading-icon="i-heroicons-arrow-path" :loading="loading" @click="fetchEvents">
            Refresh
          </UButton>
        </div>
      </div>
    </div>

    <!-- Filters -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 pt-6">
      <div class="flex flex-wrap gap-3">
        <USelect
          v-model="severityFilter"
          :items="severityOptions"
          placeholder="All severities"
          class="w-40"
        />
        <USelect
          v-model="statusFilter"
          :items="statusOptions"
          placeholder="All statuses"
          class="w-44"
        />
      </div>
    </div>

    <!-- Content -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
      <!-- Error -->
      <UCard v-if="errorMsg" class="mb-6">
        <div class="flex items-center gap-3 text-red-600 dark:text-red-400">
          <UIcon name="i-heroicons-exclamation-triangle" class="w-5 h-5 flex-shrink-0" />
          <div>
            <p class="text-sm font-medium">Failed to load alerts</p>
            <p class="text-xs mt-0.5">{{ errorMsg }}</p>
          </div>
        </div>
      </UCard>

      <!-- Loading -->
      <div v-if="loading && events.length === 0" class="text-center py-16">
        <UIcon
          name="i-heroicons-arrow-path"
          class="w-6 h-6 animate-spin text-(--ui-text-muted) mx-auto"
        />
        <p class="mt-3 text-(--ui-text-muted)">Loading alerts...</p>
      </div>

      <!-- Events Table -->
      <UCard v-else>
        <div class="overflow-x-auto">
          <table class="min-w-full divide-y divide-(--ui-border)">
            <thead>
              <tr>
                <th
                  class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider"
                >
                  Severity
                </th>
                <th
                  class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider"
                >
                  Event
                </th>
                <th
                  class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider"
                >
                  Device
                </th>
                <th
                  class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider"
                >
                  Status
                </th>
                <th
                  class="px-6 py-3 text-left text-xs font-medium text-(--ui-text-muted) uppercase tracking-wider"
                >
                  Time
                </th>
                <th class="px-6 py-3" />
              </tr>
            </thead>
            <tbody class="divide-y divide-(--ui-border)">
              <tr
                v-for="event in filteredEvents"
                :key="event.id"
                class="hover:bg-(--ui-bg-accented) transition-colors"
              >
                <td class="px-6 py-4">
                  <UBadge :color="severityColor(event.severity)" variant="subtle" size="sm">
                    {{ event.severity }}
                  </UBadge>
                </td>
                <td class="px-6 py-4">
                  <p class="text-sm font-medium text-(--ui-text-highlighted)">
                    {{ event.event_type.replace(/_/g, ' ') }}
                  </p>
                  <p class="text-xs text-(--ui-text-muted) max-w-md truncate">
                    {{ event.description }}
                  </p>
                </td>
                <td class="px-6 py-4 text-sm font-mono text-(--ui-text)">
                  {{ event.device_serial }}
                </td>
                <td class="px-6 py-4">
                  <UBadge :color="eventStatusColor(event.status)" variant="subtle" size="sm">
                    {{ event.status }}
                  </UBadge>
                </td>
                <td class="px-6 py-4 text-sm text-(--ui-text-muted)">
                  {{ formatTimeAgo(event.created_at) }}
                </td>
                <td class="px-6 py-4 text-right">
                  <div class="flex items-center gap-2 justify-end">
                    <UButton
                      v-if="event.status === 'active'"
                      size="xs"
                      variant="outline"
                      @click="acknowledgeEvent(event.id)"
                    >
                      Acknowledge
                    </UButton>
                    <UButton
                      v-if="event.status === 'active' || event.status === 'acknowledged'"
                      size="xs"
                      variant="subtle"
                      color="success"
                      @click="resolveEvent(event.id)"
                    >
                      Resolve
                    </UButton>
                  </div>
                </td>
              </tr>
              <tr v-if="filteredEvents.length === 0 && !loading">
                <td colspan="6" class="px-6 py-12 text-center text-(--ui-text-muted)">
                  No events found.
                </td>
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

useSeoMeta({ title: 'Alerts - EsoCore Portal' })

interface SystemEvent {
  id: string
  device: string
  device_serial: string
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
const loading = ref(false)
const errorMsg = ref('')
const events = ref<SystemEvent[]>([])
const severityFilter = ref('all')
const statusFilter = ref('all')

const severityOptions = [
  { label: 'All severities', value: 'all' },
  { label: 'Critical', value: 'critical' },
  { label: 'High', value: 'high' },
  { label: 'Medium', value: 'medium' },
  { label: 'Low', value: 'low' },
]

const statusOptions = [
  { label: 'All statuses', value: 'all' },
  { label: 'Active', value: 'active' },
  { label: 'Acknowledged', value: 'acknowledged' },
  { label: 'Resolved', value: 'resolved' },
  { label: 'Suppressed', value: 'suppressed' },
]

const filteredEvents = computed(() => {
  let result = events.value
  if (severityFilter.value && severityFilter.value !== 'all')
    result = result.filter(e => e.severity === severityFilter.value)
  if (statusFilter.value && statusFilter.value !== 'all')
    result = result.filter(e => e.status === statusFilter.value)
  return result
})

function severityColor(severity: string): string {
  const map: Record<string, string> = {
    low: 'success',
    medium: 'warning',
    high: 'warning',
    critical: 'error',
  }
  return map[severity] || 'neutral'
}

function eventStatusColor(status: string): string {
  const map: Record<string, string> = {
    active: 'error',
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
  return `${Math.floor(hours / 24)}d ago`
}

async function fetchEvents() {
  loading.value = true
  errorMsg.value = ''
  try {
    const data = await api.get<PaginatedResponse<SystemEvent>>('/events/events/', {
      ordering: '-created_at',
    })
    events.value = data.results || []
  } catch (error: unknown) {
    const err = error as { message?: string; data?: { detail?: string } }
    errorMsg.value = err?.data?.detail || err?.message || 'Unknown error'
    console.error('Failed to fetch events:', error)
  } finally {
    loading.value = false
  }
}

async function acknowledgeEvent(eventId: string) {
  try {
    await api.post(`/events/events/${eventId}/acknowledge/`)
    const event = events.value.find(e => e.id === eventId)
    if (event) event.status = 'acknowledged'
  } catch (error: unknown) {
    const err = error as { data?: { detail?: string }; message?: string }
    toast.add({
      title: 'Failed to acknowledge event',
      description: err?.data?.detail || err?.message || 'Unknown error',
      color: 'error',
    })
  }
}

async function resolveEvent(eventId: string) {
  try {
    await api.post(`/events/events/${eventId}/resolve/`)
    const event = events.value.find(e => e.id === eventId)
    if (event) event.status = 'resolved'
  } catch (error: unknown) {
    const err = error as { data?: { detail?: string }; message?: string }
    toast.add({
      title: 'Failed to resolve event',
      description: err?.data?.detail || err?.message || 'Unknown error',
      color: 'error',
    })
  }
}

onMounted(() => {
  fetchEvents()
})
</script>
