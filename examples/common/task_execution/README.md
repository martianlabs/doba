# Controlled task ownership

Build `common_task_execution` from the repository root with examples enabled.
The executable prints `Completed steps: 3` after a yield and a short sleep.

`task_execution` owns a lazy task through suspension and completion. This
standalone example supplies turns from the calling thread; `sleep_until`
belongs to the example, not the task utility or transport. No executor is
created or injected. The HTTP engine translates readiness/deadlines into
generic native processing turns; TCP and TLS support that path on both OSes.

`cancel()` can instead destroy a suspended task without running the remaining
body. All calls must be serialized with the coroutine. Only `yield()` and
`sleep_for()` are supported under controlled ownership; ordinary `task::start`
continues to support custom awaiters under its existing lifetime contract.

See [the API contract](../../../docs/reference/common/task_execution.md).
