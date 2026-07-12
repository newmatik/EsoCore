<template>
  <div>
    <SiteHeader />
    <slot />
    <SiteFooter />
  </div>
</template>

<script setup>
const route = useRoute()
const siteUrl = 'https://www.esocore.com'

// Remove trailing slashes for canonical consistency
const canonicalPath = computed(() => route.path.replace(/\/+$/, '') || '/')
const canonicalUrl = computed(() => `${siteUrl}${canonicalPath.value}`)

// Set page title dynamically, canonical URL, and og:url
useHead({
  titleTemplate: title =>
    title ? `${title} – EsoCore` : 'EsoCore – Edge-native intelligence and observability',
  link: [{ rel: 'canonical', href: canonicalUrl }],
  meta: [{ property: 'og:url', content: canonicalUrl }],
})
</script>
