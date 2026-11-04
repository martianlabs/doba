<a id="tcp-windows-transport"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_transport/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_transport/title.svg" alt="tcp_windows_transport">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_transport/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_transport/title-dark.svg" alt="tcp_windows_transport">
</a>

**Header:** [include/transport/server/tcp_windows_transport.h](../../../../include/transport/server/tcp_windows_transport.h)

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

basic_transport delegates start, stop and notification registration to its IOCP
worker group. Its parameters are the engine and its factory; tcp_policies
configure workers, the IPv4 endpoint, RX capacity, buffered TX and admission.
IOCP, AcceptEx, WSARecv and WSASend remain the native Windows architecture.

Engines receive bytes synchronously and retain their own incomplete messages.
Transport callbacks accumulate output before submission. Prefix/source output
supports one terminal reader; inline RX and TX may overlap in the kernel while
connection state remains serialized. See the context and worker references for
buffer ownership, close progression and completion lifetime.

An external sequential owner configures, starts and stops the transport. Engine
callbacks cannot escape serialized RX/processing entries, and administrative calls cannot run
from its IOCP workers. Stop drains accepted output and socket operations before
joining workers and closing IOCP. Connection notifications are paired once and
run outside the transport's locks.

There is no application executor, TLS, inactivity timer, quiescence or flow
pressure/resume handshake. The transport does not schedule or retain application
tasks. A blocking engine or reader occupies its IOCP worker.
