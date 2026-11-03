import { readdir, readFile } from 'fs/promises'
import { join, relative, resolve } from 'path'

// Full text of every doc for the client-side search on /docs. Prerendered to a single static
// file: the site is hosted on GitHub Pages, where the per-document /api/docs/* routes do not exist.
async function walk(dir: string): Promise<string[]> {
  const entries = await readdir(dir, { withFileTypes: true })
  const files: string[] = []
  for (const entry of entries) {
    const full = join(dir, entry.name)
    if (entry.isDirectory()) files.push(...(await walk(full)))
    else if (entry.isFile() && entry.name.endsWith('.md')) files.push(full)
  }
  return files
}

export default defineEventHandler(async () => {
  const root = resolve(process.cwd(), 'content')
  const files = await walk(root)
  const docs: Record<string, string> = {}
  for (const file of files.sort()) {
    const slug = relative(root, file).replace(/\\/g, '/').replace(/\.md$/, '')
    docs[slug] = await readFile(file, 'utf-8')
  }
  return docs
})
