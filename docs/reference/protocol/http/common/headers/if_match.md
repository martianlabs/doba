<a id="if-match"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/index.svg" alt="if_match">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/index-dark.svg" alt="if_match">
</a>

**Header:** [include/protocol/http/common/headers/if_match.h](../../../../../../include/protocol/http/common/headers/if_match.h)

|                                                                  if-match | | RFC 9110 S13.1.1 If-Match                                                 | | The "If-Match" header field makes the requested method conditional on the | | target resource having a current representation that satisfies the        | | supplied validator condition.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[if_match](#type-if-match)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::if_match

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-if-match"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> if_match

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class if_match { ... };
```

|                                                                  if-match | | RFC 9110 S13.1.1 If-Match                                                 | | The "If-Match" header field makes the requested method conditional on the | | target resource having a current representation that satisfies the        | | supplied validator condition.

**Source:** [include/protocol/http/common/headers/if_match.h:127](../../../../../../include/protocol/http/common/headers/if_match.h#L127)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::if_match*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP if-match header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/if_match.h:132](../../../../../../include/protocol/http/common/headers/if_match.h#L132)
