import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest'
import {
  apiErrorMessage,
  formatTimeAgo,
  humanizeKey,
  truncatedListNotice,
} from '../app/utils/format'

describe('formatTimeAgo', () => {
  const now = new Date('2026-03-10T12:00:00Z')

  beforeEach(() => {
    vi.useFakeTimers()
    vi.setSystemTime(now)
  })

  afterEach(() => {
    vi.useRealTimers()
  })

  function ago(ms: number) {
    return new Date(now.getTime() - ms).toISOString()
  }

  const minute = 60_000
  const hour = 60 * minute
  const day = 24 * hour

  it.each([
    [0, 'just now'],
    [59_999, 'just now'],
    [minute, '1m ago'],
    [59 * minute + 59_999, '59m ago'],
    [hour, '1h ago'],
    [23 * hour + 59 * minute, '23h ago'],
    [day, '1d ago'],
    [45 * day, '45d ago'],
  ])('renders an age of %d ms as "%s"', (ms, expected) => {
    expect(formatTimeAgo(ago(ms))).toBe(expected)
  })

  it('treats a timestamp slightly in the future (device clock skew) as "just now"', () => {
    expect(formatTimeAgo(ago(-5000))).toBe('just now')
  })
})

describe('humanizeKey', () => {
  it.each([
    ['spindle_speed', 'Spindle Speed'],
    ['max_temp_c', 'Max Temp C'],
    ['voltage', 'Voltage'],
    ['', ''],
  ])('turns "%s" into "%s"', (key, expected) => {
    expect(humanizeKey(key)).toBe(expected)
  })
})

describe('apiErrorMessage', () => {
  it('prefers the detail the server sent', () => {
    const error = { data: { detail: 'Not found.' }, message: '[GET] "/x": 404' }
    expect(apiErrorMessage(error)).toBe('Not found.')
  })

  it('falls back to the error message when the response has no detail', () => {
    expect(apiErrorMessage({ data: {}, message: 'Failed to fetch' })).toBe('Failed to fetch')
    expect(apiErrorMessage(new Error('Network down'))).toBe('Network down')
  })

  it('uses the fallback when the error carries nothing usable', () => {
    expect(apiErrorMessage({})).toBe('Unknown error')
    expect(apiErrorMessage(null)).toBe('Unknown error')
    expect(apiErrorMessage(undefined, 'Failed to load device data')).toBe(
      'Failed to load device data'
    )
  })
})

describe('truncatedListNotice', () => {
  it('is empty when the whole list is shown', () => {
    expect(truncatedListNotice(12, 12, 'devices')).toBe('')
    expect(truncatedListNotice(0, 0, 'devices')).toBe('')
  })

  it('says how many of the rows are shown', () => {
    expect(truncatedListNotice(2000, 12345, 'alerts (newest first)')).toBe(
      'Showing 2,000 of 12,345 alerts (newest first).'
    )
  })
})
