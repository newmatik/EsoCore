/** Shared display helpers for the portal pages. Auto-imported by Nuxt. */

/** Age of a timestamp as "just now", "5m ago", "3h ago" or "2d ago". */
export function formatTimeAgo(dateStr: string): string {
  const diff = Date.now() - new Date(dateStr).getTime()
  const minutes = Math.floor(diff / 60000)
  if (minutes < 1) return 'just now'
  if (minutes < 60) return `${minutes}m ago`
  const hours = Math.floor(minutes / 60)
  if (hours < 24) return `${hours}h ago`
  return `${Math.floor(hours / 24)}d ago`
}

/** Turns an API key such as `spindle_speed` into the label "Spindle Speed". */
export function humanizeKey(key: string): string {
  return key.replace(/_/g, ' ').replace(/\b\w/g, c => c.toUpperCase())
}

/**
 * Message to show for a failed API call: the server's `detail` if it sent one,
 * else the error's own message, else the fallback.
 */
export function apiErrorMessage(error: unknown, fallback = 'Unknown error'): string {
  const err = error as { data?: { detail?: string }; message?: string } | null | undefined
  return err?.data?.detail || err?.message || fallback
}

/**
 * Notice for a list that was cut short, for example "Showing 2,000 of 12,345
 * alerts.", or an empty string when the list is complete.
 */
export function truncatedListNotice(shown: number, total: number, noun: string): string {
  if (total <= shown) return ''
  return `Showing ${shown.toLocaleString('en-US')} of ${total.toLocaleString('en-US')} ${noun}.`
}
