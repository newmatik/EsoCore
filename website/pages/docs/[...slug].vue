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
                <div class="nav-group">
                  <NuxtLink
                    :to="getGroupHeaderLink(doc)"
                    class="nav-group-header"
                    :class="{ active: isGroupExpanded(doc, currentSlug) }"
                  >
                    <span class="icon" aria-hidden="true" v-html="doc.icon" />
                    <span class="label">{{ doc.title }}</span>
                  </NuxtLink>
                  <ul class="nav-sublist" v-show="isGroupExpanded(doc, currentSlug)">
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
                </div>
              </template>
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
import { docsConfig, getGroupHeaderLink, isGroupExpanded } from '~/utils/docs-config'

const route = useRoute()
const slug = computed(() => {
  const pathSlug = Array.isArray(route.params.slug)
    ? route.params.slug.join('/')
    : route.params.slug
  return pathSlug || 'README'
})

// Track current slug for active navigation
const currentSlug = computed(() => slug.value)

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

function onSearch() {
  // Simple search implementation - you can enhance this
  const q = searchQuery.value.trim().toLowerCase()
  if (!q) {
    searchResults.value = []
    return
  }

  const flatDocs = flattenDocs(docsConfig)
  searchResults.value = flatDocs
    .filter(doc => doc.title.toLowerCase().includes(q) || doc.slug?.toLowerCase().includes(q))
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
    'bom-main': '/bom/bom-main.csv',
    'bom-vibration': '/bom/bom-vibration-sensor.csv',
    'bom-acoustic': '/bom/bom-acoustic-sensor.csv',
    'bom-current': '/bom/bom-current-sensor.csv',
    'bom-air-quality': '/bom/bom-air-quality-sensor.csv',
    'bom-light': '/bom/bom-light-sensor.csv',
    'bom-oil-quality': '/bom/bom-oil-quality-sensor.csv',
    'bom-pressure': '/bom/bom-pressure-sensor.csv',
    'bom-temperature-spot': '/bom/bom-temperature-spot-sensor.csv',
    'bom-proximity-position': '/bom/bom-proximity-position-sensor.csv',
    'bom-cables': '/bom/bom-cables.csv',
    // support per-BOM page mount ids that include "-sensor"
    'bom-vibration-sensor': '/bom/bom-vibration-sensor.csv',
    'bom-acoustic-sensor': '/bom/bom-acoustic-sensor.csv',
    'bom-current-sensor': '/bom/bom-current-sensor.csv',
    'bom-air-quality-sensor': '/bom/bom-air-quality-sensor.csv',
    'bom-light-sensor': '/bom/bom-light-sensor.csv',
    'bom-oil-quality-sensor': '/bom/bom-oil-quality-sensor.csv',
    'bom-pressure-sensor': '/bom/bom-pressure-sensor.csv',
    'bom-temperature-spot-sensor': '/bom/bom-temperature-spot-sensor.csv',
    'bom-proximity-position-sensor': '/bom/bom-proximity-position-sensor.csv',
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
