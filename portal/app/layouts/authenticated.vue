<template>
  <div class="min-h-screen bg-(--ui-bg)">
    <header class="bg-(--ui-bg-elevated) border-b border-(--ui-border)">
      <div
        class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-3 flex items-center justify-between gap-2"
      >
        <!-- Brand -->
        <NuxtLink
          to="/dashboard"
          class="flex min-w-0 items-center gap-2 hover:opacity-80 transition-opacity"
        >
          <img src="/icon.svg" alt="Portal Icon" class="h-6 w-6 shrink-0" />
          <span class="truncate text-base sm:text-lg font-semibold text-(--ui-text-highlighted)">
            {{ portalTitle }}
          </span>
        </NuxtLink>

        <div class="flex shrink-0 items-center gap-1">
          <!-- Navigation (medium screens and up) -->
          <nav aria-label="Main" class="hidden md:flex items-center gap-1">
            <NuxtLink
              v-for="item in navItems"
              :key="item.to"
              :to="item.to"
              class="px-3 py-1.5 text-sm rounded-md transition-colors"
              :class="navItemClass(item.to)"
            >
              {{ item.label }}
            </NuxtLink>
          </nav>

          <div class="hidden md:block w-px h-5 bg-(--ui-border) mx-2" />

          <ClientOnly>
            <ThemeToggle />
          </ClientOnly>

          <!-- User Menu -->
          <div class="relative ml-1" ref="menuRef">
            <button
              type="button"
              aria-label="Account menu"
              :aria-expanded="showMenu"
              @click="showMenu = !showMenu"
              class="flex items-center gap-2 px-2 py-1.5 rounded-md text-sm text-(--ui-text-muted) hover:bg-(--ui-bg-accented) transition-colors"
            >
              <span
                class="w-7 h-7 rounded-full bg-(--ui-color-primary-100) dark:bg-(--ui-color-primary-950) text-(--ui-color-primary-600) flex items-center justify-center text-xs font-semibold"
              >
                {{ authStore.initials || '?' }}
              </span>
              <span class="hidden sm:inline text-(--ui-text)">{{
                authStore.displayName || 'User'
              }}</span>
              <UIcon name="i-heroicons-chevron-down" class="w-4 h-4" />
            </button>

            <!-- Dropdown -->
            <Transition
              enter-active-class="transition ease-out duration-100"
              enter-from-class="transform opacity-0 scale-95"
              enter-to-class="transform opacity-100 scale-100"
              leave-active-class="transition ease-in duration-75"
              leave-from-class="transform opacity-100 scale-100"
              leave-to-class="transform opacity-0 scale-95"
            >
              <div
                v-if="showMenu"
                class="absolute right-0 mt-2 w-48 bg-(--ui-bg-elevated) rounded-lg shadow-lg border border-(--ui-border) py-1 z-50"
              >
                <div class="px-4 py-2 border-b border-(--ui-border)">
                  <p class="text-sm font-medium text-(--ui-text-highlighted)">
                    {{ authStore.displayName }}
                  </p>
                  <p class="text-xs text-(--ui-text-muted) truncate">
                    {{ authStore.user?.email }}
                  </p>
                </div>
                <button
                  type="button"
                  @click="handleLogout"
                  class="w-full text-left px-4 py-2 text-sm text-red-600 dark:text-red-400 hover:bg-(--ui-bg-accented) transition-colors"
                >
                  Sign out
                </button>
              </div>
            </Transition>
          </div>

          <!-- Navigation toggle (small screens) -->
          <UButton
            class="md:hidden"
            variant="ghost"
            color="neutral"
            :icon="showNav ? 'i-heroicons-x-mark' : 'i-heroicons-bars-3'"
            :aria-label="showNav ? 'Close navigation' : 'Open navigation'"
            :aria-expanded="showNav"
            aria-controls="mobile-nav"
            @click="showNav = !showNav"
          />
        </div>
      </div>

      <!-- Navigation (small screens) -->
      <nav
        v-if="showNav"
        id="mobile-nav"
        aria-label="Main"
        class="md:hidden border-t border-(--ui-border) px-4 sm:px-6 py-2 flex flex-col gap-1"
      >
        <NuxtLink
          v-for="item in navItems"
          :key="item.to"
          :to="item.to"
          class="px-3 py-2 text-sm rounded-md transition-colors"
          :class="navItemClass(item.to)"
        >
          {{ item.label }}
        </NuxtLink>
      </nav>
    </header>

    <main>
      <slot />
    </main>
  </div>
</template>

<script setup lang="ts">
import { useBranding } from '../composables/useBranding'
import { useAuthStore } from '../stores/auth'

const { portalTitle } = useBranding()
const authStore = useAuthStore()
const route = useRoute()

useHead({ title: portalTitle })

const showMenu = ref(false)
const showNav = ref(false)
const menuRef = ref<HTMLElement>()

const navItems = [
  { to: '/dashboard', label: 'Dashboard' },
  { to: '/devices', label: 'Devices' },
  { to: '/alerts', label: 'Alerts' },
  { to: '/assets', label: 'Assets' },
]

function isActive(path: string) {
  return route.path === path || route.path.startsWith(path + '/')
}

function navItemClass(path: string) {
  return isActive(path)
    ? 'bg-(--ui-bg-accented) text-(--ui-text-highlighted) font-medium'
    : 'text-(--ui-text-muted) hover:bg-(--ui-bg-accented) hover:text-(--ui-text)'
}

// Close the small-screen navigation after a link has been followed.
watch(
  () => route.path,
  () => {
    showNav.value = false
  }
)

async function handleLogout() {
  showMenu.value = false
  await authStore.logout()
  await navigateTo('/login')
}

function handleClickOutside(event: MouseEvent) {
  if (menuRef.value && !menuRef.value.contains(event.target as Node)) {
    showMenu.value = false
  }
}

onMounted(() => {
  document.addEventListener('click', handleClickOutside)
})
onUnmounted(() => {
  document.removeEventListener('click', handleClickOutside)
})
</script>
