<template>
  <button
    class="rounded-md p-1.5 text-gray-700 dark:text-gray-200 hover:bg-gray-100 dark:hover:bg-gray-700"
    :aria-label="`Switch to ${nextLabel} mode`"
    @click="toggle"
  >
    <UIcon v-if="isDark" name="i-heroicons-moon-20-solid" class="w-5 h-5" aria-hidden="true" />
    <UIcon v-else name="i-heroicons-sun-20-solid" class="w-5 h-5" aria-hidden="true" />
  </button>
</template>

<script setup lang="ts">
const colorMode = useColorMode()
const isDark = computed(() => colorMode.value === 'dark')
const nextLabel = computed(() => (isDark.value ? 'light' : 'dark'))
function toggle(){
  colorMode.preference = isDark.value ? 'light' : 'dark'
}
</script>


