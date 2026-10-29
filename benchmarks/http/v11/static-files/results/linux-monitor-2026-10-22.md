# Native monitor versus initial TTL cache

Historical record: the native monitor and its example were removed on 29 October
2026. Commands and implementation descriptions below refer to the archived build,
not the current library.

Development comparison, 22 October 2026. These are local HTTP measurements, not
HttpArena/production numbers. They do not establish that the remote 67239 RPS
baseline has been recovered or that external timeouts are resolved.

## Configuration

- Linux/WSL kernel: 6.6.87.2-microsoft-standard-WSL2.
- Complete initial headers exported from `6fbb4290`, versus the working-tree library
  with native monitoring. The metadata mode uses the same current binary with
  `--metadata-cache`; monitor mode uses its default policy.
- Same example program and transport, 8 workers, Clang C++20 `-O3 -DNDEBUG -pthread`,
  plaintext HTTP, one immutable file under five directory levels in `/tmp`.
- `wrk -t4 -c256 -d5s -T5s --latency`, three repetitions per size/mode. Mode order
  rotates each repetition. Ten initial GETs are checked for status and exact bytes.
- Default cache budgets and one-second TTL retained in all cached configurations.
  No filesystem page-cache flushing. The root supports inotify; this is not a
  measurement of attribute fallback on unsupported mounts.
- RPS below uses request count divided by the parent process's monotonic wall
  measurement, including wrk startup/connect time consistently in all modes.
  Raw wrk output is retained; some timeout runs report inconsistent shorter
  durations. No best-run selection or exclusion of timeout runs is performed.
- CPU is server process CPU time divided by that wall duration. RSS is sampled
  **after** wrk exits; peak RSS and descriptor counts are retained in JSON.

## Results

All values below are medians of the three repetitions, including runs with errors.

| File | Mode | RPS | Mean latency reported by wrk (ms) | Server CPU (%) | RSS after run (MiB) | Runs with socket timeouts |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| 4096 | ttl | 337061 | 0.449 | 500.4 | 8.66 | 1/3 |
| 4096 | metadata | 377510 | 0.616 | 746.8 | 9.38 | 0/3 |
| 4096 | monitor | 352763 | 0.426 | 532.7 | 8.98 | 0/3 |
| 1048576 | ttl | 6543 | 26.390 | 615.4 | 11.14 | 0/3 |
| 1048576 | metadata | 5232 | 47.180 | 768.1 | 11.69 | 1/3 |
| 1048576 | monitor | 7522 | 31.380 | 769.8 | 12.36 | 0/3 |

The monitor median is +4.7% versus initial TTL at 4 KiB and +15.0% at 1 MiB.
The monitor does not beat metadata mode for the 4 KiB case on this host; it does
for 1 MiB. These figures must not be extrapolated to the remote TLS workload.
All six monitor runs report zero socket errors. The initial TTL 4 KiB run 2 and
metadata 1 MiB run 2 report timeouts; their results remain included.

| File | Run | Mode | Requests | Wall seconds | RPS by wall | Socket timeouts |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| 4096 | 1 | ttl | 1712660 | 5.081 | 337061 | 0 |
| 4096 | 1 | metadata | 1909246 | 5.057 | 377510 | 0 |
| 4096 | 1 | monitor | 1758523 | 5.096 | 345104 | 0 |
| 4096 | 2 | metadata | 1972979 | 5.087 | 387873 | 0 |
| 4096 | 2 | monitor | 1803770 | 5.060 | 356450 | 0 |
| 4096 | 2 | ttl | 1889695 | 5.531 | 341627 | 143 |
| 4096 | 3 | monitor | 1795236 | 5.089 | 352763 | 0 |
| 4096 | 3 | ttl | 1435980 | 5.107 | 281190 | 0 |
| 4096 | 3 | metadata | 1522095 | 5.061 | 300738 | 0 |
| 1048576 | 1 | ttl | 33384 | 5.103 | 6543 | 0 |
| 1048576 | 1 | metadata | 26695 | 5.102 | 5232 | 0 |
| 1048576 | 1 | monitor | 43431 | 5.086 | 8540 | 0 |
| 1048576 | 2 | metadata | 32009 | 5.056 | 6331 | 256 |
| 1048576 | 2 | monitor | 38518 | 5.121 | 7522 | 0 |
| 1048576 | 2 | ttl | 28400 | 5.117 | 5550 | 0 |
| 1048576 | 3 | monitor | 34688 | 5.092 | 6812 | 0 |
| 1048576 | 3 | ttl | 34448 | 5.101 | 6754 | 0 |
| 1048576 | 3 | metadata | 26540 | 5.106 | 5198 | 0 |

Full data: [final comparison](linux-monitor-2026-10-22.json), including raw wrk
latency distributions, errors, process CPU/RSS and descriptors. The preliminary
exclusive-lock implementation is retained separately as
[the first series](linux-monitor-serial-2026-10-22.json); it is not pooled with the
final series or used to claim a controlled speedup from the locking change.

## Syscall verification

100 HEAD requests, including the initial load and 99 warm hits:

| Mode | Final-file read opens | Attribute-only opens | Final-file fstat | Content reads | Monitor readiness polls |
| --- | ---: | ---: | ---: | ---: | ---: |
| Initial TTL | 1 | 0 | 3 | 1 | 0 |
| Metadata | 1 | 99 | 102 | 1 | 0 |
| Monitor | 1 | 0 | 3 | 1 | 101 |

Thus the 99 monitored warm hits do not open/stat/read the target file. Polls
address the inotify descriptor, not the file or its path. The initial load has
two additional admission/reuse checks. [Counts](linux-monitor-syscalls-2026-10-22.json).
The existing transport and the HttpArena driver have no changes in this work.

## Reproduction and limits

Build the initial headers from `git archive 6fbb4290 include` and the current
headers with the same example source/compiler flags. Fix the example's
`worker_count` to 8 in **both** profiling executables; omit the new monitor_files
assignment only when compiling against the initial headers. No transport changes
are needed. Compare default initial TTL, current `--metadata-cache` and current
default monitoring with the wrk command above and fresh server processes per run.
The full temporary profiling source/runner and traces are retained in the local
`out/cache-monitor-profile.cpp`, `out/cache-monitor-compare.py` and
`out/cache-monitor-trace-*.log` files.

The repository's `benchmark.py --ttl-binary ... --runs 3` can reproduce the four
modes with byte checks; its Python client can constrain saturation throughput.
Native notifications are subject to documented filesystem and delivery limits;
this change does not promise universal immediate coherence for delayed Windows
notifications, mmap/remote/mount changes or external Windows hard-link aliases.
Use explicit invalidation/coordinated publication where a stronger guarantee is
required. Unsupported/lost native coverage uses safe revalidation or a new load.
