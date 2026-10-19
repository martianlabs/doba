<a id="target"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/target/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/target/index.svg" alt="target">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/target/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/target/index-dark.svg" alt="target">
</a>

**Header:** [include/protocol/http/common/target.h](../../../../../include/protocol/http/common/target.h)

This header declares the types and operations listed below.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[target](#enum-target)

<a id="api-details"></a>
<a href="../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="enum-target"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> target

**ENUM** · *protocol::http*

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
