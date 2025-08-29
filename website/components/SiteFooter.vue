<template>
  <footer class="footer">
    <div class="container">
      <div>© {{ currentYear }} EsoCore</div>
      <div>
        <NuxtLink to="/docs">Docs</NuxtLink>
        &nbsp;•&nbsp;
        <a href="https://github.com/newmatik/EsoCore" target="_blank" rel="noopener">Open Source</a>
        &nbsp;•&nbsp;
        <NuxtLink to="/imprint">Imprint</NuxtLink>
      </div>
    </div>
  </footer>
</template>

<script setup>
const currentYear = new Date().getFullYear()
</script>
