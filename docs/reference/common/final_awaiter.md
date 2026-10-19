<a id="final-awaiter"></a>
<a href="../../../resources/docs/reference/common/final_awaiter/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/final_awaiter/index.svg" alt="final_awaiter">
</a>
<a href="../../../resources/docs/reference/common/final_awaiter/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/final_awaiter/index-dark.svg" alt="final_awaiter">
</a>

**Header:** [include/common/task.h](../../../include/common/task.h)

**Declared in:** [promise_type](promise_type.md#type-promise-type)

Provides the final awaiter type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::task::promise_type

[final_awaiter](#type-final-awaiter)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::task::promise_type::final_awaiter

[promise](#field-promise) ·
[await_ready](#method-await-ready) ·
[await_suspend](#method-await-suspend) ·
[await_resume](#method-await-resume)

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
