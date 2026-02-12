<template>
  <UButton
    variant="ghost"
    color="neutral"
    :aria-label="`Switch to ${nextLabel} mode`"
    :icon="isDark ? 'i-heroicons-moon-20-solid' : 'i-heroicons-sun-20-solid'"
    @click="toggle"
  />
</template>

<script setup lang="ts">
const colorMode = useColorMode()
const isDark = computed(() => colorMode.value === 'dark')
const nextLabel = computed(() => (isDark.value ? 'light' : 'dark'))
function toggle() {
  colorMode.preference = isDark.value ? 'light' : 'dark'
}
</script>
