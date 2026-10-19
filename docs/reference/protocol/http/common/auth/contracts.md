<a id="contracts-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/auth/contracts/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/auth/contracts/index.svg" alt="Index for contracts.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/auth/contracts.h](../../../../../../include/protocol/http/common/auth/contracts.h)

This header declares the types and operations listed below.

**protocol::http::auth::contracts**

- [provider](#concept-provider) · *concept*

<a id="concept-provider"></a>

## provider

🟠 **CONCEPT** · *protocol::http::auth::contracts*

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
