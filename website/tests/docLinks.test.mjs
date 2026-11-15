// Fails when a docs page, a Vue page or the docs navigation links to a /docs page that has no Markdown file in
// content/. Run with `pnpm test`. External links (http, mailto) and same-page anchors are not checked.
import assert from 'node:assert/strict'
import { existsSync, readdirSync, readFileSync } from 'node:fs'
import { dirname, join, relative, resolve, sep } from 'node:path'
import { test } from 'node:test'
import { fileURLToPath } from 'node:url'

import { docsConfig } from '../utils/docs-config.js'

const siteDir = resolve(dirname(fileURLToPath(import.meta.url)), '..')
const contentDir = join(siteDir, 'content')
const publicDir = join(siteDir, 'public')
const ORIGIN = 'https://site.invalid'

function walk(dir, ext) {
  const files = []
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const full = join(dir, entry.name)
    if (entry.isDirectory()) files.push(...walk(full, ext))
    else if (entry.isFile() && entry.name.endsWith(ext)) files.push(full)
  }
  return files
}

// The URL path a content file is served at: content/README.md -> /docs, content/a/b.md -> /docs/a/b.
function pagePath(file) {
  const slug = relative(contentDir, file).split(sep).join('/').replace(/\.md$/, '')
  return slug === 'README' ? '/docs' : `/docs/${slug}`
}

// True when an internal path is served by the site: a docs page, a Vue page or a file in public/.
function exists(path) {
  const clean = path.replace(/\/+$/, '') || '/'
  if (clean === '/docs') return existsSync(join(contentDir, 'README.md'))
  if (clean.startsWith('/docs/')) {
    const slug = decodeURIComponent(clean.slice('/docs/'.length))
    return slug !== 'README' && existsSync(join(contentDir, `${slug}.md`))
  }
  if (clean === '/') return true
  if (existsSync(join(siteDir, 'pages', `${clean.slice(1)}.vue`))) return true
  return existsSync(join(publicDir, decodeURIComponent(clean.slice(1))))
}

// Markdown link targets outside fenced code blocks and inline code.
function markdownLinks(markdown) {
  const targets = []
  // The open fence (e.g. '````'): a block ends only at a fence of the same character, at least as long, with no info string.
  let fence = null
  for (const line of markdown.split('\n')) {
    const marker = /^ {0,3}(`{3,}|~{3,})(.*)$/.exec(line)
    if (marker) {
      if (fence === null) {
        fence = marker[1]
        continue
      }
      if (marker[1][0] === fence[0] && marker[1].length >= fence.length && !marker[2].trim()) {
        fence = null
        continue
      }
    }
    if (fence !== null) continue
    const text = line.replace(/`[^`]*`/g, '')
    for (const match of text.matchAll(/\]\(\s*<?([^)\s>]+)>?(?:\s+"[^"]*")?\s*\)/g))
      targets.push(match[1])
  }
  return targets
}

function isInternal(target) {
  return !/^[a-z][a-z0-9+.-]*:/i.test(target) && !target.startsWith('#') && !target.startsWith('//')
}

test('every internal link in content/ points to an existing page', () => {
  const broken = []
  for (const file of walk(contentDir, '.md')) {
    const base = `${ORIGIN}${pagePath(file)}`
    for (const target of markdownLinks(readFileSync(file, 'utf-8'))) {
      if (!isInternal(target)) continue
      const { pathname } = new URL(target, base)
      if (!exists(pathname)) broken.push(`${relative(siteDir, file)}: ${target}`)
    }
  }
  assert.deepEqual(broken, [], `broken internal links:\n${broken.join('\n')}`)
})

test('every /docs link in the Vue pages and components points to an existing page', () => {
  const broken = []
  const vueFiles = ['pages', 'components', 'layouts']
    .map(dir => join(siteDir, dir))
    .filter(dir => existsSync(dir))
    .flatMap(dir => walk(dir, '.vue'))
  for (const file of [...vueFiles, join(siteDir, 'app.vue'), join(siteDir, 'error.vue')]) {
    if (!existsSync(file)) continue
    const source = readFileSync(file, 'utf-8')
    for (const match of source.matchAll(/\b(?:to|href)="(\/docs[^"#?]*)/g)) {
      if (!exists(match[1])) broken.push(`${relative(siteDir, file)}: ${match[1]}`)
    }
  }
  assert.deepEqual(broken, [], `broken /docs links:\n${broken.join('\n')}`)
})

test('every docs navigation entry has a Markdown file', () => {
  const missing = []
  const visit = items => {
    for (const item of items) {
      if (typeof item.slug === 'string' && !exists(item.slug ? `/docs/${item.slug}` : '/docs')) {
        missing.push(item.slug)
      }
      if (item.children) visit(item.children)
    }
  }
  visit(docsConfig)
  assert.deepEqual(missing, [], `navigation entries without a page: ${missing.join(', ')}`)
})

test('the link check itself detects a missing page', () => {
  assert.equal(exists('/docs/solutions/does-not-exist'), false)
  assert.equal(exists('/docs/solutions/predictive-maintenance'), true)
  assert.equal(exists('/docs/'), true)
  assert.deepEqual(markdownLinks('```\n[a](/x)\n```\n`[b](/y)` [c](/z "t")'), ['/z'])
  assert.deepEqual(markdownLinks('````md\n```\n[a](/x)\n```\n````\n[b](/y)'), ['/y'])
})
