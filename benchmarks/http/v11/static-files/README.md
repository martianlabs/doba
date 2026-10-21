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
OS page cache is never flushed. The cached configuration reloads after its default
one-second retention TTL; requests revalidate open-file identity, size,
modification/change timestamps, native file flags and link count on every access,
without reading contents on cache hits.
Same-key requests bypass active loaders instead of waiting. Files above 1 MiB
use direct streaming in both modes. Its default
budget is 512 MiB with at most 1024 entries and 16 simultaneous loads.

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
