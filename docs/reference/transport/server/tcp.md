<a id="tcp"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp/title.svg" alt="tcp">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp/title-dark.svg" alt="tcp">
</a>

**Header:** [include/transport/server/tcp.h](../../../../include/transport/server/tcp.h)


<a id="runtime-contract"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> TCP runtime contract

This is the runtime contract for the bundled Linux and Windows TCP transports.
The engine and transport concepts check C++ signatures; they do not enforce
these ownership and execution rules. Both backends support synchronous inline
pipelining, prefix/source and generic processing turns;
coroutine frames and execution policy belong to the engine. TLS is separate.

**Input and callbacks.** Each connection has its own engine. Input and processing calls never
overlap for that connection. RX bytes are immutable and borrowed until the call
returns; the engine retains any incomplete protocol message it needs later.
Different connections may execute concurrently. Linux has one owning worker per
connection; Windows may use different IOCP workers between serialized calls.

The engine stores registered send/close callbacks for its context lifetime and
invokes them only during its current serialized input or processing turn. They
must not be exported, invoked later or called from another thread. After requesting
close, the engine must return without further output. Protocol processing and
any independent execution service belong to the engine. A blocking engine or
reader occupies its I/O worker; TCP does not schedule application tasks.

**Output ownership and order.** A send emits head, body and then its optional
reader. Accepted inline bytes are copied before send returns, so their original
storage can be reused. The transport owns the transferred reader. Borrowed
backing storage must remain valid until that reader is released. Multiple inline
sends accumulate in order; output generated during input is batched before socket
submission. Partial writes preserve the unsent bytes and their order. Acceptance
or socket completion does not guarantee that the peer consumed the bytes.

**One terminal reader.** A reader follows all accepted inline bytes. A second
reader or later nonempty output while it is pending closes the connection.
Source blocks are bounded by the transport TX budget. No new input is delivered
until pending reader output drains. Reader failure or no progress before EOF
closes the connection. An empty submission without a reader is a no-op.

Empty submissions are also no-ops before or after a terminal reader within the
current input call. They neither consume TX budget nor release or replace the
reader. A reader with no data still transfers ownership and is terminal; it is
not an empty submission. Empty calls cannot clear a previous output violation.

**Close, errors and administration.** Engine close stops new input and drains
previously accepted valid output. Socket errors, input exceptions, allocation
failures and output-contract or budget violations can abort the connection;
unsent output is not guaranteed to drain on these paths. Configuration,
notification registration, start and stop belong to a sequential external owner,
outside worker callbacks. Concurrent or reentrant administration is unsupported.
Start failure releases resources of workers already started. Stop releases the
transport's resources and returns only after its workers have joined. There is
no stop timeout: blocking engine/reader calls can delay shutdown, and Windows
also waits for pending output to make socket progress.

Connection notifications are optional and run outside transport locks. A
connection that reached notification initialization is reported disconnected
once. Notifications, factories and application state used by different workers
may be concurrent; their owner supplies synchronization for shared state.

**Limits.** receive_buffer_capacity bounds one RX block, independently of protocol
message limits. max_buffered_send_bytes bounds logical buffered TX bytes, including
the whole active batch during partial writes; it does not cap allocator capacity,
process RSS or reader backing storage. Both capacities must be positive. Exceeding
the TX budget closes the connection. max_connections=0 disables admission limiting.
listen_backlog=0 requests the system default; the kernel can cap that backlog.
Policies are copied and fixed for the lifetime of a transport instance.

**Deliberate platform differences.**

| Behavior | Linux | Windows |
|---|---|---|
| Socket I/O | Independent SO_REUSEPORT listeners and epoll workers | IOCP, AcceptEx, WSARecv and WSASend |
| worker_count=0 | One worker | Selected from hardware concurrency |
| Finite max_connections | Partitioned across workers; must cover every worker. A full worker can reject while another has room | Shared reservation budget; a slot remains held through pending I/O and context release |
| RX while inline TX is pending | Waits for TX to drain | May overlap RX and TX in the kernel; engine input remains serialized |
| Administrative stop | Waits for the active call, cancels pending output and closes connections | Drains accepted output and socket completions before joining workers |
| Engine close | Drains accepted output and closes | Drains accepted output, shuts down TX and discards RX until peer EOF |

A fatal Linux accept4 resource/system failure stops admission on that worker
until an explicit stop/start; existing connections continue. SO_REUSEPORT can
also share an endpoint across separate instances, so use one external owner per
endpoint unless sharing is intentional. Windows retains contexts and their
OVERLAPPED buffers until real I/O completion. These differences do not grant
asynchronous engine callbacks or application scheduling to either backend.

The public TCP transport. Platform workers, connection state and dispatch helpers are internal implementation details.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[tcp](#type-tcp)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-tcp"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tcp

**TYPE** · *transport::server*

**Linux declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tcp : public detail::linux_transport<ENty, FAty> { ... };
```

This class provides a TCP transport for an engine. It takes a `tcp_policies`
configuration and an engine factory. Linux uses independent SO_REUSEPORT
workers through `detail::linux_transport`; Windows retains its existing
`detail::basic_transport` backend.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.

**Source:** [include/transport/server/tcp.h:92](../../../../include/transport/server/tcp.h#L92)


## Processing-result support

Linux and Windows TCP deliver immediate, monotonic-deadline and output-drain
processing turns on their transport workers. TLS forwards the same complete
snapshots and executes engine turns through those serialized entries.
Coroutine frames remain engine-owned; no transport accepts application jobs
or externally resumed handles.

Both entry points retain the general
[processing_result](../../protocol/processing_result.md) contract. Each backend
reconciles one connection snapshot after an engine entry and output progress.
Busy TX still applies the existing RX policy, but does not block processing
turns of already-admitted tasks. No callback is recursive or invoked by a
background timer thread. Close withdraws scheduling while accepted output drains.

Linux uses a lazy ready/deadline registry and at most one lazy timerfd per
worker. Uncontended synchronous traffic creates neither. Unchanged earliest
deadlines avoid rearming the kernel timer. Epoll records use nonreused lifetime
identities, and registration cancellation precedes engine destruction.
See [Linux processing turns](tcp_linux_processing.md) for ownership, budgets,
shutdown, resource failures and tests.

Windows reuses its per-connection mutex for RX, TX and processing turns. Generic
immediate packets hold the exact connection owner. One lazy native thread-pool
timer serves the IOCP worker group; its callback only posts a coalesced wake.
IOCP workers select up to 64 due connections per notification, release the timer
lock and enter each connection under its existing lock. No application task or
SSL call runs in the native timer callback. See
[Windows processing turns](tcp_windows_processing.md).
