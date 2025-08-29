export default defineNuxtPlugin(() => {
  // Ensure HTML has correct class when color mode changes via toggle
  const colorMode = useColorMode()
  watchEffect(() => {
    const isDark = colorMode.value === 'dark'
    if (process.client) {
      document.documentElement.classList.toggle('dark', isDark)
      document.documentElement.setAttribute('data-theme', isDark ? 'dark' : 'light')
    }
  })
})


