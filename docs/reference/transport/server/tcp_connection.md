<a id="tcp-connection"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_connection/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_connection/title.svg" alt="tcp_connection">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_connection/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_connection/title-dark.svg" alt="tcp_connection">
</a>

**Header:** [include/transport/server/tcp_connection.h](../../../../include/transport/server/tcp_connection.h)

Internal Windows RX adapter. It owns one engine and a fixed receive buffer.
receive_buffer exposes that reusable block; process rejects a completion larger
than the block before calling the engine with fresh bytes. The protocol retains
incomplete messages after the synchronous call.

Output storage, synchronization and socket lifetime belong to the owning
[context](tcp_windows_context.md). This adapter has no TX queue, application
execution service or TLS state.
