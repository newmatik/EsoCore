<template>
  <div class="error-page">
    <SiteHeader />
    <main class="container error-content">
      <h1>{{ error?.statusCode || 404 }}</h1>
      <p class="error-message">{{ error?.statusMessage || 'Page not found' }}</p>
      <p class="error-help">
        The page you are looking for does not exist or may have been moved.
      </p>
      <div class="error-actions">
        <NuxtLink to="/" class="btn">Go to Homepage</NuxtLink>
        <NuxtLink to="/docs" class="btn secondary">Browse Documentation</NuxtLink>
      </div>
    </main>
    <SiteFooter />
  </div>
</template>

<script setup>
const props = defineProps({
  error: Object,
})

useHead({
  title: `${props.error?.statusCode || 404} - Page Not Found`,
  meta: [
    { name: 'description', content: 'The requested page could not be found.' },
    { name: 'robots', content: 'noindex' },
  ],
})
</script>

<style scoped>
.error-content {
  padding-top: 140px;
  padding-bottom: 80px;
  text-align: center;
  min-height: 60vh;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
}

.error-content h1 {
  font-size: 6rem;
  font-weight: 700;
  margin: 0;
  color: var(--accent);
  line-height: 1;
}

.error-message {
  font-size: 1.5rem;
  color: var(--text);
  margin: 1rem 0 0.5rem;
}

.error-help {
  color: var(--muted);
  margin-bottom: 2rem;
  max-width: 400px;
}

.error-actions {
  display: flex;
  gap: 1rem;
  flex-wrap: wrap;
  justify-content: center;
}
</style>
