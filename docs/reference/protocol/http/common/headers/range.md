<a id="range-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/range/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/range/index.svg" alt="Index for range.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/range.h](../../../../../../include/protocol/http/common/headers/range.h)

|                                                                     range | | RFC 9110 S14.2 Range                                                      | | The "Range" request header field modifies GET semantics to request the    | | transfer of only one or more subranges of the selected representation     | | data instead of the complete represe

**protocol::http::headers**

- [range](#type-range) · *type*

**protocol::http::headers::range**

- [check](#method-check) · *method*

<a id="type-range"></a>

## range

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class range { ... };
```

|                                                                     range | | RFC 9110 S14.2 Range                                                      | | The "Range" request header field modifies GET semantics to request the    | | transfer of only one or more subranges of the selected representation     | | data instead of the complete represe

**Source:** [include/protocol/http/common/headers/range.h:136](../../../../../../include/protocol/http/common/headers/range.h#L136)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::range*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP range header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/range.h:141](../../../../../../include/protocol/http/common/headers/range.h#L141)
