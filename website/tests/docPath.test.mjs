// Regression tests for the docs API path-traversal fix. Run with `pnpm test`
// (Node's built-in test runner with TypeScript type stripping; no extra dependencies).
import assert from 'node:assert/strict'
import { join, resolve } from 'node:path'
import { test } from 'node:test'

import { resolveDocPath } from '../server/utils/docPath.ts'

const root = resolve('/srv/site/content')

test('maps valid slugs to Markdown files inside content/', () => {
  assert.equal(resolveDocPath(root, 'README'), join(root, 'README.md'))
  assert.equal(
    resolveDocPath(root, 'comparisons/foo-bar_1.2'),
    join(root, 'comparisons', 'foo-bar_1.2.md')
  )
})

test('rejects traversal and malformed slugs', () => {
  const payloads = [
    '..',
    '../package',
    '../../etc/passwd',
    'comparisons/../../secret',
    '..%2F..%2Fetc%2Fpasswd',
    '.env',
    'a/.hidden',
    '/etc/passwd',
    'a//b',
    'a\\..\\b',
    '',
    'foo\u0000bar',
  ]
  for (const slug of payloads) {
    assert.equal(resolveDocPath(root, slug), null, `slug ${JSON.stringify(slug)} must be rejected`)
  }
})
