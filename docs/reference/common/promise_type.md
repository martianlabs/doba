<a id="promise-type"></a>
<a href="../../../resources/docs/reference/common/promise_type/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/promise_type/index.svg" alt="promise_type">
</a>
<a href="../../../resources/docs/reference/common/promise_type/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/promise_type/index-dark.svg" alt="promise_type">
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

<a id="api-details"></a>
<a href="../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

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
