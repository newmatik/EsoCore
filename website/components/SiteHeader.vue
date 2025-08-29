<template>
  <header class="header">
    <div class="container navbar">
      <NuxtLink to="/" class="brand">
        <img src="/icon.svg" alt="EsoCore" width="28" height="28" />
        <span>EsoCore</span>
      </NuxtLink>
      <nav class="nav" aria-label="Main Navigation">
        <NuxtLink to="/" :class="{ active: $route.path === '/' }">Home</NuxtLink>
        <NuxtLink to="/docs" :class="{ active: $route.path.startsWith('/docs') }">Docs</NuxtLink>
        <a href="https://github.com/newmatik/EsoCore" target="_blank" rel="noopener">GitHub</a>
      </nav>
    </div>
  </header>
</template>

<script setup>
// No additional setup needed for this component
</script>

<style scoped>
.nav a.active {
  color: var(--text);
  background: rgba(255, 255, 255, 0.04);
  border: 1px solid var(--border);
  border-color: rgba(0, 220, 130, 0.35);
}
</style>
