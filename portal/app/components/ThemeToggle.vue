<template>
  <button
    class="rounded-md p-1.5 text-gray-700 dark:text-gray-200 hover:bg-gray-100 dark:hover:bg-gray-700"
    :aria-label="`Switch to ${nextLabel} mode`"
    @click="toggle"
  >
    <span v-if="isDark" class="i-heroicons-moon-20-solid w-5 h-5" aria-hidden="true"></span>
    <span v-else class="i-heroicons-sun-20-solid w-5 h-5" aria-hidden="true"></span>
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


