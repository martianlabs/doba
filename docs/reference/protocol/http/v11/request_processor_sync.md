# <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Synchronous request processor

Header: [request_processor_sync.h](../../../../../include/protocol/http/v11/request_processor_sync.h).

`detail::request_processor_sync<RQty, RSty, ROty>` executes synchronous static
or parametrized handlers and route fallbacks. It has no per-connection state,
coroutine frame, scheduler or pending-response storage. The common engine
performs authentication, owns reusable response storage and handles exceptions,
serialization and ordered delivery. See [engine](engine.md).
