<a id="task-awaiters"></a>
<a href="../../../resources/docs/reference/common/task_awaiters/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/task_awaiters/index.svg" alt="task_awaiters">
</a>
<a href="../../../resources/docs/reference/common/task_awaiters/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/task_awaiters/index-dark.svg" alt="task_awaiters">
</a>

**Header:** [include/common/task_awaiters.h](../../../include/common/task_awaiters.h)

Provides the yield awaiter type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[yield_awaiter](#type-yield-awaiter) ·
[yield](#function-yield) ·
[sleep_awaiter](#type-sleep-awaiter) ·
[sleep_for](#function-sleep-for)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::yield_awaiter

[await_ready](#method-await-ready) ·
[await_suspend](#method-await-suspend) ·
[await_resume](#method-await-resume)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::sleep_awaiter

[duration](#field-duration) ·
[await_ready](#method-await-ready-2) ·
[await_suspend](#method-await-suspend-2) ·
[await_resume](#method-await-resume-2)

<a id="type-yield-awaiter"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> yield_awaiter

**TYPE** · *common*

**Declaration synopsis:**

```cpp
struct yield_awaiter { ... };
```

Provides the yield awaiter type.

**Source:** [include/common/task_awaiters.h:40](../../../include/common/task_awaiters.h#L40)

<a id="method-await-ready"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_ready

**METHOD** · *common::yield_awaiter*

```cpp
bool await_ready() const noexcept;
```

Implements the coroutine await_ready hook.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task_awaiters.h:44](../../../include/common/task_awaiters.h#L44)

<a id="method-await-suspend"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_suspend

**METHOD** · *common::yield_awaiter*

```cpp
template <typename Pty>
  void await_suspend(std::coroutine_handle<Pty> handle) const;
```

Implements the coroutine await_suspend hook.

**Template parameters:**

- **Pty:** `typename Pty`.

**Parameters:**

- **handle:** Handle to the coroutine or operating-system object. Declaration: `std::coroutine_handle<Pty> handle`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/common/task_awaiters.h:49](../../../include/common/task_awaiters.h#L49)

<a id="method-await-resume"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_resume

**METHOD** · *common::yield_awaiter*

```cpp
void await_resume() const noexcept;
```

Implements the coroutine await_resume hook.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/task_awaiters.h:57](../../../include/common/task_awaiters.h#L57)

<a id="function-yield"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> yield

**FUNCTION** · *common*

```cpp
inline yield_awaiter yield() noexcept;
```

Yields a value to the awaiting coroutine.

**Returns:** A value of type `yield_awaiter`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task_awaiters.h:65](../../../include/common/task_awaiters.h#L65)

<a id="type-sleep-awaiter"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sleep_awaiter

**TYPE** · *common*

**Declaration synopsis:**

```cpp
struct sleep_awaiter { ... };
```

Provides the sleep awaiter type.

**Source:** [include/common/task_awaiters.h:72](../../../include/common/task_awaiters.h#L72)

<a id="field-duration"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> duration

**FIELD** · *common::sleep_awaiter*

```cpp
std::chrono::steady_clock::duration duration;
```

Stores the duration value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/task_awaiters.h:76](../../../include/common/task_awaiters.h#L76)

<a id="method-await-ready-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_ready

**METHOD** · *common::sleep_awaiter*

```cpp
bool await_ready() const noexcept;
```

Implements the coroutine await_ready hook.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task_awaiters.h:80](../../../include/common/task_awaiters.h#L80)

<a id="method-await-suspend-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_suspend

**METHOD** · *common::sleep_awaiter*

```cpp
template <typename Pty>
  void await_suspend(std::coroutine_handle<Pty> handle) const;
```

Implements the coroutine await_suspend hook.

**Template parameters:**

- **Pty:** `typename Pty`.

**Parameters:**

- **handle:** Handle to the coroutine or operating-system object. Declaration: `std::coroutine_handle<Pty> handle`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/common/task_awaiters.h:87](../../../include/common/task_awaiters.h#L87)

<a id="method-await-resume-2"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> await_resume

**METHOD** · *common::sleep_awaiter*

```cpp
void await_resume() const noexcept;
```

Implements the coroutine await_resume hook.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/common/task_awaiters.h:96](../../../include/common/task_awaiters.h#L96)

<a id="function-sleep-for"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sleep_for

**FUNCTION** · *common*

```cpp
inline sleep_awaiter sleep_for(
    std::chrono::steady_clock::duration duration) noexcept;
```

Suspends for the requested duration.

**Parameters:**

- **duration:** Duration to wait or schedule. Declaration: `std::chrono::steady_clock::duration duration`.

**Returns:** A value of type `sleep_awaiter`.

**Errors:** Declared `noexcept`.

**Source:** [include/common/task_awaiters.h:104](../../../include/common/task_awaiters.h#L104)
