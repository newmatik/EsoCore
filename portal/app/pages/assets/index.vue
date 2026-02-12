<template>
  <div class="min-h-screen bg-(--ui-bg)">
    <!-- Header -->
    <div class="bg-(--ui-bg-elevated) shadow">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
        <div class="flex items-center justify-between">
          <div>
            <h1 class="text-2xl font-bold text-(--ui-text-highlighted)">Assets</h1>
            <p class="text-(--ui-text-muted)">Equipment and machinery being monitored</p>
          </div>
          <UButton leading-icon="i-heroicons-arrow-path" :loading="loading" @click="fetchAssets">
            Refresh
          </UButton>
        </div>
      </div>
    </div>

    <!-- Filters -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 pt-6">
      <div class="flex flex-wrap gap-3">
        <USelect v-model="typeFilter" :items="typeOptions" placeholder="All types" class="w-44" />
      </div>
    </div>

    <!-- Content -->
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
      <!-- Error -->
      <UCard v-if="errorMsg" class="mb-6">
        <div class="flex items-center gap-3 text-red-600 dark:text-red-400">
          <UIcon name="i-heroicons-exclamation-triangle" class="w-5 h-5 flex-shrink-0" />
          <div>
            <p class="text-sm font-medium">Failed to load assets</p>
            <p class="text-xs mt-0.5">{{ errorMsg }}</p>
          </div>
        </div>
      </UCard>

      <!-- Loading -->
      <div v-if="loading && assets.length === 0" class="text-center py-16">
        <UIcon name="i-heroicons-arrow-path" class="w-6 h-6 animate-spin text-(--ui-text-muted) mx-auto" />
        <p class="mt-3 text-(--ui-text-muted)">Loading assets...</p>
      </div>

      <!-- Assets Grid -->
      <template v-else>
        <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          <UCard v-for="asset in filteredAssets" :key="asset.id">
            <div class="flex items-start justify-between mb-3">
              <div>
                <h3 class="text-sm font-semibold text-(--ui-text-highlighted)">{{ asset.asset_id }}</h3>
                <p class="text-xs text-(--ui-text-muted)">{{ asset.make }} {{ asset.model }}</p>
              </div>
              <UBadge :color="typeColor(asset.asset_type)" variant="subtle" size="sm">
                {{ assetTypeLabel(asset.asset_type) }}
              </UBadge>
            </div>

            <dl class="space-y-2 text-sm">
              <div class="flex justify-between">
                <dt class="text-(--ui-text-muted)">Site</dt>
                <dd class="text-(--ui-text-highlighted)">{{ asset.site_name }}</dd>
              </div>
              <div class="flex justify-between">
                <dt class="text-(--ui-text-muted)">Location</dt>
                <dd class="text-(--ui-text-highlighted)">{{ asset.location || '-' }}</dd>
              </div>
              <div v-if="asset.serial_number" class="flex justify-between">
                <dt class="text-(--ui-text-muted)">Serial</dt>
                <dd class="font-mono text-xs text-(--ui-text-highlighted)">{{ asset.serial_number }}</dd>
              </div>
              <div class="flex justify-between">
                <dt class="text-(--ui-text-muted)">Cycles</dt>
                <dd class="text-(--ui-text-highlighted)">{{ asset.cycle_count }}</dd>
              </div>
            </dl>

            <!-- Specifications -->
            <div v-if="asset.specifications && Object.keys(asset.specifications).length > 0" class="mt-3 pt-3 border-t border-(--ui-border)">
              <p class="text-xs font-medium text-(--ui-text-muted) mb-1.5">Specifications</p>
              <div class="flex flex-wrap gap-1.5">
                <UBadge
                  v-for="(val, key) in asset.specifications"
                  :key="key"
                  variant="subtle"
                  color="neutral"
                  size="sm"
                >
                  {{ formatSpecKey(String(key)) }}: {{ val }}
                </UBadge>
              </div>
            </div>

            <!-- Status -->
            <div class="mt-3 flex items-center gap-1.5">
              <span class="w-1.5 h-1.5 rounded-full" :class="asset.is_active ? 'bg-green-500' : 'bg-gray-400'" />
              <span class="text-xs" :class="asset.is_active ? 'text-green-600 dark:text-green-400' : 'text-(--ui-text-muted)'">
                {{ asset.is_active ? 'Active' : 'Inactive' }}
              </span>
            </div>
          </UCard>
        </div>

        <div v-if="filteredAssets.length === 0 && !loading" class="text-center py-12">
          <p class="text-(--ui-text-muted)">No assets found.</p>
        </div>
      </template>
    </div>
  </div>
</template>

<script setup lang="ts">
import { useApi } from '../../composables/useApi'

definePageMeta({
  layout: 'authenticated',
  middleware: ['auth'],
})

useSeoMeta({ title: 'Assets - EsoCore Portal' })

interface AssetItem {
  id: string
  asset_id: string
  site: number
  site_name: string
  asset_type: string
  make: string
  model: string
  serial_number: string
  location: string
  specifications: Record<string, unknown>
  is_active: boolean
  cycle_count: number
}

interface PaginatedResponse<T> {
  count: number
  results: T[]
}

const api = useApi()
const loading = ref(false)
const errorMsg = ref('')
const assets = ref<AssetItem[]>([])
const typeFilter = ref('all')

const typeOptions = [
  { label: 'All types', value: 'all' },
  { label: 'CNC Machine', value: 'cnc' },
  { label: 'Robot', value: 'robot' },
  { label: 'Conveyor', value: 'conveyor' },
  { label: 'Pump', value: 'pump' },
  { label: 'Motor', value: 'motor' },
  { label: 'Door', value: 'door' },
  { label: 'Other', value: 'other' },
]

const filteredAssets = computed(() => {
  if (!typeFilter.value || typeFilter.value === 'all') return assets.value
  return assets.value.filter(a => a.asset_type === typeFilter.value)
})

const typeLabels: Record<string, string> = {
  cnc: 'CNC Machine', robot: 'Robot', conveyor: 'Conveyor',
  pump: 'Pump', motor: 'Motor', door: 'Door', other: 'Other',
}

function assetTypeLabel(type: string) { return typeLabels[type] || type }

function typeColor(type: string): string {
  const map: Record<string, string> = {
    cnc: 'info', robot: 'secondary', conveyor: 'info',
    pump: 'success', motor: 'warning', door: 'neutral',
  }
  return map[type] || 'neutral'
}

function formatSpecKey(key: string) {
  return key.replace(/_/g, ' ').replace(/\b\w/g, c => c.toUpperCase())
}

async function fetchAssets() {
  loading.value = true
  errorMsg.value = ''
  try {
    const data = await api.get<PaginatedResponse<AssetItem>>('/assets/assets/')
    assets.value = data.results || []
  }
  catch (error: unknown) {
    const err = error as { message?: string; data?: { detail?: string } }
    errorMsg.value = err?.data?.detail || err?.message || 'Unknown error'
    console.error('Failed to fetch assets:', error)
  }
  finally {
    loading.value = false
  }
}

onMounted(() => {
  fetchAssets()
})
</script>
