<a id="tcp-windows-overlapped-receive"></a>
<a href="../../../resources/docs/reference/transport/tcp_windows_overlapped_receive/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/transport/tcp_windows_overlapped_receive/title.svg" alt="tcp_windows_overlapped_receive">
</a>
<a href="../../../resources/docs/reference/transport/tcp_windows_overlapped_receive/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/transport/tcp_windows_overlapped_receive/title-dark.svg" alt="tcp_windows_overlapped_receive">
</a>

**Header:** [include/transport/tcp_windows_overlapped_receive.h](../../../include/transport/tcp_windows_overlapped_receive.h)

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

Internal WSARecv operation. It retains its context until the one outstanding
receive completion is handled, including cancellation after socket close.
The context supplies a fixed RX block and processes the completed fresh bytes
under its connection lock. The engine owns any incomplete protocol message.

The operation is destroyed after completion, or immediately if submission failed
without posting I/O. See [context](tcp_windows_context.md).
