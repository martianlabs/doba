# Attribute-validated static files: twenty real TLS routes

29 October 2026. The native filesystem monitor has been removed. This is a local
Linux/WSL development comparison, not the Scaleway result. These measurements do
not establish that the remote performance gap has been closed.

## Changes

- Every cached payload is validated by confined file attributes. Identity, device,
  size, modification/change timestamps, flags and link count participate. Changed
  attributes reload bytes even if content is identical.
- Missing variants use attribute queries before a data handle is acquired.
  Linux uses openat2/O_PATH when available, with the existing confined fallback.
  Windows uses FILE_READ_ATTRIBUTES handles and retains reparse/ancestor checks.
- No negative TTL or directory-timestamp cache: directory attributes can remain
  identical across rapid creates, which reproduced a missed variant in testing.
  New compressed siblings must be checked on the following request. MIME type
  does not suppress a legitimately published compressed sibling.
- Generic cache find() uses concurrent immutable lookups. LRU promotions use a
  nonblocking exclusive lock; overlapping recency updates can be coalesced.
  Sequential LRU order, admission/live-cost bounds, invalidation, expiry and
  single-flight contracts remain covered by tests. Custom clock/hash/equality
  functions must support concurrent calls.
- Last-Modified is formatted when a snapshot is loaded, avoiding repeated calendar
  conversion. Future timestamps still clamp to response time. Variant preference
  is ordered stably on the stack, without stable_sort's temporary allocation.
- Transport source, send procedure, TLS implementation and buffer sizes unchanged.

## Setup

Baseline: complete headers at aad25d74d8b96d7b600ea865df8e051a3a7381d1,
with its monitor and exclusive cache mutex. Candidate: final working-tree headers.
Both use the same tls_server.cpp, Clang flags, TLS fixtures, cache defaults and
ext4 asset root. The JSON records the kernel/compiler and every asset size/hash.

The twenty official route names rotate with Accept-Encoding: br;q=1, gzip;q=0.8.
Each fresh server first passes exact encoded-byte checks for all twenty routes.
wrk uses four threads, 1024 connections and five seconds, with three repetitions
per mode and worker count; order alternates. No best-run selection or exclusions.
RPS is request count divided by parent monotonic wall duration, including startup
of wrk consistently. RSS is sampled after load, not continuously.

The workstation has **16 logical CPUs**. A 64-worker case is oversubscribed, not
an emulation of Scaleway's 64 cores. Client and server share this host, limiting
what can be inferred about server saturation or production throughput.

## Final results

Medians include every repetition, including the timeout run.

| Workers | Version | RPS by wall time | CPU | RSS MiB | Runs with socket errors |
| --- | --- | ---: | ---: | ---: | ---: |
| 16 | before | 56219 | 610.0% | 105.2 | 0/3 |
| 16 | after | 57386 | 384.5% | 105.5 | 1/3 |
| 64 | before | 60075 | 550.8% | 110.2 | 0/3 |
| 64 | after | 56969 | 401.1% | 110.9 | 0/3 |

At 16 workers throughput changes by about +2.1%, CPU by -37.0%; at 64 workers
throughput changes by about -5.2%, CPU by -27.2%. Lower CPU and latency are useful,
but this does **not** demonstrate a meaningful throughput recovery. The 16-worker
candidate run 3 reports 795 timeouts and an inconsistent shorter wrk summary
duration; it remains included using the measured wall duration. Its cause is not
resolved or attributed to a component by this experiment.

| Workers | Run | Version | RPS by wall | Mean latency (wrk) | Socket connect/read/write/timeout |
| --- | --- | --- | ---: | --- | --- |
| 16 | 1 | before | 53578 | 18.93ms | 0/0/0/0 |
| 16 | 1 | after | 57386 | 13.20ms | 0/0/0/0 |
| 16 | 2 | after | 55609 | 14.53ms | 0/0/0/0 |
| 16 | 2 | before | 56219 | 18.24ms | 0/0/0/0 |
| 16 | 3 | before | 58812 | 17.33ms | 0/0/0/0 |
| 16 | 3 | after | 60215 | 13.62ms | 0/0/0/795 |
| 64 | 1 | before | 61567 | 18.06ms | 0/0/0/0 |
| 64 | 1 | after | 54419 | 14.52ms | 0/0/0/0 |
| 64 | 2 | after | 57127 | 14.01ms | 0/0/0/0 |
| 64 | 2 | before | 60075 | 18.39ms | 0/0/0/0 |
| 64 | 3 | before | 57926 | 19.07ms | 0/0/0/0 |
| 64 | 3 | after | 56969 | 13.92ms | 0/0/0/0 |

[Full final measurements](linux-attributes-20-routes-2026-10-29.json) contain raw
wrk output, metrics, asset hashes and syscall/adapter checks.
[Preliminary measurements](linux-attributes-preliminary-2026-10-29.json) predate
cached HTTP dates/stack variant ordering; they are retained separately and are
not pooled or used to claim an isolated speedup for either optimization.

## Syscalls and correctness

A separate trace of 100 HEAD requests (five rotations, including cold loading):

| Work | Before | After |
| --- | ---: | ---: |
| Failed statfs for missing variants | 25 | 0 |
| Failed data opens for missing variants | 25 | 0 |
| Failed attribute-only queries for missing variants | 0 | 25 |
| Native monitor polls | 120 | 0 |
| Native watch registrations | 26 | 0 |
| Payload read calls, cold snapshots only | 20 | 20 |

Warm reuse does not read the payload. Raw traces remain in
out/cache-removal-traces; summary is embedded in the final JSON.

All final checks pass: Linux 1084 unit + 209 integration; Windows 1068 unit + 158
integration. ASan/UBSan/leak checks pass 53 cache/filesystem/static unit tests;
TSan passes 13 generic-cache tests including overlapping hits and load races.
Both platforms build the updated examples. Public reference verification reports
zero errors. Tests include attribute-only changes, same-size/same-mtime
replacement, directory/symlink confinement, immediate add/remove of compressed
siblings, future date clamping, pinned cost and concurrent cache operations.

The actual HttpArena adapter was also built and checked independently: all twenty
encoded responses, four immediate publication/update checks, echo and baseline.
Adapter-only adjustments retain CPU affinity/cgroup detection, round fractional
quotas down to complete cores (minimum one), expose STATIC_DIR/TLS_CERT/TLS_KEY
for reproduction and log effective workers/cache limits. The controller, cache
and compressed-variant implementation remain library code, with one shared budget.
No worker-policy change was made to Doba itself. The complete official HttpArena
validator was not run in this local comparison.

## Reproduce

Use the build commands and compare_tls.py invocation in the
[benchmark instructions](../README.md#twenty-real-tls-routes). Preserve the same
assets, compiler flags, TLS implementation, workers and all repetitions. Build the
baseline from the full include tree at the recorded revision. Do not compare the
old one-file plaintext measurements with this twenty-route TLS workload.
