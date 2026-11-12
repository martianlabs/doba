<a id="tcp-windows-context"></a>
<a href="../../../resources/docs/reference/transport/tcp_windows_context/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/transport/tcp_windows_context/title.svg" alt="tcp_windows_context">
</a>
<a href="../../../resources/docs/reference/transport/tcp_windows_context/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/transport/tcp_windows_context/title-dark.svg" alt="tcp_windows_context">
</a>

**Header:** [include/transport/tcp_windows_context.h](../../../include/transport/tcp_windows_context.h)

Implementation support for doba. These declarations are internal and are not part of the supported public API.

[Internal implementation](#implementation)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="implementation"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Internal implementation

**INTERNAL HEADER**

Use the public TCP transport. This header is internal.

A context owns the socket, engine, fixed RX buffer, active TX batch and pending
inline output. The protocol retains incomplete messages. The engine processes
fresh input and processing turns under the same mutex. Its registered
send/close callbacks must not escape an entry or run from an external thread.
They copy output or mark close without posting socket I/O. Close also withdraws
any registered deadline through the group timer registry. The completed RX or
processing entry submits its accumulated output.

One mutex serializes engine input/turns, completions and administrative stop.
RX and TX may remain in flight together. A kernel-owned TX batch stays stable
while inline output accumulates separately. Their combined logical size is
limited by max_buffered_send_bytes; the limit does not preallocate storage.
Completion flags are published before releasing the mutex, including when a
packet is dequeued before WSARecv or WSASend returns.

At most one terminal reader is pending. Its prefix drains first, followed by
bounded chunks. A second reader or later nonempty output closes the connection;
empty submissions do nothing. No receive is posted until the reader's final
socket batch completes and the reader is released. Reader errors and lack of
progress before EOF close the socket. A reader can exceed the TX limit; borrowed
backing storage must outlive that reader and is outside the buffer limit.

Close advances through open, draining, shutdown, aborting and closed phases.
Engine close rejects later output, drains accepted TX and discards RX until
peer EOF, preserving the final response. Administrative stop drains TX but does
not wait for peer EOF. Errors close immediately. Successful or pending socket
operations keep their context, buffers and OVERLAPPED alive until completion.
A failed submission has no completion and releases its operation immediately.

Completions return the finished decision under the mutex: socket closed, no RX,
TX or immediate processing packet pending. The worker claims one removal from its live set and notifies
outside both locks. Contexts never call a retirement delegate. The destructor
closes any socket left by failed setup, after the final owner releases it.

The context owns generic processing interests and one coalesced immediate IOCP
packet, plus an optional versioned deadline in the group's timer registry.
Engine input, processing turns and native completions use the existing context
mutex. The engine owns task frames and ordering; a blocking entry occupies its
worker. No task executor, inactivity scan or pressure hook exists here.
See [Windows turns](tcp_windows_processing.md) and
[admission ownership](connection_admission.md).
