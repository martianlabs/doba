<a id="content-range"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/title.svg" alt="content_range">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/title-dark.svg" alt="content_range">
</a>

**Header:** [include/protocol/http/common/headers/content_range.h](../../../../../../include/protocol/http/common/headers/content_range.h)

|                                                             content-range | | RFC 9110 S14.4 Content-Range                                              | | The "Content-Range" header field identifies which range of a selected     | | representation is enclosed in a message or body part.                     | | It is sent in a single-part 206 (Par

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[content_range](#type-content-range)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::content_range

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-content-range"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> content_range

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class content_range { ... };
```

|                                                             content-range | | RFC 9110 S14.4 Content-Range                                              | | The "Content-Range" header field identifies which range of a selected     | | representation is enclosed in a message or body part.                     | | It is sent in a single-part 206 (Par

**Source:** [include/protocol/http/common/headers/content_range.h:111](../../../../../../include/protocol/http/common/headers/content_range.h#L111)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::content_range*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP content-range header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/content_range.h:116](../../../../../../include/protocol/http/common/headers/content_range.h#L116)
