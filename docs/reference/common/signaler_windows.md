<a id="signaler-windows"></a>
<a href="../../../resources/docs/reference/common/signaler_windows/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/signaler_windows/title.svg" alt="signaler_windows">
</a>
<a href="../../../resources/docs/reference/common/signaler_windows/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/signaler_windows/title-dark.svg" alt="signaler_windows">
</a>

**Header:** [include/common/signaler_windows.h](../../../include/common/signaler_windows.h)

Provides the signaler type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[signaler](#type-signaler)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::signaler

[wait](#method-wait)

<a id="api"></a>
<a href="../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-signaler"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> signaler

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class signaler { ... };
```

Provides the signaler type.

**Source:** [include/common/signaler_windows.h](../../../include/common/signaler_windows.h)

<a id="method-wait"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> wait

**METHOD** · *common::signaler*

```cpp
static void wait();
```

Register a temporary console handler, then block until CTRL_C_EVENT or
CTRL_BREAK_EVENT requests shutdown. Other console events are not handled here.
The loop checks the shared flag every 50 ms; this is not an exact wakeup
deadline. The handler is removed before returning.

Use one application-owned wait at a time: its flag and console registration
are process-wide. It does not stop a server itself. Registration failure
throws `std::system_error`; removal errors are not reported.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::system_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/common/signaler_windows.h](../../../include/common/signaler_windows.h)
