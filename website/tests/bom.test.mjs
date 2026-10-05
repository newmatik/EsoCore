// Regression tests for public/bom.js: CSV values must never be rendered as HTML,
// and a failed CSV request must not be rendered as BOM rows. Run with `pnpm test`.
// bom.js is a plain browser script, so it runs here in a vm context with a
// minimal fake DOM (no extra dependencies).
import assert from 'node:assert/strict'
import { readFileSync } from 'node:fs'
import { test } from 'node:test'
import vm from 'node:vm'

const script = readFileSync(new URL('../public/bom.js', import.meta.url), 'utf8')

// Every innerHTML assignment is recorded; bom.js should make none.
const htmlWrites = []

class FakeNode {
  constructor(tag) {
    this.tagName = tag.toUpperCase()
    this.children = []
    this.dataset = {}
    this.style = {}
    this.className = ''
    this.value = ''
    this._text = ''
  }

  get textContent() {
    return this._text + this.children.map(c => c.textContent).join('')
  }

  set textContent(value) {
    this.children = []
    this._text = String(value)
  }

  set innerHTML(value) {
    htmlWrites.push(String(value))
  }

  appendChild(child) {
    this.children.push(child)
    return child
  }

  replaceChildren(...nodes) {
    this._text = ''
    this.children = nodes
  }

  addEventListener() {}

  walk() {
    return [this, ...this.children.flatMap(c => c.walk())]
  }
}

async function render(response) {
  htmlWrites.length = 0
  const mount = new FakeNode('div')
  const context = {
    document: {
      getElementById: id => (id === 'bom' ? mount : null),
      createElement: tag => new FakeNode(tag),
    },
    fetch: async () => response,
    CustomEvent: class {},
    Date,
  }
  context.window = { dispatchEvent() {} }
  vm.runInNewContext(script, context)
  await context.window.EdgeBOM.renderBOM('bom', '/bom/test.csv')
  return mount
}

const payload = '<img src=x onerror=alert(1)>'

test('renders CSV values as text, never as HTML', async () => {
  const csv = [
    'Category,Item,MPN,Supplier,Qty,Unit,Notes,Package',
    `${payload},Item <b>1</b>,MPN-1,Supplier,2,1.5,"note, with comma",0402`,
  ].join('\n')
  const mount = await render({ ok: true, status: 200, text: async () => csv })
  const nodes = mount.walk()

  const option = nodes.find(n => n.tagName === 'OPTION' && n.value === payload)
  assert.ok(option, 'category option is created with the raw value')
  assert.equal(option.textContent, payload)

  const cells = nodes.filter(n => n.tagName === 'TD').map(n => n.textContent)
  assert.deepEqual(cells, [
    payload,
    'Item <b>1</b>',
    'MPN-1',
    'Supplier',
    '2',
    '1.50',
    'note, with comma',
    '0402',
    '$3.00',
  ])

  const breakdown = nodes.find(n => n.tagName === 'SPAN')
  assert.equal(breakdown.textContent, payload)
  assert.deepEqual(htmlWrites, [])
})

test('does not render an error response as BOM rows', async () => {
  const mount = await render({
    ok: false,
    status: 404,
    text: async () => 'Category,Item\n<h1>Not found</h1>,missing',
  })
  assert.equal(mount.textContent, 'Failed to load BOM.')
  assert.equal(mount.children.length, 0)
  assert.deepEqual(htmlWrites, [])
})
