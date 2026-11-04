<a id="send-state"></a>
<a href="../../../../resources/docs/reference/transport/server/send_state/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/send_state/title.svg" alt="send_state">
</a>
<a href="../../../../resources/docs/reference/transport/server/send_state/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/send_state/title-dark.svg" alt="send_state">
</a>

**Header:** [include/transport/server/send_state.h](../../../../include/transport/server/send_state.h)

Internal Windows TX storage: an active socket batch, accumulated inline bytes
and one terminal reader. The whole active batch remains charged to the logical
TX budget until its last byte completes. Allocator capacity and reader backing
storage are excluded from that budget.

push copies head/body and transfers the reader. An empty submission without a
reader changes nothing, including at exact capacity or with a pending reader.
A second reader or later nonempty output is rejected; the context then closes.
A non-null empty reader is still terminal.

fill never moves or resizes a batch still borrowed by the socket. It drains the
prefix before reading bounded source chunks. clear requires no outstanding I/O.
The context owns synchronization and I/O publication. See
[context](tcp_windows_context.md) and the [TCP contract](tcp.md#runtime-contract).
