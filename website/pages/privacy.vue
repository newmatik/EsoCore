<template>
  <main class="container" style="padding-top: 84px">
    <article class="doc-content">
      <h1>Privacy</h1>
      <p>
        This is the privacy notice (Datenschutzerklärung) for www.esocore.com. It covers this
        website only. The EsoCore portal and the Newmatik company site at newmatik.com are separate
        services with their own notices.
      </p>

      <section>
        <h2>Who is responsible</h2>
        <p>
          Newmatik GmbH, Am Markt 1, 55619 Hennweiler, Germany, represented by its managing
          directors Dominik Ottenbreit and Björn Ottenbreit. Privacy questions go to
          <a href="mailto:service@newmatik.com">service@newmatik.com</a> or +49 6752 16998-0.
        </p>
      </section>

      <section>
        <h2>Opening a page</h2>
        <p>
          The site is hosted on GitHub Pages (GitHub, Inc., 88 Colin P. Kelly Jr. Street, San
          Francisco, CA 94107, USA) and delivered through Cloudflare, Inc. (101 Townsend Street, San
          Francisco, CA 94107, USA). Delivering a page creates a connection record: IP address, date
          and time, the address you requested and the browser’s user agent. The providers process
          that record to deliver the page and to keep the site available and secure. We do not
          receive a visitor profile from it and do not use it for advertising or analytics.
        </p>
        <p>
          The legal basis is our legitimate interest in running a site that loads and stays up (Art.
          6 (1) (f) GDPR). You can object; if you do, we stop unless we can show compelling grounds
          that override your interest.
        </p>
        <p>
          Connection data can be processed in the United States. GitHub, Inc. and Cloudflare, Inc.
          are certified under the EU-US Data Privacy Framework, so that transfer rests on the
          European Commission’s adequacy decision.
        </p>
        <p>
          This site sets no cookies and loads no analytics, advertising or fonts from other servers.
          Links to GitHub only contact GitHub when you follow them.
        </p>
      </section>

      <section>
        <h2>Email</h2>
        <p>
          If you write to us, we use your address and message to reply. The basis is Art. 6 (1) (b)
          GDPR when the message concerns a contract we are discussing, and Art. 6 (1) (f) GDPR
          otherwise. We keep the thread while that work is open, and longer only where commercial or
          tax law requires us to keep business correspondence.
        </p>
      </section>

      <section>
        <h2>Your rights</h2>
        <p>
          You can ask for access, correction, deletion, restriction or a copy of personal data we
          hold about you, and you can object to processing based on legitimate interest. You can
          also complain to the supervisory authority for our seat: Der Landesbeauftragte für den
          Datenschutz und die Informationsfreiheit Rheinland-Pfalz, Hintere Bleiche 34, 55116 Mainz,
          Germany,
          <a href="https://www.datenschutz.rlp.de" target="_blank" rel="noopener noreferrer"
            >datenschutz.rlp.de</a
          >.
        </p>
      </section>

      <p><NuxtLink to="/imprint">Imprint</NuxtLink></p>
    </article>
  </main>
</template>

<script setup>
useHead({
  title: 'Privacy',
  meta: [
    {
      name: 'description',
      content: 'Privacy notice for www.esocore.com, provided by Newmatik GmbH.',
    },
  ],
})
</script>
