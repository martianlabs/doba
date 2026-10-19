<a id="accept-ranges-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/accept_ranges/index.svg" alt="Index for accept_ranges.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/accept_ranges.h](../../../../../../include/protocol/http/common/headers/accept_ranges.h)

|                                                             accept-ranges | | RFC 9110 S14.3 Accept-Ranges                                              | | The "Accept-Ranges" response header field indicates whether an upstream   | | server supports range requests for the target resource.                   | | A server that supports byte-range re

**protocol::http::headers**

- [accept_ranges](#type-accept-ranges) · *type*

**protocol::http::headers::accept_ranges**

- [check](#method-check) · *method*

<a id="type-accept-ranges"></a>

## accept_ranges

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class accept_ranges { ... };
```

|                                                             accept-ranges | | RFC 9110 S14.3 Accept-Ranges                                              | | The "Accept-Ranges" response header field indicates whether an upstream   | | server supports range requests for the target resource.                   | | A server that supports byte-range re

**Source:** [include/protocol/http/common/headers/accept_ranges.h:126](../../../../../../include/protocol/http/common/headers/accept_ranges.h#L126)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::accept_ranges*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP accept-ranges header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/accept_ranges.h:131](../../../../../../include/protocol/http/common/headers/accept_ranges.h#L131)
