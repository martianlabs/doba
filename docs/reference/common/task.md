<a id="task"></a>
<a href="../../../resources/docs/reference/common/task/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/task/title.svg" alt="task">
</a>
<a href="../../../resources/docs/reference/common/task/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/task/title-dark.svg" alt="task">
</a>

**Header:** [include/common/task.h](../../../include/common/task.h)

Provides the task type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[task](#type-task)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::task

[promise_type](promise_type.md) ·
[handle_type](#alias-handle-type) ·
[start(completion)](#method-start) ·
[start(completion, scheduler)](#method-start-2)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-task"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> task

**TYPE** · *common*

**Declaration synopsis:**

```cpp
template <typename Tty = void>
class task { ... };
```

Provides the task type.

**Template parameters:**

- **Tty:** `typename Tty = void`.

**Source:** [include/common/task.h:46](../../../include/common/task.h#L46)

<a id="alias-handle-type"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> handle_type

**ALIAS** · *common::task*

```cpp
using handle_type = std::coroutine_handle<promise_type>;
```

Alias for `std::coroutine_handle<promise_type>`.

**Source:** [include/common/task.h:91](../../../include/common/task.h#L91)

<a id="method-start"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> start

**METHOD** · *common::task*

```cpp
void start(std::function<void(std::exception_ptr)> completion);
```

Starts the task lifecycle.

**Parameters:**

- **completion:** Callback notified on completion. Declaration: `std::function<void(std::exception_ptr)> completion`.

**Returns:** No value (`void`).

**Source:** [include/common/task.h:114](../../../include/common/task.h#L114)

<a id="method-start-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> start

**METHOD** · *common::task*

```cpp
void start(std::function<void(std::exception_ptr)> completion,
             task_scheduler scheduler);
```

Starts the task lifecycle.

**Parameters:**

- **completion:** Callback notified on completion. Declaration: `std::function<void(std::exception_ptr)> completion`.
- **scheduler:** Scheduler used for the coroutine. Declaration: `task_scheduler scheduler`.

**Returns:** No value (`void`).

**Source:** [include/common/task.h:120](../../../include/common/task.h#L120)

## Ownership models

A newly created task is lazy and owns its frame until moved, destroyed or
transferred. `start` transfers ownership to the detached coroutine: completion
runs once and the frame then destroys itself. A custom scheduler or awaiter
must keep every published handle valid and resume it exactly once. Destruction
of the moved-from task does not cancel detached work. Starting an empty or
already-transferred task throws `std::logic_error` instead of dereferencing an
empty handle. Exceptions from the completion callback remain suppressed.

For caller-controlled resumption and cancellation, transfer the unstarted task
to [task_execution](task_execution.md). It retains the frame at final
suspension, exposes the exception without a callback and only accepts
`yield()`/`sleep_for()`. The ownership choice is not an HTTP async-mode flag;
the engine will select controlled ownership automatically for async handlers.
