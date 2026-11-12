<a name="controlled-task-ownership"></a>
<h1>
  <a href="../../../resources/examples/common/task_execution/h1-controlled-task-ownership.svg#gh-light-mode-only">
    <img src="../../../resources/examples/common/task_execution/h1-controlled-task-ownership.svg" alt="Controlled task ownership">
  </a>
  <a href="../../../resources/examples/common/task_execution/h1-controlled-task-ownership-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/examples/common/task_execution/h1-controlled-task-ownership-dark.svg" alt="Controlled task ownership">
  </a>
</h1>

Build `common_task_execution` from the repository root with examples enabled.
The executable prints `Completed steps: 3` after a yield and a short sleep.

`task_execution` owns a lazy task through suspension and completion. This
standalone example supplies turns from the calling thread; `sleep_until`
belongs to the example, not the task utility or transport. No executor is
created or injected. The HTTP engine translates readiness/deadlines into
generic native processing turns; TCP and TLS support that path on both OSes.

`cancel()` can instead destroy a suspended task without running the remaining
body. All calls must be serialized with the coroutine. Only `yield()` and
`sleep_for()` are supported.

See [the API contract](../../../docs/reference/common/task_execution.md).
