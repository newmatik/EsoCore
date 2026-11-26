# Performance audit, 2026-11-25

Baseline: freshly fetched `main` at `a8a8a0a`. Finding: [#172](https://github.com/newmatik/EsoCore/issues/172).

## Verified improvement

The firmware compatible-device action serialized `site.name` and `configuration` with two extra queries per device.
Eager-loading those relations keeps the existing user-site restriction, compatibility filters and complete response.
No cache, schema or transaction changes are involved.

Reproduce from `server/`: `poetry run python scripts/benchmark_firmware_devices.py`.
The script explicitly replaces the database setting with an in-memory SQLite database before Django setup or any database operation.
It creates synthetic devices (half with configurations) and renders the actual DRF action's JSON response.
Authentication is forced; middleware, networking, production PostgreSQL and concurrent clients are outside this measurement.
Python 3.12.15, locked dependencies, Windows; 20 measured warm requests after one initial request per size.

| Devices | Queries before / after | p50 ms before / after | p95 ms before / after | p99 ms before / after | JSON bytes |
| --- | --- | --- | --- | --- | --- |
| 1 | 4 / 2 | 4.076 / 1.965 | 5.187 / 2.987 | 7.137 / 4.186 | 457 |
| 25 | 52 / 2 | 21.721 / 4.665 | 26.909 / 5.346 | 34.811 / 5.811 | 9,508 |
| 100 | 202 / 2 | 95.335 / 8.779 | 126.758 / 11.450 | 136.483 / 13.324 | 37,741 |

At 100 devices: 99.0% fewer queries, 90.8% lower observed warm p50. Payload size is unchanged.
The initial request at 100 devices was 78.789 ms before and 10.208 ms after; this is not a cold operating-system-cache benchmark.
Twenty samples make tail estimates coarse; parallel work on this machine affects wall-clock results. Query counts are deterministic.
No production latency, throughput, error-rate, CPU, memory or Core Web Vitals improvement is claimed.

## Validation and coverage

All 127 Django tests passed, with three PostgreSQL-specific tests skipped on SQLite.
The new test asserts two queries at both one and 25 devices and compares every serialized field, including missing configurations.
Existing tenant-isolation and anonymous-caller tests pass. Ruff, Black and isort checks pass.
PostgreSQL CI remains necessary before merge; no production load tests or deployment were run.

Reviewed API querysets, serializers, telemetry ingestion, portal API communication and public-site build/search configuration.
Ordinary lists already use joins/annotations; portal dashboard and device details already fetch independent data concurrently.
The public site is prerendered and loads its document-search payload only on first search.
No additional change was justified without representative fleet sizes and PostgreSQL query plans.
The firmware compatibility response remains unpaginated to preserve API behavior; assess a separately versioned paging contract
if fleet-size measurements show payload or memory pressure. Production traces, browser metrics, regional latency and saturation
testing require an agreed staging workload and telemetry access.
