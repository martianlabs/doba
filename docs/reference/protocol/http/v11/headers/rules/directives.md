<a id="directives-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/directives/index-dark.svg">
    <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/directives/index.svg" alt="Index for directives.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/headers/rules/directives.h](../../../../../../../include/protocol/http/v11/headers/rules/directives.h)

The transversal "connection directives" rules: they relate the Connection option tokens (the hop-by-hop directives) to other headers, which the intra-header Connection interpreter cannot check alone because it never sees those other headers. Applied by deserialize() after the single header pass has populated the context. Named "directives" (not

**protocol::http::v11::headers::rules**

- [directives](#type-directives) · *type*

**protocol::http::v11::headers::rules::directives**

- [apply](#method-apply) · *method*

<a id="type-directives"></a>

## directives

🟠 **TYPE** · *protocol::http::v11::headers::rules*

**Declaration synopsis:**

```cpp
class directives { ... };
```

The transversal "connection directives" rules: they relate the Connection option tokens (the hop-by-hop directives) to other headers, which the intra-header Connection interpreter cannot check alone because it never sees those other headers. Applied by deserialize() after the single header pass has populated the context. Named "directives" (not

**Source:** [include/protocol/http/v11/headers/rules/directives.h:59](../../../../../../../include/protocol/http/v11/headers/rules/directives.h#L59)

<a id="method-apply"></a>

## apply

🟠 **METHOD** · *protocol::http::v11::headers::rules::directives*

```cpp
static constexpr verdict apply(const context& ctx);
```

Applies these rules to the supplied state.

**Parameters:**

- **ctx:** Context for this operation. Declaration: `const context& ctx`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/rules/directives.h:64](../../../../../../../include/protocol/http/v11/headers/rules/directives.h#L64)
