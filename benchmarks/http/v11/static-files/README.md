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
OS page cache is never flushed. Modes are direct streaming, attribute-based cache
(--metadata-cache) and native monitor cache (the example default). --runs defaults
to 3 and rotates mode order; every repetition is retained. Native hits check queued
events and a generation without opening/statting/reading the payload. Empty-queue
queries run concurrently; event consumption and generation changes are exclusive.
No callback thread or interval timer participates. Unsupported/lost coverage uses
safe attribute revalidation or a new load. Files above 1 MiB stream in every mode.
The default budget is 512 MiB, 1024 entries and 16 simultaneous loads; occupied
same-key loaders are bypassed. Native monitor resources are bounded separately.

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


Current native-monitor development comparison (all 18 final runs, initial TTL
headers from 6fbb4290, metadata and monitor modes):
[Linux/WSL report](results/linux-monitor-2026-10-22.md). The report retains timeouts,
raw data, the preliminary series and syscall counts; it is not a remote benchmark
claim. Rerun the same remote workload to evaluate production performance.

A [fallback investigation](results/linux-monitor-fallback-2026-10-22.md) reproduces
a roughly tenfold throughput loss on a WSL Windows-drive mount. Monitoring enabled
in policy does not prove native coverage: check the filesystem and observation
probe in the same container/mount namespace before comparing modes.
