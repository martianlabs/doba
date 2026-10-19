<a id="if-range-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/if_range/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/if_range/index.svg" alt="Index for if_range.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/if_range.h](../../../../../../include/protocol/http/common/headers/if_range.h)

|                                                                  if-range | | RFC 9110 S13.1.5 If-Range                                                 | | The "If-Range" header field provides a conditional mechanism specifically | | for requests that also contain a Range header field. It allows a client   | | holding a partial representation to

**protocol::http::headers**

- [if_range](#type-if-range) · *type*

**protocol::http::headers::if_range**

- [check](#method-check) · *method*

<a id="type-if-range"></a>

## if_range

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class if_range { ... };
```

|                                                                  if-range | | RFC 9110 S13.1.5 If-Range                                                 | | The "If-Range" header field provides a conditional mechanism specifically | | for requests that also contain a Range header field. It allows a client   | | holding a partial representation to

**Source:** [include/protocol/http/common/headers/if_range.h:158](../../../../../../include/protocol/http/common/headers/if_range.h#L158)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::if_range*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP if-range header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/if_range.h:163](../../../../../../include/protocol/http/common/headers/if_range.h#L163)
