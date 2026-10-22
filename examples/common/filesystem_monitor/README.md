# Filesystem monitor

`common::filesystem_monitor` tracks native changes on the calling thread. It
creates no worker thread and invokes no user callback. Its generation covers the
whole observed root, so unrelated relevant changes within that coverage can also
invalidate a value. It is independent of HTTP and `common::cache`.

Build the `common_filesystem_monitor` target and run:

```sh
common_filesystem_monitor /absolute/asset/root hello.txt 100
```

The existing file is observed before the initial generation is printed. Edit,
replace, rename or remove it from another process. The example polls every 100 ms
for the requested number of iterations and prints generation changes or loss of
coverage. That polling interval belongs to this demonstration, not the monitor:
`poll()` and `unchanged()` return without waiting for future events. An HTTP cache
calls `unchanged()` immediately before accepting a hit.

Capture `observe(relative, error)` **before** reading a value, then check
`unchanged(version, error)` before admission and reuse. Zero/false means revalidate;
it must never be interpreted as an unchanged file. `reset()` invalidates every
observation and releases native resources. A subsequent observe rebuilds coverage.
A failed extension (for example a missing file) leaves existing coverage intact.

Linux uses nonblocking inotify, with ancestor/directory watches and inode watches
for covered files. Windows uses overlapped directory notifications and completion
events checked with zero wait. Windows ignores access-only directory notices only
after confirming stable directory attributes on that event. Reading a cached file
is not itself a reason to invalidate the snapshot.

Limits and coherence depend on the filesystem. Linux fast coverage is enabled for
local ext*, XFS, Btrfs, tmpfs and overlayfs; other mounts require revalidation.
Windows remote drives are rejected for fast coverage. The default budget is 4096
registered paths/directory snapshots, with at most 64 Windows directory handles.
Resources are retained until reset, topology rebuild or destruction. mmap writes,
external mount changes and external hard-link aliases on Windows are outside the
fast-coverage guarantee. Windows notifications can be delayed by filesystem
caching. Use explicit invalidation/publication coordination when a strict contract
requires stronger guarantees; notification absence alone does not prove freshness.

The [API reference](../../../docs/reference/common/filesystem_monitor.md) explains
resource limits, error handling and generation scope.
