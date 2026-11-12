# <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> HTTP/1.1 engine

Header: [engine.h](../../../../../include/protocol/http/v11/engine.h).

One `engine<RQty, RSty, ROty>` handles both synchronous and asynchronous routes
without an activation flag, executor injection or alternative engine type.
The router must outlive its engines. All input, processing turns, callbacks
and destruction for one instance must be serialized and nonreentrant.

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Public interface

```cpp
engine(policies configuration, const ROty& router);
void set_on_send(protocol::send_delegate output);
void set_on_close(std::function<void()> close);
processing_result on_bytes_received(std::string_view input);
processing_result on_processing_turn(bool output_idle);
```

RX is borrowed for the whole call and may be overwritten when it returns.
The decoder owns incomplete messages. Requests for suspended handlers own
headers, body and route parameters before first invocation. Unadmitted bytes
are retained by the engine under a finite limit. There is no consumed-byte
return, input pause/resume interface or application execution service.

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Concurrent execution, ordered output

The engine admits requests in wire order. Several async handlers on the same
connection may be suspended simultaneously; later sync and async handlers can
execute while predecessors wait. Each coroutine's absolute steady-clock sleep
starts at its own suspension and is never restarted by RX or other work.
A fresh clock snapshot is used for each resume, so deadlines that expire while
another handler runs can become eligible in the same processing turn.

The earliest pending future deadline and immediate runnable work are reported
independently through [processing_result](../../processing_result.md).
A bounded round-robin scan progresses eligible tasks. The per-entry budget
covers async starts/resumes and buffered admissions; excess work requests a
later turn. A full window waiting only for timers does not busy-poll.

Only the completed prefix of request order may be emitted. Later completed
responses retain owned serialized bytes/readers in engine records. A terminal
source gates later sends until output_idle, but does not gate eligible handler
execution or their clocks. Pending interim 100 Continue output and final output
share one request record. Exceptions, authentication rejection and HEAD retain
the ordinary HTTP serialization rules.

An ordered close at request K preserves predecessors, discards successors and
calls close at its output frontier. Fatal byte-limit/delivery failures close
and cancel pending work immediately, without an out-of-order error response.
Accepted source ownership is retained through drain or engine destruction.

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Synchronous path and ownership

Uncontended synchronous inline responses use reusable engine storage and emit
directly. They do not allocate or scan async records. Pending-state allocation
is lazy, used when ordering or a terminal source requires it. The engine adds
no cross-thread completion callbacks, mutexes or shared application execution
state. Transports schedule generic connection turns, not application tasks.
Windows retains the existing context serialization required by IOCP.

The synchronous handler logic lives in
[request_processor_sync](request_processor_sync.md); coroutine ownership lives
in [request_processor_async](request_processor_async.md). Common routing,
authentication, response serialization and ordered delivery stay in engine.h.
[pending_response](pending_response.md) describes the ordered record.

Only `common::yield()` and `common::sleep_for()` awaiters are supported by the
controlled task owner. Arbitrary foreign resumption is rejected. Handlers are
cooperative: blocking or non-suspending computation occupies their worker.
There is no preemption or background application pool.

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Limits and current integration status

The [policies](policies.md) require positive finite limits: 128 pending request
positions, 1 MiB deferred raw input, 1 MiB pending serialized head/inline body
bytes and 64 processing steps per entry by default. A full position window
retains raw input until capacity is released; byte overflow closes before
copying excess data. Request/body/source policies remain independent.

Linux and Windows TCP supply immediate/deadline/output-drain turns through
transport workers, so cooperative HTTP handlers and reader continuation work
over plain TCP. No application executor or mode switch is required.
TLS forwards the same snapshots and engine turns over both TCP backends.
Its existing finite plaintext and ciphertext batch limits apply to each
input/processing entry. See the [TLS contract](../../../transport/tls.md).

See the [compiled in-memory example](../../../../../examples/http/v11/processing_turns/README.md)
and the [native TCP example](../../../../../examples/http/v11/async_routes/README.md).

See the [retirement inventory](async-retirement.md) for removed legacy
mechanisms, preserved ownership and coverage mapping.
