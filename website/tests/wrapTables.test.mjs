// Tests for the table scroll wrapper of the docs pages. Run with `pnpm test`.
import assert from 'node:assert/strict'
import { test } from 'node:test'

import { marked } from 'marked'

import { wrapTables } from '../utils/wrap-tables.js'

test('wraps each rendered Markdown table in a scroll container', () => {
  const html = wrapTables(
    marked.parse('| a | b |\n| - | - |\n| 1 | 2 |\n\ntext\n\n| c |\n| - |\n| 3 |\n', { gfm: true })
  )
  assert.equal(html.match(/<div class="table-scroll"><table>/g)?.length, 2)
  assert.equal(html.match(/<\/table><\/div>/g)?.length, 2)
  assert.match(html, /<p>text<\/p>/)
})

test('keeps table attributes and leaves other HTML alone', () => {
  assert.equal(
    wrapTables('<table class="x"><tr><td>1</td></tr></table>'),
    '<div class="table-scroll"><table class="x"><tr><td>1</td></tr></table></div>'
  )
  assert.equal(wrapTables('<p>tablet</p><thead></thead>'), '<p>tablet</p><thead></thead>')
})
