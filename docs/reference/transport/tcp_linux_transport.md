<a id="tcp-linux-transport"></a>
<a href="../../../resources/docs/reference/transport/tcp_linux_transport/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/transport/tcp_linux_transport/title.svg" alt="tcp_linux_transport">
</a>
<a href="../../../resources/docs/reference/transport/tcp_linux_transport/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/transport/tcp_linux_transport/title-dark.svg" alt="tcp_linux_transport">
</a>

**Header:** [include/transport/tcp_linux_transport.h](../../../include/transport/tcp_linux_transport.h)

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

Internal Linux TCP backend. It validates policies and the complete numeric
endpoint, divides finite admission capacity across the effective worker count,
and starts independent SO_REUSEPORT workers. Zero workers selects one. A full
worker can reject while another has spare capacity; no redistribution is offered.

Local unique owners release the partially started group on failure. Start,
stop and notification registration belong to a sequential external owner.
Stop rejects worker-owned calls and joins workers before releasing shared
callbacks. Engine factories and notifications can run concurrently for different
connections; their shared state belongs to the application.

The backend supplies synchronous socket I/O, inline pipelining and one terminal
reader. It does not schedule application tasks or implement TLS, inactivity
scans, pressure/resume hooks or retained engine callbacks. See the
[TCP contract](tcp.md#runtime-contract) and [policy reference](tcp_policies.md).
