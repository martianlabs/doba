<a id="task"></a>
<a href="../../../resources/docs/reference/common/task/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/task/index.svg" alt="task">
</a>
<a href="../../../resources/docs/reference/common/task/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/task/index-dark.svg" alt="task">
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

**Source:** [include/common/task.h:44](../../../include/common/task.h#L44)

<a id="alias-handle-type"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> handle_type

**ALIAS** · *common::task*

```cpp
using handle_type = std::coroutine_handle<promise_type>;
```

Alias for `std::coroutine_handle<promise_type>`.

**Source:** [include/common/task.h:79](../../../include/common/task.h#L79)

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

**Source:** [include/common/task.h:102](../../../include/common/task.h#L102)

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

**Source:** [include/common/task.h:108](../../../include/common/task.h#L108)
