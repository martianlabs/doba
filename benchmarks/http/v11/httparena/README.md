# HttpArena HTTP/1.1 benchmark adapter

This adapter supports **baseline**, **pipelined** and **async** over plain TCP
on Linux and Windows. All handlers are registered in the same executable with
the normal single HTTP engine; there is no async mode switch or executor.

## Routes and scope

| Method | Route | Response |
| --- | --- | --- |
| GET | `/baseline11?a=13&b=42` | Query sum: `55`. |
| POST | `/baseline11?a=13&b=42` | Query sum plus decoded integer body. |
| GET | `/pipeline` | Plain text `ok`, including pipelined batches. |
| GET | `/delay/:ms` | Wait the parsed path value in milliseconds; plain text echo of that value. |

Baseline parses each request and checks signed addition for overflow. POST
accepts Content-Length and chunked bodies through the engine's body reader.
Invalid integers or overflowing sums return 400. No response is precomputed.

The async handler obtains its parameter through doba's own router, validates a
whole integer in 0..60000, then co_awaits common::sleep_for. Invalid or out-of-range
values return 400. Zero remains zero. Each request owns its value and absolute
wait; no blocking sleep, external I/O, shared delay variable or fixed timer is
used. The engine owns coroutine frames and ordered responses; the native
transport owns generic turns and bytes. Mixing sync/async in one pipeline is
supported without changing the transport or tuning library policies.

This follows the [HttpArena implementation contract](https://http-arena.com/docs/test-profiles/h1/async/implementation/).
Its [validation checks](https://http-arena.com/docs/test-profiles/h1/async/validation/)
require dynamic path-derived delays, matching response bodies and no early
response. The official timed workload currently uses a flat 10 ms value;
validation varies it. Library qualification additionally checks mixed ordered
requests on one connection. Supporting a single request per measured connection
does not qualify that same-connection behavior by itself.

Only HTTP on port 8080 is started by default. There are no TLS, JSON, static-file
or echo profiles and no datasets, certificate mounts or response caches.
meta.json declares exactly baseline, pipelined and async. PORT optionally chooses
another listening port for local tests; WORKERS selects a positive worker count
(default 1). Both are driver-only settings, not engine configuration switches.

## Build and run

Use C++20, CMake 3.20+ and platform thread support. Standalone builds require no
Python, RapidJSON, zlib or OpenSSL:

```sh
cmake -S benchmarks/http/v11/httparena -B out/httparena \
  -DCMAKE_BUILD_TYPE=Release
cmake --build out/httparena --config Release --parallel
WORKERS=1 ./out/httparena/doba_httparena_http_v11
```

Linux workers use independent SO_REUSEPORT listeners; a connection stays on its
accepting worker. Root test builds also build this driver and register
`doba_httparena_tests`, using Python 3.9+. That is a correctness test, not a
throughput run: dynamic GET/POST, chunked/fragmented input, sixteen-response
pipelines, invalid arithmetic, random delays, 32 different overlapping waits,
zero delay and mixed same-connection response order. The script reports batch
elapsed time but asserts no concurrency throughput ceiling.

The container still exposes port 8080 without data volumes:

```sh
docker build --tag doba-httparena-http-v11 \
  benchmarks/http/v11/httparena
docker run --rm --publish 8080:8080 --env WORKERS=1 doba-httparena-http-v11
```

The Dockerfile fetches DOBA_REF (default main). That ref must contain the
cooperative engine/native processing implementation; it does not include local
uncommitted library changes. Use the local build above to qualify this source
tree. Select a remote ref containing the matching implementation.

## Local reference against Spliffy

Use the same worker count and server CPU affinity for both servers, and a
separate CPU affinity for the load generator. Run servers sequentially.
Verify response bodies before measuring, including dynamic GET and POST
values, Content-Length and chunked bodies, fragmented requests and ordered
pipelines. Check the complete worker/listener group before admitting load.

The async-plan block-2 reference uses these platform-specific resources:

| Setting | Linux / WSL | Windows |
| --- | --- | --- |
| Workers per server | 4 | 1 |
| Server CPUs | 0,2,4,6 | 0 |
| Connections | 4096 | 1024 |
| Generator threads | 4 | 8 |
| Generator CPUs | 8,10,12,14 | 4-15 |
| Observer CPU | 15 | 2 |
| Generator | gcannon 0.6.1 | autocannon 8.0.0 |
| Node for Spliffy | 26.13.0 | 24.16.0 |

Both profiles warm the same admitted sockets for five seconds before
resetting measurement counters. Baseline measures ten seconds at depth one;
pipelined measures fifteen seconds at depth sixteen. Baseline rotates raw
GET, Content-Length POST and chunked POST templates. Pipelined targets
`/pipeline`. Both tools send batches of sixteen rather than a sliding window.

The Windows gate requires a checked response from all 1024 connections.
The Linux gate verifies 4096 accepted server sockets before warmup. Failed
admission, measured socket errors, timeouts, reconnects or invalid responses
reject a trial. Linux does not separately retain warmup error counters after
reset; Windows retains them and rejects warmup errors too. Functional
preflights run outside measured processes. Pilots are
preserved separately and do not enter the reported medians.

Run three alternating repetitions for each server and profile. Keep the
median-throughput run with its own latency, CPU and memory figures, and
report dispersion from all repetitions. CPU and peak RSS cover same-cohort
warmup plus measurement; throughput and latency exclude warmup. Summed
process RSS can count shared pages more than once. Resource figures and
throughput from different platforms are not directly comparable.

The runners, request templates, patches, manifests, binaries and raw output
are preserved in `out/async-processing/block-2/`. From the repository root:

```sh
python3 out/async-processing/block-2/linux/compare.py
```

```powershell
pwsh -NoProfile -File out/async-processing/block-2/windows/compare.ps1
```

The runners intentionally refuse to overwrite results. Linux runtime paths
and Windows dependency paths are recorded in the archived runners and
manifests; restore those frozen dependencies before reproducing a campaign.
These commands describe the local qualification setup, not a portable
HttpArena deployment. Choose disjoint physical cores suitable for another
host. Variation up to 10% is expected on this WSL machine.

The private Linux generator publishes stop/reset flags through C11 atomics;
its request, parser and statistics implementation remains unchanged. The
Windows copy adds cohort admission and counter reset around autocannon's
normal measurement path. Public client hooks supply raw templates, validate
response bodies and maintain sixteen-response batches. Original files and
patches remain in the artifacts; upstream installations are preserved.

Spliffy 1.7.0 uses the original HttpArena baseline and pipeline handlers,
with unrelated route files omitted and TLS disabled. Its pipeline handler
uses a native declarative response; doba parses each request and invokes
its normal synchronous handler. The comparison includes that difference.

The additional `original` executable uses the source snapshot preceding
async-plan block 1. It is rebuilt with the same compiler and Release options
as the current candidate and participates in the same alternating campaign.
Preserve it for the final accumulated sync-regression comparison. Register
both sync and async handlers in the current candidate, without an activation
switch, while retaining these baseline and pipelined loads. The final campaign
adds real /delay/10 requests for async against an equivalently configured
Spliffy handler; it must preserve all three profiles and raw validation logs.

Historical transport-rewrite evidence remains separately preserved under
`out/transport-linux/minimal/`. Its different campaigns and resource counts
must not be substituted for the new paired reference. Local measurements
are not published HttpArena leaderboard results.
