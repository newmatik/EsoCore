<template>
  <div>
    <SiteHeader />
    <slot />
    <SiteFooter />
  </div>
</template>

<script setup>
// Set page title dynamically
useHead({
  titleTemplate: title =>
    title ? `${title} – EsoCore` : 'EsoCore – Edge-native intelligence and observability',
})
</script>
