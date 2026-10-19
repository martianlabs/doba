<a id="accept-ranges-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/index.svg" alt="Index for accept_ranges.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/index-dark.svg" alt="Index for accept_ranges.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/accept_ranges.h](../../../../../../include/protocol/http/common/headers/accept_ranges.h)

|                                                             accept-ranges | | RFC 9110 S14.3 Accept-Ranges                                              | | The "Accept-Ranges" response header field indicates whether an upstream   | | server supports range requests for the target resource.                   | | A server that supports byte-range re

**protocol::http::headers**

- [accept_ranges](#type-accept-ranges) · *type*

**protocol::http::headers::accept_ranges**

- [check](#method-check) · *method*

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
