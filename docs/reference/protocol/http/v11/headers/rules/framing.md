<a id="framing"></a>
<a href="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/framing/title.svg#gh-light-mode-only">
  <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/framing/title.svg" alt="framing">
</a>
<a href="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/framing/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../../resources/docs/reference/protocol/http/v11/headers/rules/framing/title-dark.svg" alt="framing">
</a>

**Header:** [include/protocol/http/v11/headers/rules/framing.h](../../../../../../../include/protocol/http/v11/headers/rules/framing.h)

Transversal framing rules that span Content-Length and Transfer-Encoding, which no single intra-header interpreter can decide because they need both headers at once. Applied by the coordinator after every intra-header interpreter has run and the context has been populated.

### <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::rules

[framing](#type-framing)

### <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::rules::framing

[apply](#method-apply)

<a id="api"></a>
<a href="../../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-framing"></a>

## <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> framing

**TYPE** · *protocol::http::v11::headers::rules*

**Declaration synopsis:**

```cpp
class framing { ... };
```

Transversal framing rules that span Content-Length and Transfer-Encoding, which no single intra-header interpreter can decide because they need both headers at once. Applied by the coordinator after every intra-header interpreter has run and the context has been populated.

**Source:** [include/protocol/http/v11/headers/rules/framing.h:57](../../../../../../../include/protocol/http/v11/headers/rules/framing.h#L57)

<a id="method-apply"></a>

## <a href="../../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> apply

**METHOD** · *protocol::http::v11::headers::rules::framing*

```cpp
static constexpr verdict apply(const context& ctx);
```

Applies these rules to the supplied state.

**Parameters:**

- **ctx:** Context for this operation. Declaration: `const context& ctx`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/rules/framing.h:62](../../../../../../../include/protocol/http/v11/headers/rules/framing.h#L62)
