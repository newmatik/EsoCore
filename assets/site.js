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
    return p === '/docs' || p.startsWith('/docs/');
  }

  function header() {
    const here = isDocs() ? 'docs' : 'home';
    const homeHref = '/';
    const docsHref = '/docs/';
    const githubHref = 'https://github.com/newmatik/edgesentinel-web';
    return el(`
      <header class="header">
        <div class="container navbar">
          <a class="brand" href="${homeHref}">
            <span class="brand-badge" aria-hidden="true"></span>
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
    return el(`
      <footer class="footer">
        <div class="container">
          <div>© ${year} EdgeSentinel</div>
          <div>
            <a href="/docs/">Docs</a>
            &nbsp;•&nbsp;
            <a href="https://github.com/newmatik/edgesentinel-web" target="_blank" rel="noopener">Open Source</a>
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
    document.addEventListener('DOMContentLoaded', () => { inject(); gate(); });
  } else {
    inject();
    gate();
  }
})();
