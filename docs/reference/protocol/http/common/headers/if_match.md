<a id="if-match-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_match/index.svg" alt="Index for if_match.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/if_match.h](../../../../../../include/protocol/http/common/headers/if_match.h)

|                                                                  if-match | | RFC 9110 S13.1.1 If-Match                                                 | | The "If-Match" header field makes the requested method conditional on the | | target resource having a current representation that satisfies the        | | supplied validator condition.

**protocol::http::headers**

- [if_match](#type-if-match) · *type*

**protocol::http::headers::if_match**

- [check](#method-check) · *method*

<a id="type-if-match"></a>

## if_match

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class if_match { ... };
```

|                                                                  if-match | | RFC 9110 S13.1.1 If-Match                                                 | | The "If-Match" header field makes the requested method conditional on the | | target resource having a current representation that satisfies the        | | supplied validator condition.

**Source:** [include/protocol/http/common/headers/if_match.h:127](../../../../../../include/protocol/http/common/headers/if_match.h#L127)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::if_match*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP if-match header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/if_match.h:132](../../../../../../include/protocol/http/common/headers/if_match.h#L132)
