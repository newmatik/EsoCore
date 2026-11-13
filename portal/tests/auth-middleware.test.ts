import { describe, expect, it } from 'vitest'
import authMiddleware from '../app/middleware/auth'
import { useAuthStore } from '../app/stores/auth'
import { nuxt } from './setup'

type Middleware = (to: { path: string }) => unknown
const middleware = authMiddleware as unknown as Middleware

describe('auth middleware', () => {
  it('sends a signed-out visitor to the login page', () => {
    nuxt.navigateTo.mockReturnValue('redirect')

    expect(middleware({ path: '/dashboard' })).toBe('redirect')
    expect(nuxt.navigateTo).toHaveBeenCalledWith('/login')
  })

  it('lets a signed-in user through', () => {
    useAuthStore().token = 'abc123'

    expect(middleware({ path: '/devices/42' })).toBeUndefined()
    expect(nuxt.navigateTo).not.toHaveBeenCalled()
  })

  it('restores the session from the cookie before deciding', () => {
    nuxt.cookie('auth_token').value = 'abc123'

    expect(middleware({ path: '/dashboard' })).toBeUndefined()
    expect(useAuthStore().token).toBe('abc123')
    expect(nuxt.navigateTo).not.toHaveBeenCalled()
  })

  it('does not redirect the login page to itself', () => {
    expect(middleware({ path: '/login' })).toBeUndefined()
    expect(nuxt.navigateTo).not.toHaveBeenCalled()
  })
})
