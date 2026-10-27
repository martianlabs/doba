# Monitor fallback investigation

22 October 2026. This experiment follows the reported remote regression (67239
initial TTL RPS versus 8443 current RPS). It reproduces a similar loss locally,
but does not establish the cause on the remote host.

The same binaries and eight-worker HTTP setup as the
[native-monitor comparison](linux-monitor-2026-10-22.md) were used. The change is
the asset root: it now resides on the WSL Windows-drive mount under
`/mnt/d/projects/martianlabs/doba/out`, reported by statfs as v9fs (0x01021997),
instead of Linux-local storage. Each mode ran three times with rotating order,
4 KiB payloads and `wrk -t4 -c256 -d5s -T5s --latency`. Every repetition is retained.

| Mode | Median requests / measured wall second | Median server CPU |
| --- | ---: | ---: |
| Initial TTL | 331632 | 493.2% |
| Current default (attribute fallback) | 33314 | 234.1% |

A separate observation probe returned zero and `Operation not supported`.
Tracing 100 HEAD requests produced 99 final-file openat2 calls, 102 final-file
fstat calls including cold admission, and zero monitor poll calls. This is
attribute fallback, despite monitoring being enabled in the policy.

The current-default run 3 reported 512 socket timeouts and an inconsistent wrk
summary duration. Its result is retained; throughput consistently uses request
count divided by the parent process's measured wall duration. The other five
runs reported no socket errors. This experiment does not resolve external
socket timeout reports.

[Complete measurements and probe](linux-monitor-fallback-2026-10-22.json).
Raw trace: `out/cache-monitor-fallback-trace.log`; reproduction runner:
`out/cache-monitor-mounted-compare.py`, derived from the original comparison
runner by placing its temporary directory in `out` and selecting the two modes.

Before interpreting a remote monitor benchmark, establish actual coverage
inside the server's container/mount namespace using the existing example:

```sh
stat -f -c '%T %t' /data/static
common_filesystem_monitor /data/static EXISTING_RELATIVE_FILE 1
```

A printed generation establishes initial native coverage for that file; an error
identifies a fallback reason. It does not prove continued coverage under load.
Resource limits, notifications and cache admission must still be investigated
if the benchmark is slow with working native coverage. Do not enable unsupported
filesystems indiscriminately or restore TTL-only freshness to hide the regression.
