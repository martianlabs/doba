<a id="filesystem-monitor"></a>
<a href="../../../resources/docs/reference/common/filesystem_monitor/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_monitor/title.svg" alt="filesystem_monitor">
</a>
<a href="../../../resources/docs/reference/common/filesystem_monitor/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/filesystem_monitor/title-dark.svg" alt="filesystem_monitor">
</a>

**Header:** [include/common/filesystem_monitor.h](../../../include/common/filesystem_monitor.h)

Caller-driven native change generations, independent of protocols and caches.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[filesystem_monitor](#type-filesystem-monitor) ·
[observe](#observe) · [poll](#poll) · [unchanged](#unchanged) · [reset](#reset)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>


<a id="type-filesystem-monitor"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> filesystem_monitor

**TYPE** · *common*

```cpp
explicit filesystem_monitor(const std::filesystem::path& root,
                            std::size_t max_watches = 4096);
```

Noncopyable, nonmovable and thread-safe. The constructor resolves the root; errors
are reported by observe(). No background thread or callback is created. Native
resources are acquired lazily and released at reset/destruction. Capture a token
before loading and check it after loading and immediately before reuse. A token
is valid only with the same monitor instance that produced it.

| Parameter | Default | Meaning |
| --- | --- | --- |
| root | required | Trusted directory, canonicalized during construction. |
| max_watches | 4096 | Bound on registered Linux paths/native watches and Windows directory snapshots; zero disables monitoring. Windows also limits native directory handles to 64. |

Generations cover **all** observations of this monitor, not a single filename.
Any relevant event invalidates earlier tokens, even for equal contents. Topology
changes and overflow return zero until coverage is rebuilt. Registered resources
are bounded and retained until reset, topology rebuild or destruction; individual
cache evictions do not release a subscription. Object lifetime must outlive calls;
destruction must not race callers.

<a id="observe"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> observe

```cpp
[[nodiscard]] std::uint64_t observe(std::string_view relative,
                                  std::error_code& error);
```

Registers the decoded root-relative file and its necessary ancestors/inode before
reading it. Returns a nonzero generation or zero if coverage cannot be established.
Rejects traversal/invalid paths. A failed extension leaves existing coverage intact.
A successful observation does not replace a confined file open or prove that the
path currently names a readable regular file. After coverage loss, retries rebuild
native state; callers must revalidate data before adopting a new generation.

<a id="poll"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> poll

```cpp
[[nodiscard]] std::uint64_t poll(std::error_code& error);
```

Consumes queued native events on the caller, without waiting for new events.
Returns the current nonzero generation, or zero on loss/unavailability of coverage.
Errors/overflow, unknown watch descriptors and exhausted drain budgets revoke fast
coverage. Linux drains at most 16 buffers per call; Windows at most 64 completions.
Empty-queue readiness checks use shared locking and may run concurrently. Event
processing/registration/reset use exclusive locking so no caller can accept an old
generation while another consumer has dequeued but not applied changes. No call reads
payload contents. Windows may query stable directory attributes **on an event** to
filter access-only notices caused by confined directory opens.

<a id="unchanged"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unchanged

```cpp
[[nodiscard]] bool unchanged(std::uint64_t generation, std::error_code& error);
```

Calls poll() and returns true only for a matching nonzero generation. False means
revalidate/discard; it is not permission to adopt a new generation for old bytes.
There is no interval timer and no dependency on a background consumer being scheduled.

<a id="reset"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> reset

```cpp
void reset();
```

Invalidates every previous token, cancels/releases watches and pending operations,
and makes the next observe rebuild coverage. Windows waits for cancellation to
complete before freeing OVERLAPPED/buffer storage. This does not alter files.

<a id="support"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Support and coherence

Linux uses nonblocking inotify with directory/ancestor and per-inode watches.
Fast coverage is enabled for ext*, XFS, Btrfs, tmpfs and overlayfs; unsupported
mounts or resource exhaustion require attribute revalidation/direct reads. Windows
uses ReadDirectoryChangesW with stable OVERLAPPED buffers, recursive root coverage,
ancestor filters and unique events polled with zero wait. No transport completion
port is shared. Windows remote drives do not get fast coverage.

No events available is not a universal proof of unchanged contents. inotify does
not cover all mmap or remote/mount changes. Windows size/write notifications may
be delayed by filesystem caching; external hard-link aliases on Windows are not a
fast-coverage contract. Publication must use supported native operations (prefer
atomic replacement), or explicit invalidation/cooperation where a stricter
next-response guarantee is required. Notify delivery, not a TTL, drives generations.

Native backends are implementation headers; include common/filesystem.h or this
header. See the [example](../../../examples/common/filesystem_monitor/README.md).
The generic cache contains no filesystem dependencies.
