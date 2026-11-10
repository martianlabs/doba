# Linux processing turns

Internal headers:
[context](../../../include/transport/tcp_linux_context.h),
[worker](../../../include/transport/tcp_linux_worker.h),
[registrations](../../../include/transport/tcp_linux_processing_queue.h).
These are implementation details, not an application execution service.

## Responsibilities and entry rules

The context owns its RX/TX buffers, reader, engine and latest processing_result.
Both receive and processing entries allow send/close callbacks only inside the
current serialized owner-worker call. Input remains borrowed for the full call
and is discarded afterward. A processing entry receives output_idle=true only
when the inline buffer and terminal reader have drained into the socket API.
Close overrides returned scheduling interests and drains accepted valid output.

The worker reconciles the snapshot after an entry and socket output progress.
Immediate requests and output interest with already-empty TX queue a later turn,
without waiting for a new edge. Busy output does not suppress deadline turns;
the engine receives output_idle=false in that case. A ready reason and a future
deadline can coexist. Repeating an unchanged deadline does not postpone it.
An empty snapshot eagerly removes prior interests.

The worker stores connection identities in epoll and processing registrations,
never frames, application function objects or output bytes. It alone calls the
engine. Its one stop eventfd remains administrative; it carries no work payload.
Windows IOCP delivery is documented in
[Windows processing turns](tcp_windows_processing.md). TLS forwards turns through the same serialized entries.

## Storage and bounded progress

The processing registry is allocated only for runnable/deadline work. It holds
one coalesced ready-list entry and at most one ordered deadline per connection.
Output-only interest while busy stays in the context until flush makes it ready.
Uncontended synchronous traffic creates no registry or timer descriptor.

A timerfd using CLOCK_MONOTONIC is created only for a future deadline. It arms
for the remaining monotonic interval to the earliest target. The target remains
absolute in engine/registry state; unrelated RX does not restart a sleep. A cached
unchanged earliest target avoids timerfd_settime. Due work left by a bounded
promotion is polled directly rather than requiring another kernel timer.

Each worker iteration handles a bounded epoll batch, promotes at most 64 expired
connection deadlines and delivers at most 64 ready connection turns. A turn
which requeues itself goes to the tail and is not recursively entered. Socket
polling continues between batches. When nothing is ready or overdue, epoll waits
for socket, timer or stop events; waiting handlers do not busy-spin.

## Lifetime and shutdown

Each accepted connection receives a monotonically increasing 64-bit identity.
Descriptors may be reused; identities are not reused within a worker lifetime.
Already-captured epoll events resolve the identity in the current owner map,
so an old event cannot follow descriptor reuse to a new engine. Exhausting the
identity space stops further admission instead of wrapping.

Closing a connection removes its ready/deadline registrations before destroying
its context. An already-readable shared timer carries no context address; after
cancellation it cannot enter the destroyed engine. Shutdown prioritizes a stop
notification, clears registrations, closes the timer/listener and destroys
contexts on the owning worker before join returns. It does not wait for sleeping
coroutines. Worker-owned field changes become visible to the external owner
through join; no new synchronization primitive is needed.

Protocol exceptions/output violations close that connection. An unrecoverable
worker timer/epoll failure closes its listener and connections; restart belongs
to the external administrator. There is no timer callback thread or automatic
application executor recovery loop.

## Verification

[Unit tests](../../../tests/unit/transport/processing_queue_linux_tests.cpp)
use explicit monotonic timestamps for coalescing, unchanged/replaced deadlines,
exact due boundaries, empty-output readiness, eager cancellation and bounded
promotion. [Native tests](../../../tests/integration/transport/tcp_processing_linux_tests.cpp)
cover busy TX, future/immediate coexistence, one lazy timer, cancellation with
actual FD reuse, restart, maximum deadlines and concurrent HTTP handlers with
ordered output. Existing source/pipeline and error tests remain enabled.

See the [Linux async example](../../../examples/http/v11/async_routes/README.md)
and the [public TCP contract](tcp.md#runtime-contract).
