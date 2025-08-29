import { defaultBranding } from '../config/branding'

export function useBranding() {
  const config = useRuntimeConfig()

  const brand = computed(() => config.public.brand || defaultBranding.brand)
  const product = computed(() => config.public.product || defaultBranding.product)
  const displayBrand = computed(() => (config.public as any).displayBrand ?? defaultBranding.displayBrand)
  const fullName = computed(() => displayBrand.value ? `${brand.value} ${product.value}` : `${product.value}`)
  const portalTitle = computed(() => `${fullName.value} Portal`)

  return { brand, product, displayBrand, fullName, portalTitle }
}


