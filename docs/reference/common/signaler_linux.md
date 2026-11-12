<a id="signaler-linux"></a>
<a href="../../../resources/docs/reference/common/signaler_linux/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/signaler_linux/title.svg" alt="signaler_linux">
</a>
<a href="../../../resources/docs/reference/common/signaler_linux/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/signaler_linux/title-dark.svg" alt="signaler_linux">
</a>

**Header:** [include/common/signaler_linux.h](../../../include/common/signaler_linux.h)

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

**Source:** [include/common/signaler_linux.h](../../../include/common/signaler_linux.h)

<a id="method-wait"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> wait

**METHOD** · *common::signaler*

```cpp
static void wait();
```

Install temporary handlers for SIGINT and SIGTERM, then block until either
requests shutdown. The loop checks the shared flag every 50 ms; this is not
an exact wakeup deadline. Previous handlers are restored before returning.

Use one application-owned wait at a time: its flag and signal handlers are
process-wide. It does not stop a server itself. Handler installation failure
throws `std::runtime_error`; restoration errors are not reported.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/common/signaler_linux.h](../../../include/common/signaler_linux.h)
