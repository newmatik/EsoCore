import { readFile } from 'fs/promises'
import { resolve, sep } from 'path'

// Doc slugs are content-relative paths such as "bom" or "comparisons/foo": path segments of
// letters, digits, ".", "_" and "-", never starting with a dot. Anything else (including "..")
// is rejected before it reaches the filesystem.
const SEGMENT = /^[A-Za-z0-9][A-Za-z0-9._-]*$/

function notFound() {
  return createError({
    statusCode: 404,
    statusMessage: 'Document not found',
  })
}

function resolveDocPath(contentRoot: string, slug: string): string | null {
  const segments = slug.split('/')
  if (!segments.every(segment => SEGMENT.test(segment))) {
    return null
  }
  const root = resolve(contentRoot)
  const contentPath = resolve(root, `${segments.join('/')}.md`)
  // Defence in depth: the resolved file must stay inside content/.
  if (!contentPath.startsWith(root + sep)) {
    return null
  }
  return contentPath
}

export default defineEventHandler(async event => {
  const param = getRouterParam(event, 'slug')
  const slug = Array.isArray(param) ? param.join('/') : param || 'README'

  const contentPath = resolveDocPath(resolve(process.cwd(), 'content'), slug)
  if (!contentPath) {
    throw notFound()
  }

  try {
    return await readFile(contentPath, 'utf-8')
  } catch {
    throw notFound()
  }
})
