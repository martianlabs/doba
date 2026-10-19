<a id="allow"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/allow/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/allow/index.svg" alt="allow">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/allow/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/allow/index-dark.svg" alt="allow">
</a>

**Header:** [include/protocol/http/common/headers/allow.h](../../../../../../include/protocol/http/common/headers/allow.h)

|                                                                     allow | | RFC 9110 S10.2.1 Allow                                                    | | The "Allow" header field lists the set of methods advertised as supported | | by the target resource. Its purpose is strictly to inform the recipient   | | of valid request methods associated

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[allow](#type-allow)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::allow

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-allow"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> allow

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class allow { ... };
```

|                                                                     allow | | RFC 9110 S10.2.1 Allow                                                    | | The "Allow" header field lists the set of methods advertised as supported | | by the target resource. Its purpose is strictly to inform the recipient   | | of valid request methods associated

**Source:** [include/protocol/http/common/headers/allow.h:94](../../../../../../include/protocol/http/common/headers/allow.h#L94)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::allow*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP allow header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/allow.h:99](../../../../../../include/protocol/http/common/headers/allow.h#L99)
