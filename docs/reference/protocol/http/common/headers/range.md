<a id="range"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/range/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/range/index.svg" alt="range">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/range/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/range/index-dark.svg" alt="range">
</a>

**Header:** [include/protocol/http/common/headers/range.h](../../../../../../include/protocol/http/common/headers/range.h)

|                                                                     range | | RFC 9110 S14.2 Range                                                      | | The "Range" request header field modifies GET semantics to request the    | | transfer of only one or more subranges of the selected representation     | | data instead of the complete represe

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[range](#type-range)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::range

[check](#method-check)

<a id="type-range"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> range

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class range { ... };
```

|                                                                     range | | RFC 9110 S14.2 Range                                                      | | The "Range" request header field modifies GET semantics to request the    | | transfer of only one or more subranges of the selected representation     | | data instead of the complete represe

**Source:** [include/protocol/http/common/headers/range.h:136](../../../../../../include/protocol/http/common/headers/range.h#L136)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::range*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP range header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/range.h:141](../../../../../../include/protocol/http/common/headers/range.h#L141)
