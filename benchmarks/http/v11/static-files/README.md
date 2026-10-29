# Static file delivery

This local harness compares direct reads with the example's opt-in small-file
cache through the same TCP transport. It verifies every response's bytes.
Build **static_file_server** in Release mode first. Python 3 needs no packages.

```console
python benchmarks/http/v11/static-files/benchmark.py out/build/windows-parity/examples/http/v11/static_file_server/static_file_server.exe --output out/static-files.json
```

Use the corresponding executable path on Linux. Results include first-request
application-cold latency, warm throughput, p50/p95/p99, server CPU time, RSS/peak RSS
and handles/file descriptors after the run. Each case starts a fresh server. The
OS page cache is never flushed. Modes are direct streaming and attribute-validated
caching (the example default). --runs defaults to 3 and rotates mode order; every
repetition is retained. Cached bytes require matching identity, size, timestamps,
flags and link count on every reuse. Missing variants use an attribute-only query,
not a data open or cached absence. Files above 1 MiB stream in every mode.
The default budget is 512 MiB, 1024 entries and 16 simultaneous loads; occupied
same-key loaders are bypassed.

To include the initial TTL implementation, provide a separately built example
binary from revision 6fbb4290:

```console
python benchmarks/http/v11/static-files/benchmark.py CURRENT_BINARY --ttl-binary INITIAL_TTL_BINARY --runs 3 --output out/static-four-modes.json
```

Both binaries must expose the same root/optional-port example CLI. Build both with
matching optimization, TLS configuration and worker settings. This harness checks
all response bytes but its Python client can limit throughput; use wrk or the
remote benchmark for saturation measurements, retaining every run and socket error.

The defaults cover one hot file, 32 files, 1/8 clients, and 64 B through 2 MiB.
Add **--read-delay-ms 1** for delayed reads or reduce **--requests** for a smoke run.
Timing includes the Python client; this is a local comparison, not a saturation
benchmark or a published performance claim. Handle counts are sampled after each
run, not continuously. RSS peaks include startup. Run repeated samples on an idle
host before drawing conclusions; compare matched rows and include variance.

Recorded development samples: [Windows normal](results/windows-smoke-2026-10-21.json)
and [delayed reads](results/windows-slow-smoke-2026-10-21.json). These are short
correctness/performance smoke runs on a shared workstation, not saturation results.
Those archived samples used the earlier 64 MiB budget and 256 KiB file threshold;
their 2 MiB cases therefore streamed in both modes. New runs use the current defaults.

Archived results predate per-request metadata validation and nonwaiting load
admission. Re-run the harness for comparable current performance numbers.


The October 22 native-monitor reports below are historical: that implementation
and its example have been removed. Their single-file numbers do not represent the
twenty-route TLS workload:
[original comparison](results/linux-monitor-2026-10-22.md),
[fallback investigation](results/linux-monitor-fallback-2026-10-22.md).

## Twenty real TLS routes

Build tls_server.cpp twice with matching compiler flags, once against the baseline
include tree and once against the current tree. The program uses only the normal
static_file_server controller and cache, with the unchanged TLS transport:

```sh
clang++ -O3 -DNDEBUG -std=c++20 -pthread -Iinclude benchmarks/http/v11/static-files/tls_server.cpp -lssl -lcrypto -o out/static-tls-current
python3 benchmarks/http/v11/static-files/compare_tls.py BASELINE_BINARY out/static-tls-current /path/to/HttpArena/data/static /path/to/server.crt /path/to/server.key --workers 16 64 --runs 3 --output out/static-tls-20.json
```

The script rotates the official twenty paths with Accept-Encoding: br;q=1,
gzip;q=0.8, 1024 connections, four wrk threads and five seconds per run. It checks
the exact encoded bytes for all twenty paths before each load run. Both servers
use the same worker count, TLS fixtures, cache defaults and asset root. All runs
and raw wrk socket errors are retained. Throughput uses requests divided by the
parent's monotonic wall duration. CPU/RSS come from the server process; RSS is
sampled after load. Use native local storage, record filesystem and hardware,
and run on an idle host. A 64-worker run on a 16-vCPU workstation is oversubscribed;
it does not reproduce a 64-core Scaleway host.

Final removal/attribute-validation measurements, all twenty routes and all twelve
runs: [October 29 report](results/linux-attributes-20-routes-2026-10-29.md). The
report includes the regression at 64 workers, the timeout run and CPU changes;
it does not claim Scaleway throughput recovery.
