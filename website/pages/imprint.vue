<template>
  <main class="container" style="padding-top: 84px">
    <article class="doc-content">
      <h1>Imprint</h1>
      <p>Information pursuant to § 5 Digitale-Dienste-Gesetz (DDG).</p>

      <section>
        <h2>Company Information</h2>
        <div class="contact-info">
          <strong>Newmatik GmbH</strong><br />
          Am Markt 1<br />
          55619 Hennweiler<br />
          Germany
        </div>
      </section>

      <section>
        <h2>Contact</h2>
        <div class="contact-info">
          <strong>Phone:</strong> <a href="tel:+49675216998-0">+49 6752 16998-0</a><br />
          <strong>Email:</strong> <a href="mailto:service@newmatik.com">service@newmatik.com</a
          ><br />
          <strong>Website:</strong> <a href="https://www.newmatik.com">www.newmatik.com</a>
        </div>
      </section>

      <section>
        <h2>Management</h2>
        <div class="contact-info">
          <strong>Managing Directors:</strong> Dominik Ottenbreit, Björn Ottenbreit<br />
          <strong>Authorized Signatory:</strong> Ingrid Ottenbreit
        </div>
      </section>

      <section>
        <h2>Commercial Register</h2>
        <div class="contact-info">
          <strong>Court:</strong> Amtsgericht Bad Kreuznach<br />
          <strong>Registration Number:</strong> HRB 3460
        </div>
      </section>

      <section>
        <h2>VAT Identification Number</h2>
        <div class="contact-info"><strong>VAT ID:</strong> DE 812 004 832</div>
      </section>

      <section>
        <h2>Responsible for Content</h2>
        <div class="contact-info">
          <strong>According to § 18 (2) Medienstaatsvertrag (MStV):</strong><br />
          Dominik Ottenbreit<br />
          Newmatik GmbH<br />
          Am Markt 1<br />
          55619 Hennweiler<br />
          Germany
        </div>
      </section>

      <section>
        <h2>Privacy</h2>
        <p>
          How this website processes personal data is described in the
          <NuxtLink to="/privacy">privacy notice</NuxtLink> (German:
          <NuxtLink to="/datenschutz" hreflang="de">Datenschutzerklärung</NuxtLink>).
        </p>
      </section>
    </article>
  </main>
</template>

<script setup>
useHead({
  title: 'Imprint',
  meta: [{ name: 'description', content: 'Legal information and imprint for EsoCore.' }],
})
</script>
