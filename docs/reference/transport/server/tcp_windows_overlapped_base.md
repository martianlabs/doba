<a id="tcp-windows-overlapped-base"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/title.svg" alt="tcp_windows_overlapped_base">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/title-dark.svg" alt="tcp_windows_overlapped_base">
</a>

**Header:** [include/transport/server/tcp_windows_overlapped_base.h](../../../../include/transport/server/tcp_windows_overlapped_base.h)

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

Internal base for the three native IOCP operation kinds: accept, receive and
send. Its OVERLAPPED address remains stable while the kernel owns the operation.
The worker dispatches each real completion by kind; there are no application-job
or synthetic-output operation types.

The derived operation carries the socket/context and any WSABUF needed until
completion. The worker destroys it after handling that completion, or immediately
when submission failed without posting I/O. See [worker](tcp_windows_worker.md).
