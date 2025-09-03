import { readFile } from 'fs/promises'
import { join } from 'path'

export default defineEventHandler(async (event) => {
  const param = getRouterParam(event, 'slug')
  const slug = Array.isArray(param) ? param.join('/') : (param || 'README')

  try {
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


