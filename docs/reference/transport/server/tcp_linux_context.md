<a id="tcp-linux-context"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_linux_context/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_linux_context/title.svg" alt="tcp_linux_context">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_linux_context/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_linux_context/title-dark.svg" alt="tcp_linux_context">
</a>

**Header:** [include/transport/server/tcp_linux_context.h](../../../../include/transport/server/tcp_linux_context.h)

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

Internal connection state owned exclusively by its Linux worker. It owns the
engine, a fixed RX block, one accumulated TX string and at most one reader.
The worker owns the socket; no context mutex or shared ownership is required.

Fresh RX bytes are borrowed by one synchronous engine call. The engine stores
registered callbacks and invokes them only inside that call. Inline output is
copied before returning. Empty sends without a reader are no-ops; a non-null
empty reader is terminal. A second reader or later nonempty output is rejected.

TX preserves partial writes and EAGAIN without accepting new input until it
and the source drain. Source chunks respect the logical TX limit; backing
storage is independent. Engine close drains valid accepted output; an error
aborts it. Administrative stop releases pending output after the current call.
See the [runtime contract](tcp.md#runtime-contract) and [policies](tcp_policies.md).
