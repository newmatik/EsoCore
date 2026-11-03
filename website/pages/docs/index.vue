<template>
  <main class="container" style="padding-top: 84px">
    <div class="docs-layout">
      <aside class="sidebar">
        <div class="search">
          <input
            v-model="searchQuery"
            type="search"
            placeholder="Search docs..."
            aria-label="Search docs"
            @input="onSearch"
          />
          <div v-if="searchResults.length > 0" class="search-results">
            <NuxtLink
              v-for="result in searchResults"
              :key="result.slug"
              :to="result.slug ? `/docs/${result.slug}` : '/docs'"
              class="result"
              @click="clearSearch"
            >
              <strong>{{ result.title }}</strong
              ><br />
              <small v-html="result.snippet" />
            </NuxtLink>
          </div>
        </div>
        <nav aria-label="Documentation">
          <ul class="nav-list">
            <li v-for="doc in docsConfig" :key="doc.slug || doc.key || `group-${doc.title}`">
              <template v-if="!doc.children">
                <NuxtLink
                  :to="doc.slug ? `/docs/${doc.slug}` : '/docs'"
                  :class="{ active: currentSlug === doc.slug }"
                >
                  <span class="icon" aria-hidden="true" v-html="doc.icon" />
                  <span class="label">{{ doc.title }}</span>
                </NuxtLink>
              </template>
              <template v-else>
                <details class="nav-group" :open="isGroupExpanded(doc, currentSlug)">
                  <summary
                    class="nav-group-header"
                    :class="{ active: isGroupExpanded(doc, currentSlug) }"
                  >
                    <span class="icon" aria-hidden="true" v-html="doc.icon" />
                    <span class="label">{{ doc.title }}</span>
                  </summary>
                  <ul class="nav-sublist">
                    <li v-for="child in doc.children" :key="child.slug">
                      <NuxtLink
                        :to="child.slug ? `/docs/${child.slug}` : '/docs'"
                        :class="{ active: currentSlug === child.slug }"
                      >
                        <span class="icon" aria-hidden="true" v-html="child.icon" />
                        <span class="label">{{ child.title }}</span>
                      </NuxtLink>
                    </li>
                  </ul>
                </details>
              </template>
            </li>
          </ul>
        </nav>
      </aside>
      <article class="doc-content" aria-live="polite">
        <!-- Show the README content by default when on /docs -->
        <div v-if="!$route.params.slug">
          <div v-if="readmeContent" v-html="renderedReadmeContent" />
          <div v-else>
            <h1>Loading…</h1>
            <p>Please wait while we load the documentation.</p>
          </div>
        </div>
        <NuxtPage v-else />
      </article>
    </div>
  </main>
</template>

<script setup>
import { marked } from 'marked'
import { docsConfig, isGroupExpanded } from '~/utils/docs-config'

// Set page meta
useHead({
  title: 'Documentation',
  meta: [
    {
      name: 'description',
      content: 'Documentation for EsoCore: API, data formats, hardware, and more.',
    },
  ],
})

// Track current slug
const route = useRoute()
const currentSlug = computed(() => {
  const pathSlug = Array.isArray(route.params.slug)
    ? route.params.slug.join('/')
    : route.params.slug
  return pathSlug || ''
})

// Load README content for the main docs page
const { data: readmeContent } = await useFetch('/api/docs/README')

// Render README markdown to HTML
const renderedReadmeContent = computed(() => {
  if (!readmeContent.value) return ''

  marked.setOptions({
    gfm: true,
    breaks: false,
    headerIds: true,
  })

  return marked.parse(readmeContent.value)
})

// Helper function to flatten docs for search indexing
function flattenDocs(docs) {
  const flattened = []
  for (const doc of docs) {
    flattened.push(doc)
    if (doc.children) {
      flattened.push(...doc.children)
    }
  }
  return flattened
}

// Search functionality
const searchQuery = ref('')
const searchResults = ref([])
const searchIndex = ref([])

// The index (all docs, ~115 KB compressed) loads on the first search, not on page load.
let searchIndexLoad = null

async function buildSearchIndex() {
  // One prerendered file with every doc's text (server/api/docs-search.json.get.ts).
  let texts = {}
  try {
    texts = await $fetch('/api/docs-search.json')
  } catch (error) {
    console.warn('Failed to load the docs search index:', error)
    return
  }
  searchIndex.value = flattenDocs(docsConfig)
    .filter(doc => typeof texts[doc.slug || 'README'] === 'string')
    .map(doc => ({ slug: doc.slug, title: doc.title, text: texts[doc.slug || 'README'] }))
}

async function onSearch() {
  if (!searchIndexLoad) searchIndexLoad = buildSearchIndex()
  await searchIndexLoad
  const q = searchQuery.value.trim()
  if (!q) {
    searchResults.value = []
    return
  }

  const matches = searchIndex.value
    .flatMap(item => {
      const lines = item.text.split(/\n+/)
      const hit = lines.find(line => line.toLowerCase().includes(q.toLowerCase()))
      if (!hit) return []
      return [
        {
          slug: item.slug,
          title: item.title,
          snippet: highlight(snippetAround(hit, q), q),
        },
      ]
    })
    .slice(0, 8)

  searchResults.value = matches
}

function clearSearch() {
  searchQuery.value = ''
  searchResults.value = []
}

function highlight(text, q) {
  const re = new RegExp(q.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'ig')
  return text.replace(re, m => '<mark>' + m + '</mark>')
}

function snippetAround(text, q, len = 120) {
  const idx = text.toLowerCase().indexOf(q.toLowerCase())
  if (idx < 0) return text.slice(0, len) + (text.length > len ? '…' : '')
  const start = Math.max(0, idx - Math.floor(len / 2))
  const end = Math.min(text.length, start + len)
  return (start > 0 ? '…' : '') + text.slice(start, end) + (end < text.length ? '…' : '')
}
</script>
