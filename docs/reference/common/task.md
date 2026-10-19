<a id="task-h"></a>
<h1>
  <a href="../../../resources/docs/reference/common/task/index.svg#gh-light-mode-only">
    <img src="../../../resources/docs/reference/common/task/index.svg" alt="Index for task.h">
  </a>
  <a href="../../../resources/docs/reference/common/task/index-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/docs/reference/common/task/index-dark.svg" alt="Index for task.h">
  </a>
</h1>

**Header:** [include/common/task.h](../../../include/common/task.h)

Provides the task type.

**common**

- [task](#type-task) · *type*

**common::task**

- [promise_type](#type-promise-type) · *type*

**common::task::promise_type**

- [get_return_object](#method-get-return-object) · *method*
- [initial_suspend](#method-initial-suspend) · *method*
- [final_awaiter](#type-final-awaiter) · *type*

**common::task::promise_type::final_awaiter**

- [promise](#field-promise) · *field*
- [await_ready](#method-await-ready) · *method*
- [await_suspend](#method-await-suspend) · *method*
- [await_resume](#method-await-resume) · *method*

**common::task::promise_type**

- [final_suspend](#method-final-suspend) · *method*
- [return_void](#method-return-void) · *method*
- [unhandled_exception](#method-unhandled-exception) · *method*
- [completion](#field-completion) · *field*
- [error](#field-error) · *field*
- [scheduler](#field-scheduler) · *field*

**common::task**

- [handle_type](#alias-handle-type) · *alias*
- [start(completion)](#method-start) · *method*
- [start(completion, scheduler)](#method-start-2) · *method*

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

<a id="type-promise-type"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> promise_type

**TYPE** · *common::task*

**Declaration synopsis:**

```cpp
struct promise_type { ... };
```

Provides the promise type type.

**Source:** [include/common/task.h:51](../../../include/common/task.h#L51)

<a id="method-get-return-object"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_return_object

**METHOD** · *common::task::promise_type*

```cpp
task get_return_object() noexcept;
```

Returns return object.

**Returns:** A value of type `task`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:52](../../../include/common/task.h#L52)

<a id="method-initial-suspend"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> initial_suspend

**METHOD** · *common::task::promise_type*

```cpp
std::suspend_always initial_suspend() noexcept;
```

Controls whether a new coroutine suspends initially.

**Returns:** A value of type `std::suspend_always`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:55](../../../include/common/task.h#L55)

<a id="type-final-awaiter"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> final_awaiter

**TYPE** · *common::task::promise_type*

**Declaration synopsis:**

```cpp
struct final_awaiter { ... };
```

Provides the final awaiter type.

**Source:** [include/common/task.h:56](../../../include/common/task.h#L56)

<a id="field-promise"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> promise

**FIELD** · *common::task::promise_type::final_awaiter*

```cpp
promise_type& promise;
```

Stores the promise value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/task.h:57](../../../include/common/task.h#L57)

<a id="method-await-ready"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_ready

**METHOD** · *common::task::promise_type::final_awaiter*

```cpp
bool await_ready() noexcept;
```

Implements the coroutine await_ready hook.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:58](../../../include/common/task.h#L58)

<a id="method-await-suspend"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_suspend

**METHOD** · *common::task::promise_type::final_awaiter*

```cpp
void await_suspend(std::coroutine_handle<>) noexcept;
```

Implements the coroutine await_suspend hook.

**Parameters:**

- **coroutine_handle:** Coroutine handle to resume. Declaration: `std::coroutine_handle<>`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:66](../../../include/common/task.h#L66)

<a id="method-await-resume"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_resume

**METHOD** · *common::task::promise_type::final_awaiter*

```cpp
void await_resume() noexcept;
```

Implements the coroutine await_resume hook.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:67](../../../include/common/task.h#L67)

<a id="method-final-suspend"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> final_suspend

**METHOD** · *common::task::promise_type*

```cpp
final_awaiter final_suspend() noexcept;
```

Controls whether a coroutine suspends at completion.

**Returns:** A value of type `final_awaiter`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:69](../../../include/common/task.h#L69)

<a id="method-return-void"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> return_void

**METHOD** · *common::task::promise_type*

```cpp
void return_void() noexcept;
```

Completes a coroutine without a result value.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:70](../../../include/common/task.h#L70)

<a id="method-unhandled-exception"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unhandled_exception

**METHOD** · *common::task::promise_type*

```cpp
void unhandled_exception() noexcept;
```

Records an uncaught coroutine exception.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:71](../../../include/common/task.h#L71)

<a id="field-completion"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> completion

**FIELD** · *common::task::promise_type*

```cpp
std::function<void(std::exception_ptr)> completion;
```

Stores the completion value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/task.h:72](../../../include/common/task.h#L72)

<a id="field-error"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> error

**FIELD** · *common::task::promise_type*

```cpp
std::exception_ptr error;
```

Stores the error value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/task.h:73](../../../include/common/task.h#L73)

<a id="field-scheduler"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> scheduler

**FIELD** · *common::task::promise_type*

```cpp
task_scheduler scheduler;
```

Stores the scheduler value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/task.h:74](../../../include/common/task.h#L74)

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
