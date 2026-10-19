<a id="contracts"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/contracts/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/contracts/index.svg" alt="contracts">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/auth/contracts/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/contracts/index-dark.svg" alt="contracts">
</a>

**Header:** [include/protocol/http/common/auth/contracts.h](../../../../../../include/protocol/http/common/auth/contracts.h)

This header declares the types and operations listed below.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::auth::contracts

[provider](#concept-provider)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="concept-provider"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> provider

**CONCEPT** · *protocol::http::auth::contracts*

```cpp
template <typename Aty, typename RQty>
concept provider = requires(const Aty& value, const RQty& request) {
  { value.authenticate(request) } -> std::same_as<authentication_result>;
};
```

Checks the compile-time requirements for provider.

**Template parameters:**

- **Aty:** `typename Aty`.
- **RQty:** `typename RQty`.

**Source:** [include/protocol/http/common/auth/contracts.h:39](../../../../../../include/protocol/http/common/auth/contracts.h#L39)
