<a id="accept-ranges"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/title.svg" alt="accept_ranges">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/title-dark.svg" alt="accept_ranges">
</a>

**Header:** [include/protocol/http/common/headers/accept_ranges.h](../../../../../../include/protocol/http/common/headers/accept_ranges.h)

|                                                             accept-ranges | | RFC 9110 S14.3 Accept-Ranges                                              | | The "Accept-Ranges" response header field indicates whether an upstream   | | server supports range requests for the target resource.                   | | A server that supports byte-range re

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[accept_ranges](#type-accept-ranges)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::accept_ranges

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-accept-ranges"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> accept_ranges

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class accept_ranges { ... };
```

|                                                             accept-ranges | | RFC 9110 S14.3 Accept-Ranges                                              | | The "Accept-Ranges" response header field indicates whether an upstream   | | server supports range requests for the target resource.                   | | A server that supports byte-range re

**Source:** [include/protocol/http/common/headers/accept_ranges.h:126](../../../../../../include/protocol/http/common/headers/accept_ranges.h#L126)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::accept_ranges*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP accept-ranges header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/accept_ranges.h:131](../../../../../../include/protocol/http/common/headers/accept_ranges.h#L131)
