<a id="if-none-match"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_none_match/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_none_match/index.svg" alt="if_none_match">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/if_none_match/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_none_match/index-dark.svg" alt="if_none_match">
</a>

**Header:** [include/protocol/http/common/headers/if_none_match.h](../../../../../../include/protocol/http/common/headers/if_none_match.h)

|                                                             if-none-match | | RFC 9110 S13.1.2 If-None-Match                                            | | The "If-None-Match" header field makes a request conditional on a         | | recipient cache or origin server either not having any current            | | representation of the target resourc

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[if_none_match](#type-if-none-match)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::if_none_match

[check](#method-check)

<a id="type-if-none-match"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> if_none_match

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class if_none_match { ... };
```

|                                                             if-none-match | | RFC 9110 S13.1.2 If-None-Match                                            | | The "If-None-Match" header field makes a request conditional on a         | | recipient cache or origin server either not having any current            | | representation of the target resourc

**Source:** [include/protocol/http/common/headers/if_none_match.h:113](../../../../../../include/protocol/http/common/headers/if_none_match.h#L113)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::if_none_match*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP if-none-match header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/if_none_match.h:118](../../../../../../include/protocol/http/common/headers/if_none_match.h#L118)
