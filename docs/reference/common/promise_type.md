<a id="promise-type"></a>
<a href="../../../resources/docs/reference/common/promise_type/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/promise_type/title.svg" alt="promise_type">
</a>
<a href="../../../resources/docs/reference/common/promise_type/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/promise_type/title-dark.svg" alt="promise_type">
</a>

**Header:** [include/common/task.h](../../../include/common/task.h)

**Declared in:** [task](task.md#type-task)

Provides the promise type type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::task

[promise_type](#type-promise-type)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::task::promise_type

[final_awaiter](final_awaiter.md) ·
[get_return_object](#method-get-return-object) ·
[initial_suspend](#method-initial-suspend) ·
[final_suspend](#method-final-suspend) ·
[return_void](#method-return-void) ·
[unhandled_exception](#method-unhandled-exception) ·
[completion](#field-completion) ·
[error](#field-error) ·
[scheduler](#field-scheduler)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-promise-type"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> promise_type

**TYPE** · *common::task*

**Declaration synopsis:**

```cpp
struct promise_type { ... };
```

Provides the promise type type.

**Source:** [include/common/task.h:53](../../../include/common/task.h#L53)

<a id="method-get-return-object"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_return_object

**METHOD** · *common::task::promise_type*

```cpp
task get_return_object() noexcept;
```

Returns return object.

**Returns:** A value of type `task`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:54](../../../include/common/task.h#L54)

<a id="method-initial-suspend"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> initial_suspend

**METHOD** · *common::task::promise_type*

```cpp
std::suspend_always initial_suspend() noexcept;
```

Controls whether a new coroutine suspends initially.

**Returns:** A value of type `std::suspend_always`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:57](../../../include/common/task.h#L57)

<a id="method-final-suspend"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> final_suspend

**METHOD** · *common::task::promise_type*

```cpp
final_awaiter final_suspend() noexcept;
```

Controls whether a coroutine suspends at completion.

**Returns:** A value of type `final_awaiter`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:72](../../../include/common/task.h#L72)

<a id="method-return-void"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> return_void

**METHOD** · *common::task::promise_type*

```cpp
void return_void() noexcept;
```

Completes a coroutine without a result value.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:73](../../../include/common/task.h#L73)

<a id="method-unhandled-exception"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> unhandled_exception

**METHOD** · *common::task::promise_type*

```cpp
void unhandled_exception() noexcept;
```

Records an uncaught coroutine exception.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/task.h:74](../../../include/common/task.h#L74)

<a id="field-completion"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> completion

**FIELD** · *common::task::promise_type*

```cpp
std::function<void(std::exception_ptr)> completion;
```

Stores the completion value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/task.h:83](../../../include/common/task.h#L83)

<a id="field-error"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> error

**FIELD** · *common::task::promise_type*

```cpp
std::exception_ptr error;
```

Stores the error value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/task.h:84](../../../include/common/task.h#L84)

<a id="field-scheduler"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> scheduler

**FIELD** · *common::task::promise_type*

```cpp
task_scheduler scheduler;
```

Stores the scheduler value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/task.h:85](../../../include/common/task.h#L85)

## Controlled ownership

The promise also has `bool controlled{false}`. It is an internal lifetime flag:
`task_execution` sets it when adopting the unstarted frame. Callers must not
modify promise fields directly. `completion` is unused under controlled
ownership; `error` remains available at final suspension. `scheduler` points
to the stable owner and only records local readiness/deadline state.

`await_transform(Aty&&)` forwards ordinary await expressions unchanged for
standalone tasks. Under controlled ownership it throws `std::logic_error` for
all types except `yield_awaiter` and `sleep_awaiter`, before their awaiter hooks
can publish a handle. The coroutine's normal exception handling applies.
See [the ownership contract](task_execution.md).
