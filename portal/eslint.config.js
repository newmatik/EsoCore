import { createConfigForNuxt } from '@nuxt/eslint-config/flat'

export default createConfigForNuxt({
  // options here
}).append(
  // additional configs here
  {
    rules: {
      // Custom rules can be added here
      '@typescript-eslint/no-unused-vars': ['error', { argsIgnorePattern: '^_' }],
      'vue/multi-word-component-names': 'off', // Allow single-word component names
      'vue/no-unused-vars': 'error',
    },
  }
)
