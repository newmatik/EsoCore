import { readFile } from 'fs/promises'
import { join } from 'path'

export default defineEventHandler(async event => {
  const slug = getRouterParam(event, 'slug') || 'README'

  try {
    // Read the markdown file from the content directory
    const contentPath = join(process.cwd(), 'content', `${slug}.md`)
    const content = await readFile(contentPath, 'utf-8')

    return content
  } catch {
    throw createError({
      statusCode: 404,
      statusMessage: 'Document not found',
    })
  }
})
