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
            <li v-for="doc in docs" :key="doc.slug">
              <NuxtLink
                :to="doc.slug ? `/docs/${doc.slug}` : '/docs'"
                :class="{ active: currentSlug === doc.slug }"
              >
                <span class="icon" aria-hidden="true" v-html="doc.icon" />
                <span class="label">{{ doc.title }}</span>
              </NuxtLink>
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
    breaks: true,
    headerIds: true,
  })

  return marked.parse(readmeContent.value)
})

// Documentation structure
const docs = ref([
  {
    slug: '',
    title: 'Introduction',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 19.5V6.5a2 2 0 0 1 2-2h0l4 2 4-2h0a2 2 0 0 1 2 2v13"/><path d="M6 20V6"/></svg>',
  },
  {
    slug: 'esocore-edge',
    title: 'Hardware: Edge',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06A1.65 1.65 0 0 0 15 19.4a1.65 1.65 0 0 0-1 .6 1.65 1.65 0 0 0-.33 1.82l-.06.06a2 2 0 1 1-2.83 0l-.06-.06A1.65 1.65 0 0 0 8.6 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.6 15a1.65 1.65 0 0 0-.6-1 1.65 1.65 0 0 0-1.82-.33l-.06-.06a2 2 0 1 1 0-2.83l.06-.06A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.6a1.65 1.65 0 0 0 1-.6 1.65 1.65 0 0 0 .33-1.82l.06-.06a2 2 0 1 1 2.83 0l.06.06A1.65 1.65 0 0 0 15 4.6a1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9c.24.49.24 1.06 0 1.56.24.49.24 1.06 0 1.56Z"/></svg>',
  },
  {
    slug: 'esocore-sensors',
    title: 'Hardware: Sensors',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2v6m0 0l-3-3m3 3 3-3"/><path d="M12 8v6m0 0l-3 3m3-3 3 3"/><path d="M8 12h8"/><circle cx="12" cy="12" r="3"/></svg>',
  },
  {
    slug: 'edge-intelligence',
    title: 'Edge Intelligence',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/></svg>',
  },
  {
    slug: 'data-format-specification',
    title: 'Data Format Specification',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><ellipse cx="12" cy="5" rx="9" ry="3"/><path d="M21 5v6c0 1.66-4.03 3-9 3s-9-1.34-9-3V5"/><path d="M3 11v6c0 1.66 4.03 3 9 3s9-1.34 9-3v-6"/></svg>',
  },
  {
    slug: 'api-specification',
    title: 'API Specification',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="8 4 4 8 8 12"/><polyline points="16 4 20 8 16 12"/><line x1="12" y1="16" x2="12" y2="20"/></svg>',
  },
  {
    slug: 'cloud-infrastructure',
    title: 'Cloud Infrastructure',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M18 10h-1.26A8 8 0 1 0 9 20h9a5 5 0 0 0 0-10z"/><path d="M12 2v6m0 0l-3-3m3 3 3-3"/></svg>',
  },
  {
    slug: 'testing-procedures',
    title: 'Testing & Validation',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9 11l3 3L22 4"/><path d="M21 12v7a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11"/></svg>',
  },
  {
    slug: 'development-environment',
    title: 'Development Environment',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="14" rx="2"/><path d="M8 9l-3 3 3 3"/><path d="M13 15h5"/></svg>',
  },
  {
    slug: 'business-model-and-partnerships',
    title: 'Business Model & Partnerships',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="7" width="20" height="14" rx="2"/><path d="M16 7V5a2 2 0 0 0-2-2H10a2 2 0 0 0-2 2v2"/></svg>',
  },
  {
    slug: 'competitor-analysis',
    title: 'Competitor Analysis',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 3v18h18"/><rect x="7" y="12" width="3" height="6"/><rect x="12" y="9" width="3" height="9"/><rect x="17" y="5" width="3" height="13"/></svg>',
  },
  {
    slug: 'fieldbus',
    title: 'Industrial Fieldbus Protocols',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="14" rx="2"/><path d="M8 9l-3 3 3 3"/><path d="M13 15h5"/></svg>',
  },
  {
    slug: 'bom',
    title: 'Bill of Materials',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16l4-2 4 2 4-2 4 2V9z"/></svg>',
  },
  {
    slug: 'license',
    title: 'License',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16l4-2 4 2 4-2 4 2V9z"/></svg>',
  },
])

// Search functionality
const searchQuery = ref('')
const searchResults = ref([])
const searchIndex = ref([])

// Build search index on mount
onMounted(async () => {
  await buildSearchIndex()
})

async function buildSearchIndex() {
  const items = []
  for (const doc of docs.value) {
    try {
      const content = await $fetch(`/api/docs/${doc.slug || 'README'}`)
      items.push({
        slug: doc.slug,
        title: doc.title,
        text: content,
      })
    } catch (error) {
      console.warn(`Failed to load content for ${doc.slug}:`, error)
    }
  }
  searchIndex.value = items
}

function onSearch() {
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
