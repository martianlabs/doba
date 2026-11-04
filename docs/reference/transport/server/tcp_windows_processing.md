# Windows TCP processing turns

Headers: [context](../../../../include/transport/server/tcp_windows_context.h),
[worker](../../../../include/transport/server/tcp_windows_worker.h),
[immediate packet](../../../../include/transport/server/tcp_windows_overlapped_processing.h)
and [group timer](../../../../include/transport/server/tcp_windows_processing_timer.h).
The public [processing result](../../protocol/processing_result.md) is shared
with Linux. This page describes the internal IOCP delivery implementation.

## Ownership and serialization

The existing context mutex serializes input, socket completions, processing
turns and administrative stop. Engines have no physical-thread affinity on
Windows. Only IOCP workers enter the engine; the timer callback never enters a
context, engine, protocol, reader or coroutine. It posts one generic group wake.

An immediate turn has one IOCP packet owning the exact context with shared_ptr.
Its pending flag is set before publication under the context mutex. Another
worker may dequeue before PostQueuedCompletionStatus returns; it still cannot
enter that context before publication completes. No packet is accessed by its
producer after successful ownership transfer. A failed post retains ownership,
deletes the packet and closes the connection without leaving a pending flag.

The timer wake uses stable group-owned storage, protected by the timer mutex.
At most one wake is queued. It contains no connection address, task, frame or
application function. An IOCP worker takes up to 64 due registrations under that
mutex, releases it and then processes the connections. Registrations hold exact
connection owners and a version checked under each context lock. Closing or
replacing a deadline withdraws the registration eagerly; already-selected stale
versions cannot enter the engine or address a reused connection.

## Snapshot and output

Each engine return replaces the full processing snapshot. Immediate readiness,
a future deadline and drained-output interest remain independent. Several ready
reasons coalesce. An already-idle output interest posts a turn without requiring
a new WSASend completion. A future deadline remains registered while immediate
turns run or output is busy. Each processing entry receives the current output
idle snapshot and replaces all interests again. Close always wins.

TX bytes, prefix/source, one outstanding receive and one outstanding send retain
the existing IOCP procedure. Processing turns can advance admitted handlers
while a reader keeps TX busy; the engine retains response order. The transport
never accepts coroutine handles or runs application tasks outside engine calls.

## Timer and resource bounds

One native thread-pool timer is created lazily for the entire IOCP worker group,
not per context or coroutine. Empty synchronous results do not post packets,
allocate processing records, query the clock or take the timer mutex. Immediate
and output-only traffic do not create a native timer. There is at most one
registration per timed connection and one queued immediate packet per context.

Absolute steady-clock dates are retained; unchanged earliest dates do not
restart the native timer. Windows future targets round up to 100 ns ticks.
Already-due work posts the coalesced IOCP wake directly, including the remainder
of a bounded batch. It does not wait for another native timer tick. Native
backoff after failed publication uses at least 1 ms. Workers check the absolute
date before selecting work, so early native wakeups cannot resume handlers.
There is no real-time latency guarantee. time_point::max() remains a future
wait and is cancelled normally.

The timer callback only publishes a wake. Failed posts retain all registrations;
the same native timer provides backoff when publication remains unavailable.
Producer stop cancels retries.
Initial timer creation, registration allocation or immediate publication failure
closes the affected connection. No unbounded retry queue or extra application
thread is introduced. A bounded 64-entry array selects due work without heap
allocation. Remaining overdue registrations queue the next group notification
on IOCP immediately, preserving bounded batches and ordinary I/O interleaving.

## Stop and tests

Stop first closes admission and waits for accept initialization. It disables
timer producers, waits for native callbacks and withdraws registrations before
stopping contexts. Existing Windows output-drain semantics remain: a peer must
make progress or disconnect. Close clears engine scheduling; queued immediate
packets remain owned until IOCP consumes them. Workers drain packets, then join;
only afterwards are the timer storage and IOCP released. Restart starts empty.

[Native tests](../../../../tests/integration/transport/server/tcp_processing_windows_tests.cpp)
cover interest combinations, independent deadlines, max dates, busy source TX,
concurrent HTTP handlers with ordered mixed output, requeue and stop/restart.
The isolated
[IOCP probe](../../../../tests/integration/transport/server/tcp_processing_windows_probe.cpp)
forces dequeue before post return, queued-wake cancellation, 64/65 registration
batches without native rearming of overdue work, lazy/shared timer creation
and native publication/creation failures.

TLS forwards the same generic turns without changing this native delivery.
The final local comparison with the frozen synchronous baseline and Spliffy
covers baseline, pipelined and async with equivalent resources per server.
The [plan](../../../../ASYNC.md) records the qualification scope. Remote CI
remains a separate release gate.
