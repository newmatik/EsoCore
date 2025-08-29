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
        <div v-if="content">
          <div v-html="renderedContent" />
          <!-- BOM widgets will be mounted here -->
          <div v-for="bomId in bomMounts" :id="bomId" :key="bomId" class="bom-mount" />
        </div>
        <div v-else-if="pending">
          <h1>Loading…</h1>
          <p>Please wait while we load the documentation.</p>
        </div>
        <div v-else>
          <h1>Not found</h1>
          <p>Could not load this document.</p>
        </div>
      </article>
    </div>
  </main>
</template>

<script setup>
import { marked } from 'marked'

const route = useRoute()
const slug = computed(() => {
  const pathSlug = Array.isArray(route.params.slug)
    ? route.params.slug.join('/')
    : route.params.slug
  return pathSlug || 'README'
})

// Track current slug for active navigation
const currentSlug = computed(() => slug.value)

// Documentation structure (same as in docs/index.vue)
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
    slug: 'development-timeline',
    title: 'Development Timeline',
    icon: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="18" rx="2"/><line x1="16" y1="2" x2="16" y2="6"/><line x1="8" y1="2" x2="8" y2="6"/><line x1="3" y1="10" x2="21" y2="10"/></svg>',
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

function onSearch() {
  // Simple search implementation - you can enhance this
  const q = searchQuery.value.trim().toLowerCase()
  if (!q) {
    searchResults.value = []
    return
  }

  searchResults.value = docs.value
    .filter(doc => doc.title.toLowerCase().includes(q) || doc.slug.toLowerCase().includes(q))
    .slice(0, 5)
    .map(doc => ({
      slug: doc.slug,
      title: doc.title,
      snippet: `Documentation for ${doc.title}`,
    }))
}

function clearSearch() {
  searchQuery.value = ''
  searchResults.value = []
}

// Fetch the document content
const { data: content, pending } = await useFetch(`/api/docs/${slug.value}`)

// Render markdown to HTML
const renderedContent = computed(() => {
  if (!content.value) return ''

  // Configure marked options
  marked.setOptions({
    gfm: true,
    breaks: true,
    headerIds: true,
  })

  return marked.parse(content.value)
})

// Set page title
const pageTitle = computed(() => {
  if (!content.value) return 'Not Found'

  // Extract title from first H1 in markdown
  const titleMatch = content.value.match(/^#\s+(.+)$/m)
  if (titleMatch) {
    return titleMatch[1]
      .trim()
      .replace(/^\s*(EsoCore)\s+/i, '')
      .trim()
  }

  return titleFromSlug(slug.value)
})

useHead({
  title: pageTitle,
})

// BOM mounts detection
const bomMounts = ref([])

onMounted(async () => {
  // Detect BOM mount points in the rendered content
  detectBomMounts()

  // Initialize BOM widgets if present
  if (bomMounts.value.length > 0) {
    await initBomWidgets()
  }
})

function titleFromSlug(slug) {
  if (slug === 'README') return 'Introduction'
  return slug.replace(/-/g, ' ').replace(/\b\w/g, c => c.toUpperCase())
}

function detectBomMounts() {
  // Look for BOM mount points in the content
  const content = document.querySelector('.doc-content')
  if (!content) return

  const mounts = content.querySelectorAll('[id^="bom-"]')
  bomMounts.value = Array.from(mounts).map(el => el.id)
}

async function initBomWidgets() {
  // Load BOM script if not already loaded
  if (!window.EdgeBOM) {
    await loadScript('/bom.js')
  }

  // BOM CSV mappings
  const idToCsv = {
    'bom-main': '/docs/data/bom-main.csv',
    'bom-vibration': '/docs/data/bom-vibration-sensor.csv',
    'bom-acoustic': '/docs/data/bom-acoustic-sensor.csv',
    'bom-current': '/docs/data/bom-current-sensor.csv',
    'bom-air-quality': '/docs/data/bom-air-quality-sensor.csv',
    'bom-light': '/docs/data/bom-light-sensor.csv',
    'bom-oil-quality': '/docs/data/bom-oil-quality-sensor.csv',
    'bom-pressure': '/docs/data/bom-pressure-sensor.csv',
    'bom-temperature-spot': '/docs/data/bom-temperature-spot-sensor.csv',
    'bom-proximity-position': '/docs/data/bom-proximity-position-sensor.csv',
    'bom-cables': '/docs/data/bom-cables.csv',
  }

  // Render BOM tables
  bomMounts.value.forEach(mountId => {
    const csv = idToCsv[mountId]
    if (!csv) return

    try {
      window.EdgeBOM.renderBOM(mountId, csv)
    } catch (err) {
      console.error('Failed to render BOM', mountId, err)
      const mount = document.getElementById(mountId)
      if (mount) {
        mount.innerHTML = '<p style="color: red;">Failed to load BOM.</p>'
      }
    }
  })
}

function loadScript(src) {
  return new Promise((resolve, reject) => {
    const script = document.createElement('script')
    script.src = src
    script.async = true
    script.onload = resolve
    script.onerror = () => reject(new Error('Failed to load ' + src))
    document.head.appendChild(script)
  })
}
</script>
