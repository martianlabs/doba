# <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Asynchronous request processor

Header: [request_processor_async.h](../../../../../include/protocol/http/v11/request_processor_async.h).

`detail::request_processor_async<RQty, RSty>` owns one request, response,
optional typed route-parameter owner and `common::task_execution`. Its frame is
destroyed before the objects it borrows. Several instances may be pending on
the same engine; one instance never resumes concurrently or recursively.

`start(match)` adopts an unstarted task. `resume(now)` executes at most one
eligible step and returns whether it ran. `done()`, `error()` and `processing()`
expose final state, failure and local turn interests. `request()` and
`response()` provide the common engine's authentication/serialization access.
Only yield/sleep awaiters are permitted, as described by the
[controlled task owner](../../../common/task_execution.md).

The processor has no external completion callback, thread or native timer.
The engine aggregates all processors' independent deadlines and enforces
ordered response delivery. See [engine](engine.md).
