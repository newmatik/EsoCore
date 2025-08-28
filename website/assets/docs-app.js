(function () {
  const sidebarEl = document.getElementById('sidebar');
  const contentEl = document.getElementById('content');
  const searchInput = document.getElementById('search-input');
  const searchResults = document.getElementById('search-results');

  // Manifest of markdown files (relative to docs root)
  const docs = [
    'README.md',                          // Introduction
    'hardware-requirements.md',           // Hardware Requirements
    'edge-intelligence.md',               // Edge Intelligence
    'data-format-specification.md',       // Data Format Specification
    'api-specification.md',               // API Specification
    'testing-procedures.md',              // Testing & Validation
    'development-environment.md',         // Development Environment
    'development-timeline-budget.md',     // Development Timeline & Budget
    'business-model-and-partnerships.md', // Business Model & Partnerships
    'competitor-analysis.md',             // Competitor Analysis
    'fieldbus.md',                        // Industrial Fieldbus Protocols
    'bom.md',                             // Bill of Materials
    'license.md',                         // License
  ];

  const slugs = docs.map(name => name.replace(/\.md$/,''));
  const titlesCache = new Map();
  const textCache = new Map();
  const navItemMap = new Map(); // slug -> <li>

  const icons = {
    'README': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 19.5V6.5a2 2 0 0 1 2-2h0l4 2 4-2h0a2 2 0 0 1 2 2v13"/><path d="M6 20V6"/></svg>',
    'hardware-requirements': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06A1.65 1.65 0 0 0 15 19.4a1.65 1.65 0 0 0-1 .6 1.65 1.65 0 0 0-.33 1.82l-.06.06a2 2 0 1 1-2.83 0l-.06-.06A1.65 1.65 0 0 0 8.6 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.6 15a1.65 1.65 0 0 0-.6-1 1.65 1.65 0 0 0-1.82-.33l-.06-.06a2 2 0 1 1 0-2.83l.06-.06A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.6a1.65 1.65 0 0 0 1-.6 1.65 1.65 0 0 0 .33-1.82l.06-.06a2 2 0 1 1 2.83 0l.06.06A1.65 1.65 0 0 0 15 4.6a1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9c.24.49.24 1.06 0 1.56.24.49.24 1.06 0 1.56Z"/></svg>',
    'edge-intelligence': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/></svg>',
    'data-format-specification': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><ellipse cx="12" cy="5" rx="9" ry="3"/><path d="M21 5v6c0 1.66-4.03 3-9 3s-9-1.34-9-3V5"/><path d="M3 11v6c0 1.66 4.03 3 9 3s9-1.34 9-3v-6"/></svg>',
    'api-specification': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="8 4 4 8 8 12"/><polyline points="16 4 20 8 16 12"/><line x1="12" y1="16" x2="12" y2="20"/></svg>',
    'testing-procedures': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9 11l3 3L22 4"/><path d="M21 12v7a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11"/></svg>',
    'development-timeline-budget': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="18" rx="2"/><line x1="16" y1="2" x2="16" y2="6"/><line x1="8" y1="2" x2="8" y2="6"/><line x1="3" y1="10" x2="21" y2="10"/></svg>',
    'business-model-and-partnerships': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="7" width="20" height="14" rx="2"/><path d="M16 7V5a2 2 0 0 0-2-2H10a2 2 0 0 0-2 2v2"/></svg>',
    'competitor-analysis': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 3v18h18"/><rect x="7" y="12" width="3" height="6"/><rect x="12" y="9" width="3" height="9"/><rect x="17" y="5" width="3" height="13"/></svg>',
    'development-environment': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="4" width="18" height="14" rx="2"/><path d="M8 9l-3 3 3 3"/><path d="M13 15h5"/></svg>',
    'license': '<svg aria-hidden="true" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16l4-2 4 2 4-2 4 2V9z"/></svg>'
  };

  function titleFromName(name) {
    if (/^README\.md$/i.test(name)) return 'Introduction';
    return name
      .replace(/\-/g, ' ')
      .replace(/\.md$/, '')
      .replace(/\b\w/g, c => c.toUpperCase());
  }

  async function fetchMarkdown(path) {
    if (textCache.has(path)) return textCache.get(path);
    const res = await fetch(path + '?_=' + Date.now());
    if (!res.ok) throw new Error('Failed to fetch ' + path);
    const text = await res.text();
    textCache.set(path, text);
    return text;
  }

  async function getTitle(path) {
    if (titlesCache.has(path)) return titlesCache.get(path);
    const fileName = path.split('/').pop();
    // Force README.md to be labeled as "Introduction" in the menu and page title
    if (/^README\.md$/i.test(fileName)) {
      titlesCache.set(path, 'Introduction');
      return 'Introduction';
    }
    let title = titleFromName(fileName);
    try {
      const text = await fetchMarkdown(path);
      const m = text.match(/^#\s+(.+)$/m);
      if (m) title = m[1].trim();
    } catch (_) {}
    // Strip product name prefix in menu
    title = title.replace(/^\s*EdgeSentinel\s+/i, '').trim();
    titlesCache.set(path, title);
    return title;
  }

  function setActive(slug) {
    const links = sidebarEl.querySelectorAll('a');
    links.forEach(a => a.classList.toggle('active', a.dataset.slug === slug));
  }

  async function renderSidebar() {
    const list = document.createElement('ul');
    list.className = 'nav-list';
    for (const name of docs) {
      const slug = name.replace(/\.md$/, '');
      const path = './' + name;
      const title = await getTitle(path);
      const a = document.createElement('a');
      a.href = './' + slug;
      a.dataset.slug = slug;
      const iconKey = slug in icons ? slug : (slug === '' ? 'README' : 'README');
      const icon = icons[iconKey] || icons['README'];
      a.innerHTML = `<span class="icon" aria-hidden="true">${icon}</span><span class="label">${title}</span>`;
      a.addEventListener('click', (e) => {
        e.preventDefault();
        navigateTo(slug);
      });
      const li = document.createElement('li');
      li.appendChild(a);
      list.appendChild(li);
      navItemMap.set(slug, li);
    }
    sidebarEl.innerHTML = '';
    sidebarEl.appendChild(list);
  }

  async function renderDoc(slug) {
    const file = slug && slugs.includes(slug) ? slug + '.md' : docs[0];
    const path = './' + file;
    setActive(file.replace(/\.md$/, ''));
    try {
      const md = await fetchMarkdown(path);
      const html = marked.parse(md);
      contentEl.innerHTML = html;
      wireInternalLinks();
      await initWidgets();
      const pageTitle = await getTitle(path);
      document.title = pageTitle + ' – EdgeSentinel Docs';
      buildSectionMenu(slug);
      // Scroll to hash anchor if present (/docs/slug#section)
      const anchor = location.hash ? location.hash.substring(1) : '';
      if (anchor) {
        const target = document.getElementById(anchor);
        if (target) target.scrollIntoView({ behavior: 'smooth', block: 'start' });
      } else {
        window.scrollTo({ top: 0, behavior: 'smooth' });
      }
    } catch (err) {
      contentEl.innerHTML = '<h1>Not found</h1><p>Could not load this document.</p>';
    }
  }

  function wireInternalLinks() {
    const links = contentEl.querySelectorAll('a[href]');
    links.forEach(a => {
      const href = a.getAttribute('href');
      if (!href) return;
      if (href.startsWith('#')) return; // same-page anchors
      // Absolute or relative links to .md files within docs
      const isMd = /\.md(#[^\s#]+)?$/i.test(href);
      if (!isMd) return;
      a.addEventListener('click', e => {
        try {
          e.preventDefault();
          const url = new URL(href, location.origin + location.pathname.replace(/[^/]+$/,'') );
          const file = url.pathname.split('/').pop();
          const slug = file.replace(/\.md$/i, '');
          const anchor = url.hash || '';
          navigateTo(slug, anchor.replace('#',''));
        } catch (_) {
          // Fallback for plain relative href like 'file.md#anchor'
          const parts = href.split('#');
          const file = parts[0];
          const slug = file.replace(/.*\//, '').replace(/\.md$/i, '');
          const anchor = parts[1] ? '#' + parts[1] : '';
          navigateTo(slug, anchor.replace('#',''));
        }
      });
    });
  }

  async function initWidgets() {
    // Load BOM script if any BOM tables are present
    const bomMounts = contentEl.querySelectorAll('[id^="bom-"]');
    if (bomMounts.length > 0) {
      if (!window.EdgeBOM) {
        await loadScript('../assets/bom.js');
        console.log('BOM script loaded, EdgeBOM available:', !!window.EdgeBOM);
      }
      // Insert a structured System Cost section near the top (after the H1 if present)
      const systemNames = {
        'bom-main': 'Main Edge Device',
        'bom-vibration': 'Vibration Sensor Module',
        'bom-acoustic': 'Acoustic Sensor Module',
        'bom-current': 'Current Sensor Module',
        'bom-temperature': 'Temperature Sensor Module',
        'bom-cables': 'Cables & Interconnects'
      };
      const displayOrder = ['bom-main','bom-vibration','bom-acoustic','bom-current','bom-temperature','bom-cables'];
      let systemSection = document.getElementById('system-cost');
      if (!systemSection) {
        systemSection = document.createElement('section');
        systemSection.id = 'system-cost';
        const h2 = document.createElement('h2');
        h2.textContent = 'System Cost';
        const table = document.createElement('table');
        table.className = 'bom-table';
        table.innerHTML = `
          <thead><tr><th>Board / Module</th><th>Total $ (filtered)</th></tr></thead>
          <tbody></tbody>
          <tfoot><tr><th>Total</th><th class="grand-total">$0.00</th></tr></tfoot>
        `;
        systemSection.appendChild(h2);
        systemSection.appendChild(table);
        // Place before the first H2 (so it comes after the intro text),
        // fallback to right after H1 if no H2 exists yet.
        const firstH2 = contentEl.querySelector('h2');
        if (firstH2) {
          firstH2.parentNode.insertBefore(systemSection, firstH2);
        } else {
          const h1 = contentEl.querySelector('h1');
          if (h1 && h1.nextSibling) {
            h1.parentNode.insertBefore(systemSection, h1.nextSibling);
          } else {
            contentEl.prepend(systemSection);
          }
        }
      }
      const tableBody = systemSection.querySelector('tbody');
      const grandEl = systemSection.querySelector('.grand-total');
      const totals = {};
      function updateSummary(){
        // Ensure one row per known mount in display order
        tableBody.innerHTML = '';
        let grand = 0;
        for (const id of displayOrder) {
          if (!(id in systemNames)) continue;
          const val = totals[id] || 0;
          grand += val;
          const tr = document.createElement('tr');
          tr.innerHTML = `<td>${systemNames[id]}</td><td>$${val.toFixed(2)}</td>`;
          tableBody.appendChild(tr);
        }
        grandEl.textContent = `$${grand.toFixed(2)}`;
      }
      const onTotals = (e) => {
        const { mountId, total } = e.detail || {};
        if (!mountId) return;
        totals[mountId] = total;
        updateSummary();
      };
      window.addEventListener('EdgeBOM:totals', onTotals);
      // Render BOM tables programmatically (scripts inside injected markdown don't execute)
      const idToCsv = {
        'bom-main': './data/bom-main.csv',
        'bom-vibration': './data/bom-vibration-sensor.csv',
        'bom-acoustic': './data/bom-acoustic-sensor.csv',
        'bom-current': './data/bom-current-sensor.csv',
        'bom-temperature': './data/bom-temperature-sensor.csv',
        'bom-cables': './data/bom-cables.csv'
      };
      bomMounts.forEach(mount => {
        const id = mount.id;
        const csv = mount.getAttribute('data-csv') || idToCsv[id];
        if (!csv) return;
        try {
          window.EdgeBOM.renderBOM(id, csv);
        } catch (err) {
          console.error('Failed to render BOM', id, err);
          mount.innerHTML = '<p style="color: red;">Failed to load BOM.</p>';
        }
      });
      // Clean up listener when navigating away
      const disconnect = () => window.removeEventListener('EdgeBOM:totals', onTotals);
      window.addEventListener('popstate', disconnect, { once: true });
    }
  }

  function loadScript(src) {
    return new Promise((resolve, reject) => {
      const s = document.createElement('script');
      s.src = src;
      s.async = true;
      s.onload = () => resolve();
      s.onerror = () => reject(new Error('Failed to load '+src));
      document.head.appendChild(s);
    });
  }

  function buildSectionMenu(slug) {
    // Remove existing sub-lists
    sidebarEl.querySelectorAll('.nav-sublist').forEach(el => el.remove());
    const li = navItemMap.get(slug);
    if (!li) return;
    // Prefer multiple h1 sections; if only one, fallback to h2 sections
    let headings = Array.from(contentEl.querySelectorAll('h1'));
    if (headings.length <= 1) headings = Array.from(contentEl.querySelectorAll('h2'));
    if (headings.length === 0) return;
    const ul = document.createElement('ul');
    ul.className = 'nav-sublist';
    headings.forEach(h => {
      const text = (h.textContent || '').trim();
      let id = h.id;
      if (!id) {
        id = text.toLowerCase().replace(/[^a-z0-9\s-]/g,'').replace(/\s+/g,'-').replace(/-+/g,'-');
        h.id = id;
      }
      const a = document.createElement('a');
      a.href = './' + slug + '#' + id;
      a.textContent = text;
      a.addEventListener('click', (e) => {
        e.preventDefault();
        // Keep the current slug; just update anchor and smooth scroll
        history.pushState({}, '', './' + slug + '#' + id);
        const target = document.getElementById(id);
        if (target) target.scrollIntoView({ behavior: 'smooth', block: 'start' });
      });
      const item = document.createElement('li');
      item.appendChild(a);
      ul.appendChild(item);
    });
    li.appendChild(ul);
  }

  function currentSlug() {
    // Read from clean path <docs-root>/<slug>; default to README if missing
    const path = location.pathname.replace(/\/index.html$/, '');
    const base = path.replace(/\/[^/]*$/, '/');
    const rest = path.slice(base.length);
    const slug = rest.split('/').filter(Boolean)[0];
    return slug || slugs[0];
  }

  async function buildSearchIndex() {
    const items = [];
    for (const name of docs) {
      const slug = name.replace(/\.md$/, '');
      const path = './' + name;
      try {
        const text = await fetchMarkdown(path);
        const title = await getTitle(path);
        items.push({ slug, title, text });
      } catch (_) {}
    }
    return items;
  }

  function highlight(text, q) {
    const re = new RegExp(q.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'ig');
    return text.replace(re, m => '<mark>' + m + '</mark>');
  }

  function snippetAround(text, q, len = 120) {
    const idx = text.toLowerCase().indexOf(q.toLowerCase());
    if (idx < 0) return text.slice(0, len) + (text.length > len ? '…' : '');
    const start = Math.max(0, idx - Math.floor(len/2));
    const end = Math.min(text.length, start + len);
    return (start>0?'…':'') + text.slice(start, end) + (end<text.length?'…':'');
  }

  async function enableSearch() {
    const index = await buildSearchIndex();
    function onInput() {
      const q = searchInput.value.trim();
      if (!q) { searchResults.hidden = true; searchResults.innerHTML = ''; return; }
      const matches = index
        .flatMap(item => {
          const lines = item.text.split(/\n+/);
          const hit = lines.find(line => line.toLowerCase().includes(q.toLowerCase()));
          if (!hit) return [];
          return [{ slug: item.slug, title: item.title, snippet: snippetAround(hit, q) }];
        })
        .slice(0, 8);
      renderResults(matches, q);
    }
    searchInput.addEventListener('input', onInput);
  }

  function renderResults(items, q) {
    searchResults.hidden = items.length === 0;
    searchResults.innerHTML = items.map(i => (
      `<a class="result" href="./${i.slug}"><strong>${i.title}</strong><br><small>${highlight(i.snippet, q)}</small></a>`
    )).join('');
    // Wire results to internal router
    Array.from(searchResults.querySelectorAll('a.result')).forEach(a => {
      a.addEventListener('click', (e) => {
        e.preventDefault();
        try {
          const href = a.getAttribute('href') || '';
          const slug = href.split('/').filter(Boolean).pop();
          if (slug) navigateTo(slug);
          searchResults.hidden = true;
        } catch (_) {}
      });
    });
  }

  function navigateTo(slug, anchor) {
    const hash = anchor ? '#' + anchor : location.hash;
    history.pushState({}, '', './' + slug + (hash || ''));
    renderDoc(slug);
  }

  window.addEventListener('popstate', () => {
    renderDoc(currentSlug());
  });

  (async function init() {
    // Support GitHub Pages fallback: 404.html redirects to ./?p=<slug>[#anchor]
    const params = new URLSearchParams(location.search);
    const p = params.get('p');
    if (p) {
      const parts = p.split('#');
      const file = parts[0].replace(/.*\//, '');
      const slug = file.replace(/\.md$/i, '');
      const anchor = parts[1] || '';
      history.replaceState({}, '', './' + slug + (anchor ? '#' + anchor : ''));
    }
    await renderSidebar();
    await enableSearch();
    await renderDoc(currentSlug());
  })();
})();
