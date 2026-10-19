<a id="signaler-linux-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/signaler_linux/index-dark.svg">
    <img src="../../../resources/docs/reference/common/signaler_linux/index.svg" alt="Index for signaler_linux.h">
  </picture>
</h1>

**Header:** [include/common/signaler_linux.h](../../../include/common/signaler_linux.h)

Provides the signaler type.

**common**

- [signaler](#type-signaler) · *type*

**common::signaler**

- [wait](#method-wait) · *method*

<a id="type-signaler"></a>

## signaler

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
class signaler { ... };
```

Provides the signaler type.

**Source:** [include/common/signaler_linux.h:40](../../../include/common/signaler_linux.h#L40)

<a id="method-wait"></a>

## wait

🟠 **METHOD** · *common::signaler*

```cpp
static void wait();
```

Waits for the pending operation or event.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::runtime_error`. Calls made by this operation may propagate additional errors.

**Source:** [include/common/signaler_linux.h:45](../../../include/common/signaler_linux.h#L45)
