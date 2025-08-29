(function () {
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
    const githubHref = 'https://github.com/newmatik/EsoCore';
    return el(`
      <header class="header">
        <div class="container navbar">
          <a class="brand" href="${homeHref}">
            <img src="${isInDocs ? '../' : './'}icon.svg" alt="EsoCore" width="28" height="28" />
            <span>EsoCore</span>
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
          <div>© ${year} EsoCore</div>
          <div>
            <a href="${docsHref}">Docs</a>
            &nbsp;•&nbsp;
            <a href="https://github.com/newmatik/EsoCore" target="_blank" rel="noopener">Open Source</a>
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

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => { inject(); initTypewriter(); initHeroArtAnimation(); });
  } else {
    inject();
    initTypewriter();
    initHeroArtAnimation();
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

// Slide-in + fade-in animation for hero PCB image on homepage
function initHeroArtAnimation(){
  // Only on homepage
  const onDocs = (function(){
    const p = location.pathname.replace(/\/index.html$/, '');
    return /\/docs\/?$/.test(p) || /\/docs\//.test(p);
  })();
  if (onDocs) return;
  const img = document.querySelector('.hero-art img');
  if (!img) return;
  // Ensure initial styles have applied before toggling
  requestAnimationFrame(() => requestAnimationFrame(() => img.classList.add('enter')));
}
