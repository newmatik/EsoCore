import { resolve, sep } from 'node:path'

// Doc slugs are content-relative paths such as "bom" or "comparisons/foo": path segments of
// letters, digits, ".", "_" and "-", never starting with a dot. Anything else (including "..")
// is rejected before it reaches the filesystem.
const SEGMENT = /^[A-Za-z0-9][A-Za-z0-9._-]*$/

/**
 * Map a docs slug to a Markdown file inside `contentRoot`, or return null when the slug is
 * malformed or would resolve outside the content directory.
 */
export function resolveDocPath(contentRoot: string, slug: string): string | null {
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
