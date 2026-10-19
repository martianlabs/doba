<a id="policy-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/policy/index-dark.svg">
    <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/policy/index.svg" alt="Index for policy.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/headers/rules/policy.h](../../../../../../../include/protocol/http/v11/headers/rules/policy.h)

Inbound policy limits that span several headers Transversal policy rules that enforce limits no single intra-header interpreter can, because the quantity being limited is an aggregate of several headers. Applied by deserialize() after the single header pass has populated the context.

**protocol::http::v11::headers::rules**

- [policy](#type-policy) · *type*

**protocol::http::v11::headers::rules::policy**

- [apply](#method-apply) · *method*

<a id="type-policy"></a>

## policy

🟠 **TYPE** · *protocol::http::v11::headers::rules*

**Declaration synopsis:**

```cpp
class policy { ... };
```

Inbound policy limits that span several headers Transversal policy rules that enforce limits no single intra-header interpreter can, because the quantity being limited is an aggregate of several headers. Applied by deserialize() after the single header pass has populated the context.

**Source:** [include/protocol/http/v11/headers/rules/policy.h:51](../../../../../../../include/protocol/http/v11/headers/rules/policy.h#L51)

<a id="method-apply"></a>

## apply

🟠 **METHOD** · *protocol::http::v11::headers::rules::policy*

```cpp
static constexpr verdict apply(const context& ctx);
```

Applies these rules to the supplied state.

**Parameters:**

- **ctx:** Context for this operation. Declaration: `const context& ctx`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/rules/policy.h:56](../../../../../../../include/protocol/http/v11/headers/rules/policy.h#L56)
