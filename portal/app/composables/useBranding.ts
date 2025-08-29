import { defaultBranding, type BrandingConfig } from '../config/branding'

export function useBranding() {
  const config = useRuntimeConfig()

  const brand = computed(() => config.public.brand || defaultBranding.brand)
  const product = computed(() => config.public.product || defaultBranding.product)
  const displayBrand = computed(() => {
    const publicConfig = config.public as BrandingConfig & Record<string, unknown>
    return publicConfig.displayBrand ?? defaultBranding.displayBrand
  })
  const fullName = computed(() =>
    displayBrand.value ? `${brand.value} ${product.value}` : `${product.value}`
  )
  const portalTitle = computed(() => `${fullName.value} Portal`)

  return { brand, product, displayBrand, fullName, portalTitle }
}
