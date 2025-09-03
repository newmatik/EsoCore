<template>
  <main style="padding-top: 84px">
    <section class="hero">
      <div class="container hero-grid">
        <div>
          <div class="kicker">Modern • Complete • Open Source</div>
          <h1>Edge-native intelligence and observability</h1>
          <p ref="heroTyped" class="typewriter" aria-live="polite">
            EsoCore turns raw edge data into actionable insight.
          </p>
          <p class="hero-subcopy">
            EsoCore is an open‑source platform that makes machines easy to monitor and understand.
            Plug‑and‑play sensors capture what matters, on‑device AI spots issues early, and
            everything keeps working—even when the network is down. It connects cleanly to your
            existing systems and includes fleet setup, updates, alerts, and dashboards—so you
            prevent downtime and keep full ownership of your data.
          </p>
          <div class="cta">
            <NuxtLink to="/docs" class="btn">Read the Docs</NuxtLink>
            <a
              class="btn secondary"
              href="https://github.com/newmatik/EsoCore"
              target="_blank"
              rel="noopener"
              >View on GitHub</a
            >
          </div>
        </div>
        <div class="hero-art" aria-hidden="true">
          <img ref="heroImage" src="/pcb.png" alt="" />
        </div>
      </div>
    </section>

    <section class="features">
      <div class="container">
        <div class="features-grid">
          <div class="panel feature">
            <div class="kicker">Edge-first</div>
            <h3>Process where data is born</h3>
            <p>Reduce latency and cloud costs while keeping sensitive data on-device.</p>
          </div>
          <div class="panel feature">
            <div class="kicker">Open</div>
            <h3>Fully open-source</h3>
            <p>Transparent, auditable code with an Apache 2.0 license for maximum freedom.</p>
          </div>
          <div class="panel feature">
            <div class="kicker">Minimal</div>
            <h3>Low overhead, high signal</h3>
            <p>A focused stack with no bloat, designed for clarity and speed.</p>
          </div>
          <div class="panel feature">
            <div class="kicker">Modular sensors</div>
            <h3>RS‑485 digital sensor bus</h3>
            <p>Plug‑and‑play sensor modules enable fast integration of new or sensors.</p>
          </div>
          <div class="panel feature">
            <div class="kicker">Modern Development Framework</div>
            <h3>C, Python, Node</h3>
            <p>Modern development frameworks for fast, secure, and reliable code.</p>
          </div>
          <div class="panel feature">
            <div class="kicker">Customizable</div>
            <h3>STEP, Altium Designer, FreeCAD</h3>
            <p>Modify hardware or PCBs and customize to your needs.</p>
          </div>
        </div>
      </div>
    </section>
  </main>
</template>

<script setup>
// Set page meta
useHead({
  title: 'EsoCore – Edge-native intelligence and observability',
  meta: [
    {
      name: 'description',
      content:
        'EsoCore brings edge-native intelligence, observability and control in a sleek, minimal, open-source package.',
    },
  ],
})

// Hero image animation
const heroImage = ref(null)
const heroTyped = ref(null)

// Typewriter messages
const typewriterMessages = [
  'Open‑source. Apache 2.0 licensed.',
  'Clean REST API access.',
  'Industrial fieldbus: Modbus, PROFIBUS, PROFINET.',
  'Edge intelligence with TinyML.',
  'Robust hardware with RTC backup.',
  'Reliable sync with idempotency.',
  'Configurable sensing & I/O.',
  'Built for EMC and safety.',
  'Deploy anywhere: shopfloor to cloud.',
  'Privacy‑first. Own your data.',
  'Edge-first. Process where data is born.',
  'Minimal. Low overhead, high signal.',
  'Fully open-source. Transparent, auditable code.',
  'Modern. Complete. Open.',
  'One platform, multiple partners.',
]

let typewriterPool = []
let typewriterLast = ''
let typewriterTimer = null

function shuffle(arr) {
  const a = arr.slice()
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1))
    const t = a[i]
    a[i] = a[j]
    a[j] = t
  }
  return a
}

function showNextTypewriterMessage() {
  if (typewriterPool.length === 0) {
    typewriterPool = shuffle(typewriterMessages.filter(m => m !== typewriterLast))
  }
  const text = typewriterPool.shift()
  typewriterLast = text
  typeText(text, 0)
}

function typeText(text, i) {
  if (!heroTyped.value) return

  heroTyped.value.textContent = text.slice(0, i) + '▌'
  if (i < text.length) {
    setTimeout(() => typeText(text, i + 1), 18)
  } else {
    // Blink cursor for remainder of 3s window
    const endAt = Date.now() + 3000
    clearInterval(typewriterTimer)
    typewriterTimer = setInterval(() => {
      if (!heroTyped.value) return

      const cur = heroTyped.value.textContent.endsWith('▌')
      heroTyped.value.textContent = text + (cur ? ' ' : '▌')
      if (Date.now() > endAt) {
        clearInterval(typewriterTimer)
        showNextTypewriterMessage()
      }
    }, 420)
  }
}

function initTypewriter() {
  if (!heroTyped.value) return
  heroTyped.value.classList.add('typewriter')
  showNextTypewriterMessage()
}

onMounted(() => {
  // Initialize hero art animation
  if (heroImage.value) {
    // Ensure initial styles have applied before toggling
    requestAnimationFrame(() => {
      requestAnimationFrame(() => {
        heroImage.value.classList.add('enter')
      })
    })
  }

  // Initialize typewriter effect
  initTypewriter()
})

onUnmounted(() => {
  // Clean up timer when component is destroyed
  if (typewriterTimer) {
    clearInterval(typewriterTimer)
  }
})
</script>
