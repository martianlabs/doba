<a id="routing-h"></a>
<h1>
  <a href="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/routing/index.svg#gh-light-mode-only">
    <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/routing/index.svg" alt="Index for routing.h">
  </a>
  <a href="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/routing/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/routing/index-dark.svg" alt="Index for routing.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/headers/rules/routing.h](../../../../../../../include/protocol/http/v11/headers/rules/routing.h)

Transversal routing rules that reconcile the request-target with the Host header, which the intra-header Host interpreter cannot decide alone because it never sees the request line. Applied by deserialize() after the single header pass has populated the context.

**protocol::http::v11::headers::rules**

- [routing](#type-routing) · *type*

**protocol::http::v11::headers::rules::routing**

- [apply](#method-apply) · *method*

<a id="type-routing"></a>

## <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> routing

**TYPE** · *protocol::http::v11::headers::rules*

**Declaration synopsis:**

```cpp
class routing { ... };
```

Transversal routing rules that reconcile the request-target with the Host header, which the intra-header Host interpreter cannot decide alone because it never sees the request line. Applied by deserialize() after the single header pass has populated the context.

**Source:** [include/protocol/http/v11/headers/rules/routing.h:55](../../../../../../../include/protocol/http/v11/headers/rules/routing.h#L55)

<a id="method-apply"></a>

## <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> apply

**METHOD** · *protocol::http::v11::headers::rules::routing*

```cpp
static constexpr verdict apply(const context& ctx);
```

Applies these rules to the supplied state.

**Parameters:**

- **ctx:** Context for this operation. Declaration: `const context& ctx`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/rules/routing.h:60](../../../../../../../include/protocol/http/v11/headers/rules/routing.h#L60)
