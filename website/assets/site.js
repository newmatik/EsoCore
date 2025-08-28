(function () {
  const ACCESS_KEY = 'es_access';
  const ACCESS_OK = 'yes';
  const ACCESS_CODE = '55619';

  function el(html) {
    const template = document.createElement('template');
    template.innerHTML = html.trim();
    return template.content.firstElementChild;
  }

  function isDocs() {
    const p = location.pathname.replace(/\/index.html$/, '');
    return /\/docs\/?$/.test(p) || /\/docs\//.test(p);
  }

  function header() {
    const here = isDocs() ? 'docs' : 'home';
    // Simplified path calculation to prevent infinite loops
    const isInDocs = isDocs();
    const homeHref = isInDocs ? '../' : './';
    const docsHref = isInDocs ? './' : './docs/';
    const githubHref = 'https://github.com/newmatik/edgesentinel-web';
    return el(`
      <header class="header">
        <div class="container navbar">
          <a class="brand" href="${homeHref}">
            <span>EdgeSentinel</span>
          </a>
          <nav class="nav" aria-label="Main Navigation">
            <a href="${homeHref}" ${here==='home' ? 'aria-current="page"' : ''}>Home</a>
            <a href="${docsHref}" ${here==='docs' ? 'aria-current="page"' : ''}>Docs</a>
            <a href="${githubHref}" target="_blank" rel="noopener">GitHub</a>
          </nav>
        </div>
      </header>
    `);
  }

  function footer() {
    const year = new Date().getFullYear();
    const isInDocs = isDocs();
    const docsHref = isInDocs ? './' : './docs/';
    const homeHref = isInDocs ? '../' : './';
    return el(`
      <footer class="footer">
        <div class="container">
          <div>© ${year} EdgeSentinel</div>
          <div>
            <a href="${docsHref}">Docs</a>
            &nbsp;•&nbsp;
            <a href="https://github.com/newmatik/edgesentinel-web" target="_blank" rel="noopener">Open Source</a>
            &nbsp;•&nbsp;
            <a href="${homeHref}imprint.html">Imprint</a>
          </div>
        </div>
      </footer>
    `);
  }

  function inject() {
    const headerMount = document.getElementById('site-header');
    const footerMount = document.getElementById('site-footer');
    if (headerMount) headerMount.replaceWith(header());
    if (footerMount) footerMount.replaceWith(footer());
  }

  function isUnlocked() { return sessionStorage.getItem(ACCESS_KEY) === ACCESS_OK; }

  function gate() {
    if (isUnlocked()) return;
    const path = location.pathname.replace(/\/index.html$/, '');
    if (path.endsWith('/imprint.html') || path.endsWith('/imprint')) return;
    const overlay = el(`
      <div class="gate" role="dialog" aria-modal="true" aria-label="Access Required">
        <div class="gate-card">
          <div class="gate-brand">
            <span class="brand-badge" aria-hidden="true"></span>
            <strong>EdgeSentinel</strong>
          </div>
          <h2>Enter Access Code</h2>
          <form class="gate-form">
            <input type="password" inputmode="numeric" autocomplete="off" placeholder="Access code" aria-label="Access code" />
            <button type="submit" class="btn">Unlock</button>
            <div class="gate-msg" aria-live="polite"></div>
          </form>
        </div>
      </div>
    `);
    const input = overlay.querySelector('input');
    const msg = overlay.querySelector('.gate-msg');
    overlay.querySelector('form').addEventListener('submit', (e) => {
      e.preventDefault();
      const val = (input.value || '').trim();
      if (val === ACCESS_CODE) {
        sessionStorage.setItem(ACCESS_KEY, ACCESS_OK);
        overlay.remove();
      } else {
        msg.textContent = 'Incorrect code. Please try again.';
      }
    });
    document.body.appendChild(overlay);
    setTimeout(() => input && input.focus(), 0);
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => { inject(); gate(); initTypewriter(); });
  } else {
    inject();
    gate();
    initTypewriter();
  }
})();

// Typewriter rotation for hero subtext
function initTypewriter(){
  const el = document.getElementById('hero-typed');
  if (!el) return;
  const messages = [
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
    'One platform, multiple partners.'
  ];
  let pool = [];
  let last = '';
  let timer;
  el.classList.add('typewriter');
  function showNext(){
    if (pool.length === 0) {
      pool = shuffle(messages.filter(m => m !== last));
    }
    const text = pool.shift();
    last = text;
    typeText(text, 0);
  }
  function typeText(text, i){
    el.textContent = text.slice(0, i) + '▌';
    if (i < text.length){
      setTimeout(() => typeText(text, i+1), 18);
    } else {
      // Blink cursor for remainder of 5s window
      const endAt = Date.now() + 3000;
      clearInterval(timer);
      timer = setInterval(() => {
        const cur = el.textContent.endsWith('▌');
        el.textContent = text + (cur ? ' ' : '▌');
        if (Date.now() > endAt){
          clearInterval(timer);
          showNext();
        }
      }, 420);
    }
  }
  showNext();
}

function shuffle(arr){
  const a = arr.slice();
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    const t = a[i]; a[i] = a[j]; a[j] = t;
  }
  return a;
}
