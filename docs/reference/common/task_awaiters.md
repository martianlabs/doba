<a id="task-awaiters"></a>
<a href="../../../resources/docs/reference/common/task_awaiters/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/task_awaiters/title.svg" alt="task_awaiters">
</a>
<a href="../../../resources/docs/reference/common/task_awaiters/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/task_awaiters/title-dark.svg" alt="task_awaiters">
</a>

**Header:** [include/common/task_awaiters.h](../../../include/common/task_awaiters.h)

The two await operations supported by doba's tasks.

<a id="function-yield"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> yield

```cpp
yield_awaiter yield() noexcept;
```

Suspends the task and marks its execution owner ready for another turn.
It does not resume the task recursively.

<a id="function-sleep-for"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sleep_for

```cpp
sleep_awaiter sleep_for(std::chrono::steady_clock::duration duration) noexcept;
```

A positive duration suspends the task until the reported deadline.
Zero or negative durations continue in the same turn. Deadlines that would
overflow are clamped to steady_clock::time_point::max().

<a id="type-yield-awaiter"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> yield_awaiter

Declared in [yield_awaiter.h](../../../include/common/yield_awaiter.h).
await_ready returns false, await_suspend marks the execution owner ready,
and await_resume does nothing.

<a id="type-sleep-awaiter"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sleep_awaiter

Declared in [sleep_awaiter.h](../../../include/common/sleep_awaiter.h).
Stores the requested duration. await_ready handles nonpositive durations;
await_suspend records the deadline, and await_resume does nothing.
