<a id="task"></a>
<a href="../../../resources/docs/reference/common/task/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/task/title.svg" alt="task">
</a>
<a href="../../../resources/docs/reference/common/task/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/task/title-dark.svg" alt="task">
</a>

**Header:** [include/common/task.h](../../../include/common/task.h)

The lazy coroutine returned by doba's asynchronous HTTP handlers.

<a id="type-task"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> task

```cpp
template <typename Tty = void>
  requires std::same_as<Tty, void>
class task;
```

Only task<void> is supported. Creating a task leaves its body suspended.
Transfer it to [task_execution](task_execution.md), which resumes it on the
caller's processing turns and owns it until cancellation or destruction.

A task can be move-constructed. Copying and assignment are disabled.
Destroying a task before adoption releases its unstarted frame.
Once adopted, the original task no longer owns that frame.

<a id="method-release"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> release

```cpp
[[nodiscard]] handle_type release() && noexcept;
```

Transfers the frame and leaves the task empty. Call it on a moved task:
std::move(value).release(). The returned handle belongs to the receiver,
which must eventually destroy the frame. Releasing an empty task returns
an empty handle.

handle_type is std::coroutine_handle<promise_type>. task_execution uses this
transfer to adopt the frame. The promise creates the original task through
its explicit constructor taking promise_type&.

<a id="type-promise-type"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> promise_type

An alias for [task_promise](promise_type.md), declared in its own header.
The promise captures errors and keeps the completed frame suspended until
task_execution releases it.

Handlers use yield() and sleep_for() from [task_awaiters.h](task_awaiters.md).
The execution owner reports readiness, deadlines and errors to the HTTP
processor.
