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
          <img ref="heroImage" src="/pcb.png" alt="EsoCore Edge industrial IoT circuit board for predictive maintenance" />
        </div>
      </div>
    </section>

    <section class="features">
      <div class="container">
        <h2 class="sr-only">Key Features</h2>
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
            <p>Plug‑and‑play sensor modules enable fast integration of new or existing sensors.</p>
          </div>
          <div class="panel feature">
            <div class="kicker">Modern Development Framework</div>
            <h3>C, Python, Node</h3>
            <p>Modern development frameworks for fast, secure, and reliable code.</p>
          </div>
          <div class="panel feature">
            <div class="kicker">Customizable</div>
            <h3>STEP, Altium Designer, Fusion 360</h3>
            <p>Modify hardware or PCBs and customize to your needs.</p>
          </div>
        </div>
      </div>
    </section>

    <section class="content-section">
      <div class="container">
        <h2>What is EsoCore?</h2>
        <p>
          EsoCore is an open-source <strong>industrial IoT platform</strong> designed for comprehensive equipment monitoring and
          <strong>predictive maintenance</strong> in manufacturing environments. This advanced <strong>condition monitoring system</strong>
          combines edge computing with machine learning to deliver real-time insights into machine health, preventing costly downtime and
          optimizing maintenance schedules.
        </p>
        <p>
          As a complete <strong>industrial equipment monitoring</strong> solution, EsoCore provides everything needed for modern factory
          automation: modular sensor modules for vibration analysis, temperature monitoring, acoustic sensing, and more. The platform
          processes data at the edge using TinyML algorithms, enabling ultra-low latency anomaly detection without relying on cloud
          connectivity. This makes it ideal for mission-critical manufacturing applications where reliability and responsiveness are paramount.
        </p>
        <p>
          Unlike proprietary industrial IoT solutions, EsoCore is fully open source under the Apache 2.0 License, giving you complete control
          over your monitoring infrastructure. You can deploy it on-premises, customize it for your specific needs, and integrate it with
          existing factory systems including Modbus, PROFIBUS, and PROFINET networks. Whether you are monitoring CNC machines, industrial
          doors, injection molding equipment, or conveyor systems, EsoCore provides the flexibility and reliability your operations demand.
        </p>
      </div>
    </section>

    <section class="content-section alt">
      <div class="container">
        <h2>Who Uses EsoCore?</h2>
        <p>
          EsoCore serves manufacturing facilities and industrial operations that require reliable, real-time equipment monitoring and
          predictive maintenance capabilities. Our <strong>industrial IoT platform</strong> is deployed across diverse applications:
        </p>
        <div class="use-cases">
          <div class="use-case">
            <h3>CNC Machine Monitoring</h3>
            <p>
              Monitor machining centers, lathes, and milling machines with comprehensive vibration analysis, spindle temperature monitoring,
              and power consumption tracking. Detect bearing wear, tool degradation, and mechanical anomalies before they cause costly
              downtime or quality issues.
            </p>
            <NuxtLink to="/docs/industries/cnc-machine-monitoring" class="use-case-link">Learn more →</NuxtLink>
          </div>
          <div class="use-case">
            <h3>Industrial Door Systems</h3>
            <p>
              Track cycle counts, spring health, motor current, and operational state for sectional doors, high-speed roll-up doors, and
              automated gate systems. Optimize maintenance intervals and prevent unexpected failures with data-driven condition
              monitoring.
            </p>
            <NuxtLink to="/docs/industries/industrial-door-monitoring" class="use-case-link">Learn more →</NuxtLink>
          </div>
          <div class="use-case">
            <h3>Manufacturing Equipment</h3>
            <p>
              Monitor injection molding machines, packaging lines, conveyor systems, and process equipment with customizable sensor
              configurations. Track operational parameters, detect anomalies, and optimize production efficiency across your entire facility.
            </p>
            <NuxtLink to="/docs/industries/injection-molding-monitoring" class="use-case-link">Learn more →</NuxtLink>
          </div>
          <div class="use-case">
            <h3>Support Infrastructure</h3>
            <p>
              Maintain critical support equipment including industrial pumps, compressors, motors, and HVAC systems with continuous condition
              monitoring. Predict failures before they occur and extend equipment lifespan through data-driven maintenance strategies.
            </p>
            <NuxtLink to="/docs/industries/pump-compressor-monitoring" class="use-case-link">Learn more →</NuxtLink>
          </div>
        </div>
      </div>
    </section>

    <section class="content-section">
      <div class="container">
        <h2>Why Choose EsoCore for Industrial Equipment Monitoring?</h2>
        <p>
          EsoCore differentiates itself from proprietary industrial IoT platforms through its unique combination of open-source flexibility,
          edge intelligence, and comprehensive safety integration. Here is what makes our <strong>predictive maintenance system</strong>
          the preferred choice for forward-thinking manufacturers:
        </p>
        <div class="benefits-grid">
          <div class="benefit">
            <h3>Edge-First Architecture</h3>
            <p>
              Process sensor data directly on industrial-grade edge devices using TinyML models. Achieve sub-second anomaly detection without
              cloud dependency, ensuring your critical equipment monitoring continues even during network outages. Reduce bandwidth costs by
              90% through intelligent data filtering and prioritization.
            </p>
          </div>
          <div class="benefit">
            <h3>Open Source Freedom</h3>
            <p>
              No vendor lock-in, no hidden costs, no proprietary restrictions. The entire platform—hardware designs, firmware, and cloud
              components—is available under the Apache 2.0 License. Customize it for your specific needs, deploy it anywhere, and maintain
              complete control over your industrial data.
            </p>
          </div>
          <div class="benefit">
            <h3>Modular Sensor System</h3>
            <p>
              Add new sensor types without redesigning your infrastructure. Our RS-485 sensor bus supports hot-plug sensor modules for
              vibration, acoustics, temperature, pressure, oil quality, and more. Scale from single-machine monitoring to facility-wide
              deployments with ease.
            </p>
          </div>
          <div class="benefit">
            <h3>Safety Compliance Built-In</h3>
            <p>
              6 dual-channel safety digital inputs with cross-monitoring (EN ISO 13849 Cat. 3) for e-stops, light curtains, and safety edges.
              Built-in safety relay (dual-channel, 2NO+2NC) and SSR output for interlocking. Combined with 4 dual-mode analog inputs, EsoCore
              provides comprehensive safety-rated I/O for industrial monitoring and interlocking applications.
            </p>
          </div>
          <div class="benefit">
            <h3>Industrial Fieldbus Integration</h3>
            <p>
              Connect seamlessly to existing factory automation systems via Modbus RTU/TCP, PROFIBUS DP, and PROFINET protocols. EsoCore
              speaks the language of industrial control systems, enabling smooth integration without disrupting production workflows.
            </p>
          </div>
          <div class="benefit">
            <h3>Proven Technology Stack</h3>
            <p>
              Built on industrial-proven components: STM32 microcontrollers, Django REST Framework backend, and modern web technologies.
              Comprehensive documentation, reference designs, and professional support ensure successful deployments from pilot to
              production scale.
            </p>
          </div>
        </div>
        <div class="resources-grid">
          <div class="resource-card">
            <h4>Get Started</h4>
            <ul>
              <li><NuxtLink to="/docs/getting-started/quick-start">Quick Start Guide</NuxtLink></li>
              <li><NuxtLink to="/docs/getting-started/installation-guide">Installation Guide</NuxtLink></li>
              <li><NuxtLink to="/docs/getting-started/integration-guide">Integration Guide</NuxtLink></li>
            </ul>
          </div>
          <div class="resource-card">
            <h4>Solutions</h4>
            <ul>
              <li><NuxtLink to="/docs/solutions/predictive-maintenance">Predictive Maintenance</NuxtLink></li>
              <li><NuxtLink to="/docs/solutions/equipment-downtime-prevention">Downtime Prevention</NuxtLink></li>
            </ul>
          </div>
          <div class="resource-card">
            <h4>Compare & Choose</h4>
            <ul>
              <li><NuxtLink to="/docs/comparisons/open-source-vs-proprietary">Open Source vs Proprietary</NuxtLink></li>
              <li><NuxtLink to="/docs/comparisons/edge-ai-vs-cloud-monitoring">Edge AI vs Cloud</NuxtLink></li>
            </ul>
          </div>
        </div>
        <div class="cta-section">
          <h3>Ready to Transform Your Equipment Monitoring?</h3>
          <p>
            Explore our comprehensive documentation to learn how EsoCore can reduce downtime, optimize maintenance, and provide complete
            visibility into your industrial operations.
          </p>
          <div class="cta">
            <NuxtLink to="/docs" class="btn">Explore Documentation</NuxtLink>
            <NuxtLink to="/docs/faq" class="btn secondary">View FAQ</NuxtLink>
          </div>
        </div>
      </div>
    </section>
  </main>
</template>

<script setup>
// Set page meta with enhanced SEO
useHead({
  title: 'EsoCore - Open Source Industrial IoT Platform for Predictive Maintenance',
  meta: [
    {
      name: 'description',
      content:
        'Open-source industrial IoT platform with edge AI for equipment monitoring, predictive maintenance, and condition monitoring. Supports CNC machines, industrial doors, and manufacturing equipment.',
    },
    {
      property: 'og:title',
      content: 'EsoCore - Open Source Industrial IoT Platform for Predictive Maintenance',
    },
    {
      property: 'og:description',
      content:
        'Open-source industrial IoT platform with edge AI for equipment monitoring, predictive maintenance, and condition monitoring.',
    },
    {
      property: 'og:type',
      content: 'website',
    },
    {
      name: 'twitter:card',
      content: 'summary_large_image',
    },
    {
      name: 'twitter:title',
      content: 'EsoCore - Open Source Industrial IoT Platform',
    },
    {
      name: 'twitter:description',
      content:
        'Open-source industrial IoT platform with edge AI for predictive maintenance and equipment monitoring.',
    },
    {
      name: 'keywords',
      content:
        'industrial IoT platform, predictive maintenance system, industrial equipment monitoring, factory machine monitoring, open source IIoT, edge computing, condition monitoring, CNC machine monitoring, manufacturing IoT',
    },
  ],
  script: [
    {
      type: 'application/ld+json',
      children: JSON.stringify({
        '@context': 'https://schema.org',
        '@type': 'SoftwareApplication',
        name: 'EsoCore',
        applicationCategory: 'Industrial IoT Platform',
        description:
          'Open-source industrial IoT platform with edge AI for equipment monitoring, predictive maintenance, and condition monitoring.',
        operatingSystem: 'Linux, Embedded Systems',
        offers: {
          '@type': 'Offer',
          price: '0',
          priceCurrency: 'USD',
        },
        author: {
          '@type': 'Organization',
          name: 'Newmatik',
          url: 'https://newmatik.com',
        },
        license: 'https://www.apache.org/licenses/LICENSE-2.0',
        softwareVersion: '1.0',
        codeRepository: 'https://github.com/newmatik/EsoCore',
      }),
    },
    {
      type: 'application/ld+json',
      children: JSON.stringify({
        '@context': 'https://schema.org',
        '@type': 'Organization',
        name: 'EsoCore',
        url: 'https://www.esocore.com',
        logo: 'https://www.esocore.com/icon.png',
        description:
          'Open-source industrial IoT platform for predictive maintenance and equipment monitoring',
        sameAs: ['https://github.com/newmatik/EsoCore'],
      }),
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
