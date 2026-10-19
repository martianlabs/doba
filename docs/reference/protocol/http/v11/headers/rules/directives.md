<a id="directives"></a>
<a href="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/directives/index.svg#gh-light-mode-only">
  <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/directives/index.svg" alt="directives">
</a>
<a href="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/directives/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/directives/index-dark.svg" alt="directives">
</a>

**Header:** [include/protocol/http/v11/headers/rules/directives.h](../../../../../../../include/protocol/http/v11/headers/rules/directives.h)

The transversal "connection directives" rules: they relate the Connection option tokens (the hop-by-hop directives) to other headers, which the intra-header Connection interpreter cannot check alone because it never sees those other headers. Applied by deserialize() after the single header pass has populated the context. Named "directives" (not

### <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::rules

[directives](#type-directives)

### <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::rules::directives

[apply](#method-apply)

<a id="api-details"></a>
<a href="../../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-directives"></a>

## <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> directives

**TYPE** · *protocol::http::v11::headers::rules*

**Declaration synopsis:**

```cpp
class directives { ... };
```

The transversal "connection directives" rules: they relate the Connection option tokens (the hop-by-hop directives) to other headers, which the intra-header Connection interpreter cannot check alone because it never sees those other headers. Applied by deserialize() after the single header pass has populated the context. Named "directives" (not

**Source:** [include/protocol/http/v11/headers/rules/directives.h:59](../../../../../../../include/protocol/http/v11/headers/rules/directives.h#L59)

<a id="method-apply"></a>

## <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> apply

**METHOD** · *protocol::http::v11::headers::rules::directives*

```cpp
static constexpr verdict apply(const context& ctx);
```

Applies these rules to the supplied state.

**Parameters:**

- **ctx:** Context for this operation. Declaration: `const context& ctx`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/rules/directives.h:64](../../../../../../../include/protocol/http/v11/headers/rules/directives.h#L64)
