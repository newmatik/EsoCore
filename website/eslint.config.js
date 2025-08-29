import { createConfigForNuxt } from '@nuxt/eslint-config/flat'

export default createConfigForNuxt({
  // options here
}).append(
  // Ignore patterns
  {
    ignores: [
      '_backup/**/*',
      'assets/**/*',
      'public/**/*',
      '.output/**/*',
      'dist/**/*',
      '.nuxt/**/*',
    ],
  },
  // additional configs here
  {
    rules: {
      // Custom rules can be added here
      '@typescript-eslint/no-unused-vars': ['error', { argsIgnorePattern: '^_' }],
      'vue/multi-word-component-names': 'off', // Allow single-word component names
      'vue/no-unused-vars': 'error',
      'vue/html-self-closing': 'off', // Allow both self-closing and non-self-closing
      'vue/no-v-html': 'warn', // Allow v-html but warn about it
    },
  }
)
