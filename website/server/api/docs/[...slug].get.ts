import { readFile } from 'fs/promises'
import { resolve } from 'path'
import { resolveDocPath } from '../../utils/docPath'

function notFound() {
  return createError({
    statusCode: 404,
    statusMessage: 'Document not found',
  })
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
