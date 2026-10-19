<a id="target-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/target/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/target/index.svg" alt="Index for target.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/target.h](../../../../../include/protocol/http/common/target.h)

This header declares the types and operations listed below.

**protocol::http**

- [target](#enum-target) · *enum*

<a id="enum-target"></a>

## target

🟠 **ENUM** · *protocol::http*

```cpp
enum class target {
  kUnknown,
  kOriginForm,
  kAbsoluteForm,
  kAuthorityForm,
  kAsteriskForm
};
```

This enum class holds for the http 1.1 request-target types.

**Values:**

- **kUnknown:** Unknown. Declaration: `kUnknown`.
- **kOriginForm:** Origin form. Declaration: `kOriginForm`.
- **kAbsoluteForm:** Absolute form. Declaration: `kAbsoluteForm`.
- **kAuthorityForm:** Authority form. Declaration: `kAuthorityForm`.
- **kAsteriskForm:** Asterisk form. Declaration: `kAsteriskForm`.

**Source:** [include/protocol/http/common/target.h:36](../../../../../include/protocol/http/common/target.h#L36)
