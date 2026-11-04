<a id="tcp-windows-worker"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_worker/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_worker/title.svg" alt="tcp_windows_worker">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_worker/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_worker/title-dark.svg" alt="tcp_windows_worker">
</a>

**Header:** [include/transport/server/tcp_windows_worker.h](../../../../include/transport/server/tcp_windows_worker.h)

Implementation support for doba. These declarations are internal and are not part of the supported public API.

[Internal implementation](#implementation)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="implementation"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Internal implementation

**INTERNAL HEADER**

Use the public TCP transport. This header is internal.

The worker group owns its listener and IOCP. Start validates the endpoint,
creates IOCP, associates the listener, starts workers and arms AcceptEx. Failed
startup drains any operations already submitted and releases partial resources.
Each completion dispatches exactly one native accept, receive or send operation;
there are no synthetic output packets or application jobs. Workers wait without
a timer scan. TCP_NODELAY is enabled on accepted sockets.

The accept operation owns its socket until successful context construction.
The context then owns cleanup, including failed IOCP association or registration.
The operation destructor closes only untransferred sockets. Connection budget
reservation precedes engine allocation. Excess connections are rejected; the
reservation follows context lifetime, including outstanding I/O after closure.

A set retains admitted contexts so stop can visit idle connections. Real RX/TX
OVERLAPPED objects also retain their contexts until completion. A completion or
stop that observes a finished context claims removal by erasing it once. Only
the successful eraser reports disconnection, outside both transport locks.

Stop closes admission and waits for pending accepts to finish initialization.
It snapshots live contexts and releases the lifecycle lock. It then stops
native timer producers and requests connection drain. Real I/O and queued processing packets
finish before the live set becomes empty. Workers join before timer storage
and IOCP are released. The temporary snapshot avoids entering context
locks under the lifecycle lock. Stop never waits for application tasks.

The lifecycle mutex protects admission, pending accepts and the live set.
A condition variable waits only for real accept/context completion. The stop
signal and shared admission counter remain atomic because independent workers
and context locks access them. Each context has its own engine/socket mutex.
User notifications run outside these locks. No try_lock or weak_ptr is used.

Configuration, callback registration, start and stop are sequential operations
of an external owner. Reentrant or worker-owned administration is rejected.
Concurrent administrative success is unsupported. Engine callbacks run only
inside serialized RX/processing entries. A lazy group timer publishes generic
IOCP wakes; it never enters an engine. No external application producers,
quiescence, inactivity scan or pressure/resume service is provided. See
[processing delivery](tcp_windows_processing.md) and
[admission ownership](connection_admission.md).
