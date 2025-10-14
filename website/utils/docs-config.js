// CRITICAL: EsoCore Documentation Configuration
// This file contains the complete documentation structure and routing configuration.
// 
// IMPORTANT: This configuration works in conjunction with:
// - nuxt.config.ts (static route generation)
// - server/plugins/trailing-slash-redirect.ts (runtime redirects)
// - public/_redirects (GitHub Pages redirects)
// - pages/docs/[...slug].vue (dynamic routing)
//
// DO NOT MODIFY the slug values without updating all related routing files.
// The slug values must match the actual markdown file names in the content directory.

// Icon definitions - reusable SVG icons
const icons = {
  introduction: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 19.5V6.5a2 2 0 0 1 2-2h0l4 2 4-2h0a2 2 0 0 1 2 2v13"/><path d="M6 20V6"/></svg>',
  hardware: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06A1.65 1.65 0 0 0 15 19.4a1.65 1.65 0 0 0-1 .6 1.65 1.65 0 0 0-.33 1.82l-.06.06a2 2 0 1 1-2.83 0l-.06-.06A1.65 1.65 0 0 0 8.6 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.6 15a1.65 1.65 0 0 0-.6-1 1.65 1.65 0 0 0-1.82-.33l-.06-.06a2 2 0 1 1 0-2.83l.06-.06A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.6a1.65 1.65 0 0 0 1-.6 1.65 1.65 0 0 0 .33-1.82l.06-.06a2 2 0 1 1 2.83 0l.06.06A1.65 1.65 0 0 0 15 4.6a1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9c.24.49.24 1.06 0 1.56.24.49.24 1.06 0 1.56Z"/></svg>',
  sensor: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2v6m0 0l-3-3m3 3 3-3"/><path d="M12 8v6m0 0l-3 3m3-3 3 3"/><path d="M8 12h8"/><circle cx="12" cy="12" r="3"/></svg>',
  sensorSmall: '<svg aria-hidden="true" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2v6m0 0l-3-3m3 3 3-3"/><path d="M12 8v6m0 0l-3 3m3-3 3 3"/><path d="M8 12h8"/><circle cx="12" cy="12" r="3"/></svg>',
  overview: '<svg aria-hidden="true" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3"/><path d="M12 17h.01"/></svg>',
  intelligence: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/></svg>',
  database: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><ellipse cx="12" cy="5" rx="9" ry="3"/><path d="M21 5v6c0 1.66-4.03 3-9 3s-9-1.34-9-3V5"/><path d="M3 11v6c0 1.66 4.03 3 9 3s9-1.34 9-3v-6"/></svg>',
  api: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="8 4 4 8 8 12"/><polyline points="16 4 20 8 16 12"/><line x1="12" y1="16" x2="12" y2="20"/></svg>',
  cloud: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M18 10h-1.26A8 8 0 1 0 9 20h9a5 5 0 0 0 0-10z"/><path d="M12 2v6m0 0l-3-3m3 3 3-3"/></svg>',
  check: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9 11l3 3L22 4"/><path d="M21 12v7a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11"/></svg>',
  terminal: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="14" rx="2"/><path d="M8 9l-3 3 3 3"/><path d="M13 15h5"/></svg>',
  business: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="7" width="20" height="14" rx="2"/><path d="M16 7V5a2 2 0 0 0-2-2H10a2 2 0 0 0-2 2v2"/></svg>',
  chart: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 3v18h18"/><rect x="7" y="12" width="3" height="6"/><rect x="12" y="9" width="3" height="9"/><rect x="17" y="5" width="3" height="13"/></svg>',
  document: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16l4-2 4 2 4-2 4 2V9z"/></svg>',
  grid: '<svg aria-hidden="true" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/></svg>',
  solutions: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2l3.09 6.26L22 9.27l-5 4.87 1.18 6.88L12 17.77l-6.18 3.25L7 14.14 2 9.27l6.91-1.01L12 2z"/></svg>',
  industry: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="20" width="20" height="2"/><path d="M5 20V7l5-5v18"/><path d="M10 20V2l5 5v13"/><path d="M15 20V7l4 4v9"/></svg>',
  compare: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M12 3v18"/><path d="M3 12h18"/></svg>',
  guide: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 19.5A2.5 2.5 0 0 1 6.5 17H20"/><path d="M6.5 2H20v20H6.5A2.5 2.5 0 0 1 4 19.5v-15A2.5 2.5 0 0 1 6.5 2z"/></svg>',
  start: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="5 3 19 12 5 21 5 3"/></svg>',
  money: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="12" y1="1" x2="12" y2="23"/><path d="M17 5H9.5a3.5 3.5 0 0 0 0 7h5a3.5 3.5 0 0 1 0 7H6"/></svg>',
  faq: '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3"/><path d="M12 17h.01"/></svg>',
  smallDoc: '<svg aria-hidden="true" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/><polyline points="10 9 9 9 8 9"/></svg>',
}

// Shared docs configuration
export const docsConfig = [
  {
    slug: '',
    title: 'Introduction',
    icon: icons.introduction,
  },
  {
    slug: 'faq',
    title: 'FAQ',
    icon: icons.faq,
  },
  {
    key: 'getting-started',
    title: 'Getting Started',
    icon: icons.start,
    children: [
      { slug: 'getting-started/quick-start', title: 'Quick Start Guide', icon: icons.smallDoc },
      { slug: 'getting-started/installation-guide', title: 'Installation Guide', icon: icons.smallDoc },
      { slug: 'getting-started/integration-guide', title: 'Integration Guide', icon: icons.smallDoc },
    ],
  },
  {
    key: 'solutions',
    title: 'Solutions',
    icon: icons.solutions,
    children: [
      { slug: 'solutions/predictive-maintenance', title: 'Predictive Maintenance', icon: icons.smallDoc },
      { slug: 'solutions/equipment-downtime-prevention', title: 'Downtime Prevention', icon: icons.smallDoc },
    ],
  },
  {
    key: 'industries',
    title: 'Industries',
    icon: icons.industry,
    children: [
      { slug: 'industries/cnc-machine-monitoring', title: 'CNC Machine Monitoring', icon: icons.smallDoc },
      { slug: 'industries/industrial-door-monitoring', title: 'Industrial Door Monitoring', icon: icons.smallDoc },
      { slug: 'industries/injection-molding-monitoring', title: 'Injection Molding', icon: icons.smallDoc },
      { slug: 'industries/conveyor-monitoring', title: 'Conveyor Systems', icon: icons.smallDoc },
      { slug: 'industries/pump-compressor-monitoring', title: 'Pumps & Compressors', icon: icons.smallDoc },
    ],
  },
  {
    key: 'comparisons',
    title: 'Comparisons',
    icon: icons.compare,
    children: [
      { slug: 'comparisons/open-source-vs-proprietary', title: 'Open Source vs Proprietary', icon: icons.smallDoc },
      { slug: 'comparisons/edge-ai-vs-cloud-monitoring', title: 'Edge AI vs Cloud', icon: icons.smallDoc },
    ],
  },
  {
    key: 'guides',
    title: 'Guides',
    icon: icons.guide,
    children: [
      { slug: 'guides/what-is-predictive-maintenance', title: 'What is Predictive Maintenance?', icon: icons.smallDoc },
      { slug: 'guides/choosing-iot-platform', title: 'Choosing an IoT Platform', icon: icons.smallDoc },
    ],
  },
  {
    key: 'roi',
    title: 'ROI & Business Case',
    icon: icons.money,
    children: [
      { slug: 'roi/cost-savings-calculator', title: 'Cost Savings Calculator', icon: icons.smallDoc },
      { slug: 'roi/total-cost-ownership', title: 'Total Cost of Ownership', icon: icons.smallDoc },
      { slug: 'roi/business-case-predictive-maintenance', title: 'Business Case Guide', icon: icons.smallDoc },
    ],
  },
  {
    slug: 'esocore-edge',
    title: 'Hardware: Edge',
    icon: icons.hardware,
  },
  {
    key: 'sensors',
    slug: 'esocore-sensors',
    title: 'Hardware: Sensors',
    icon: icons.sensor,
    children: [
      { slug: 'esocore-sensors', title: 'Overview', icon: icons.overview },
      { slug: 'sensors/vibration-sensor', title: 'Vibration Sensor', icon: icons.sensorSmall },
      { slug: 'sensors/acoustic-sensor', title: 'Acoustic Sensor', icon: icons.sensorSmall },
      { slug: 'sensors/current-sensor', title: 'Current Sensor', icon: icons.sensorSmall },
      { slug: 'sensors/air-quality-sensor', title: 'Air Quality Sensor', icon: icons.sensorSmall },
      { slug: 'sensors/light-sensor', title: 'Light Sensor', icon: icons.sensorSmall },
      { slug: 'sensors/oil-quality-sensor', title: 'Oil Quality Sensor', icon: icons.sensorSmall },
      { slug: 'sensors/pressure-sensor', title: 'Pressure Sensor', icon: icons.sensorSmall },
      { slug: 'sensors/temperature-spot-sensor', title: 'Temperature Spot Sensor', icon: icons.sensorSmall },
      { slug: 'sensors/proximity-position-sensor', title: 'Proximity Position Sensor', icon: icons.sensorSmall },
    ],
  },
  {
    slug: 'edge-intelligence',
    title: 'Edge Intelligence',
    icon: icons.intelligence,
  },
  {
    slug: 'data-format-specification',
    title: 'Data Format Specification',
    icon: icons.database,
  },
  {
    slug: 'api-specification',
    title: 'API Specification',
    icon: icons.api,
  },
  {
    slug: 'cloud-infrastructure',
    title: 'Cloud Infrastructure',
    icon: icons.cloud,
  },
  {
    slug: 'testing-procedures',
    title: 'Testing & Validation',
    icon: icons.check,
  },
  {
    slug: 'development-environment',
    title: 'Development Environment',
    icon: icons.terminal,
  },
  {
    slug: 'business-model-and-partnerships',
    title: 'Business Model & Partnerships',
    icon: icons.business,
  },
  {
    slug: 'competitor-analysis',
    title: 'Competitor Analysis',
    icon: icons.chart,
  },
  {
    slug: 'fieldbus',
    title: 'Industrial Fieldbus Protocols',
    icon: icons.terminal,
  },
  {
    key: 'bom',
    slug: 'bom',
    title: 'Bill of Materials',
    icon: icons.document,
    children: [
      { slug: 'bom/edge', title: 'Edge', icon: icons.grid },
      { slug: 'bom/vibration-sensor', title: 'Vibration Sensor', icon: icons.sensorSmall },
      { slug: 'bom/acoustic-sensor', title: 'Acoustic Sensor', icon: icons.sensorSmall },
      { slug: 'bom/current-sensor', title: 'Current Sensor', icon: icons.sensorSmall },
      { slug: 'bom/air-quality-sensor', title: 'Air Quality Sensor', icon: icons.sensorSmall },
      { slug: 'bom/light-sensor', title: 'Light Sensor', icon: icons.sensorSmall },
      { slug: 'bom/oil-quality-sensor', title: 'Oil Quality Sensor', icon: icons.sensorSmall },
      { slug: 'bom/pressure-sensor', title: 'Pressure Sensor', icon: icons.sensorSmall },
      { slug: 'bom/temperature-spot-sensor', title: 'Temperature Spot Sensor', icon: icons.sensorSmall },
      { slug: 'bom/proximity-position-sensor', title: 'Proximity Position Sensor', icon: icons.sensorSmall },
      { slug: 'bom/cables', title: 'Cables & Interconnects', icon: icons.grid }
    ]
  },
  {
    slug: 'license',
    title: 'License',
    icon: icons.document,
  },
]

// Helper functions for sidebar navigation and routing
// CRITICAL: These functions determine which navigation groups are expanded
// and what links are generated. They must work consistently across all
// documentation pages to prevent navigation inconsistencies.
export function isGroupExpanded(group, currentSlug) {
  if (!group.children) return false
  
  // Use stable key-based matching for groups
  if (group.key === 'sensors') {
    return currentSlug === 'esocore-sensors' || currentSlug.startsWith('sensors/')
  }
  
  if (group.key === 'bom') {
    return currentSlug === 'bom' || currentSlug.startsWith('bom/')
  }
  
  if (group.key === 'getting-started') {
    return currentSlug.startsWith('getting-started/')
  }
  
  if (group.key === 'solutions') {
    return currentSlug.startsWith('solutions/')
  }
  
  if (group.key === 'industries') {
    return currentSlug.startsWith('industries/')
  }
  
  if (group.key === 'comparisons') {
    return currentSlug.startsWith('comparisons/')
  }
  
  if (group.key === 'guides') {
    return currentSlug.startsWith('guides/')
  }
  
  if (group.key === 'roi') {
    return currentSlug.startsWith('roi/')
  }
  
  // Fallback to slug-based matching
  if (group.slug) {
    return currentSlug === group.slug || currentSlug.startsWith(group.slug + '/')
  }
  
  return false
}

export function getGroupHeaderLink(group) {
  if (group.slug) {
    return `/docs/${group.slug}`
  }
  return '/docs'
}
