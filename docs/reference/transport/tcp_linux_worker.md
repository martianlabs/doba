<a id="tcp-linux-worker"></a>
<a href="../../../resources/docs/reference/transport/tcp_linux_worker/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/transport/tcp_linux_worker/title.svg" alt="tcp_linux_worker">
</a>
<a href="../../../resources/docs/reference/transport/tcp_linux_worker/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/transport/tcp_linux_worker/title-dark.svg" alt="tcp_linux_worker">
</a>

**Header:** [include/transport/tcp_linux_worker.h](../../../include/transport/tcp_linux_worker.h)

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

Internal owner of one SO_REUSEPORT listener, epoll set, stop event descriptor,
thread and connection map. Socket events are processed only on that worker;
there is no dispatcher, task queue or communication between workers. Engine
and reader calls execute synchronously and may occupy the worker until return.

Admission is the finite budget assigned at startup or unlimited when zero.
EINTR retries and transient accept failures preserve admission. A resource/system
accept failure closes that worker's listener until explicit stop/start; existing
connections continue. There is no retry service or timer scan.

The external sequential owner signals stop, joins the worker, then releases its
descriptors. The worker releases pending contexts/readers rather than draining
TX on administrative stop. See [transport](tcp_linux_transport.md).
