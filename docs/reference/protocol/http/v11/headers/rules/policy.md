<a id="policy"></a>
<a href="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/policy/title.svg#gh-light-mode-only">
  <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/policy/title.svg" alt="policy">
</a>
<a href="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/policy/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/policy/title-dark.svg" alt="policy">
</a>

**Header:** [include/protocol/http/v11/headers/rules/policy.h](../../../../../../../include/protocol/http/v11/headers/rules/policy.h)

Inbound policy limits that span several headers Transversal policy rules that enforce limits no single intra-header interpreter can, because the quantity being limited is an aggregate of several headers. Applied by deserialize() after the single header pass has populated the context.

### <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::rules

[policy](#type-policy)

### <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::rules::policy

[apply](#method-apply)

<a id="api"></a>
<a href="../../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-policy"></a>

## <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> policy

**TYPE** · *protocol::http::v11::headers::rules*

**Declaration synopsis:**

```cpp
class policy { ... };
```

Inbound policy limits that span several headers Transversal policy rules that enforce limits no single intra-header interpreter can, because the quantity being limited is an aggregate of several headers. Applied by deserialize() after the single header pass has populated the context.

**Source:** [include/protocol/http/v11/headers/rules/policy.h:51](../../../../../../../include/protocol/http/v11/headers/rules/policy.h#L51)

<a id="method-apply"></a>

## <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> apply

**METHOD** · *protocol::http::v11::headers::rules::policy*

```cpp
static constexpr verdict apply(const context& ctx);
```

Applies these rules to the supplied state.

**Parameters:**

- **ctx:** Context for this operation. Declaration: `const context& ctx`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/rules/policy.h:56](../../../../../../../include/protocol/http/v11/headers/rules/policy.h#L56)
