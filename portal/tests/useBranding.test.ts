import { describe, expect, it } from 'vitest'
import { useBranding } from '../app/composables/useBranding'
import { nuxt } from './setup'

describe('useBranding', () => {
  it('falls back to the default branding', () => {
    const { brand, product, fullName, portalTitle } = useBranding()

    expect(brand.value).toBe('Newmatik')
    expect(product.value).toBe('EsoCore')
    expect(fullName.value).toBe('Newmatik EsoCore')
    expect(portalTitle.value).toBe('Newmatik EsoCore Portal')
  })

  it('uses the configured brand and product', () => {
    nuxt.publicConfig = { brand: 'Acme', product: 'Monitor', displayBrand: true }

    expect(useBranding().portalTitle.value).toBe('Acme Monitor Portal')
  })

  it('leaves the brand out of the name when displayBrand is off', () => {
    nuxt.publicConfig = { brand: 'Acme', product: 'Monitor', displayBrand: false }
    const { fullName, portalTitle } = useBranding()

    expect(fullName.value).toBe('Monitor')
    expect(portalTitle.value).toBe('Monitor Portal')
  })
})
