<a id="signaler-linux"></a>
<a href="../../../resources/docs/reference/common/signaler_linux/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/signaler_linux/index.svg" alt="signaler_linux">
</a>
<a href="../../../resources/docs/reference/common/signaler_linux/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/signaler_linux/index-dark.svg" alt="signaler_linux">
</a>

**Header:** [include/common/signaler_linux.h](../../../include/common/signaler_linux.h)

Provides the signaler type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[signaler](#type-signaler)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::signaler

[wait](#method-wait)

<a id="api-details"></a>
<a href="../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-signaler"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> signaler

**TYPE** · *common*

**Declaration synopsis:**

```cpp
class signaler { ... };
```

Provides the signaler type.

**Source:** [include/common/signaler_linux.h:40](../../../include/common/signaler_linux.h#L40)

<a id="method-wait"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> wait

**METHOD** · *common::signaler*

```cpp
static void wait();
```

Waits for the pending operation or event.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/common/signaler_linux.h:45](../../../include/common/signaler_linux.h#L45)
