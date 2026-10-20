<a id="content-encoding"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_encoding/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_encoding/title.svg" alt="content_encoding">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_encoding/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_encoding/title-dark.svg" alt="content_encoding">
</a>

**Header:** [include/protocol/http/common/headers/content_encoding.h](../../../../../../include/protocol/http/common/headers/content_encoding.h)

|                                                          content-encoding | | RFC 9110 S8.4 Content-Encoding                                            | | The "Content-Encoding" header field indicates which content codings have  | | been applied to a representation beyond those inherent in its media type. | | It therefore identifies the decoding

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[content_encoding](#type-content-encoding)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::content_encoding

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-content-encoding"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> content_encoding

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class content_encoding { ... };
```

|                                                          content-encoding | | RFC 9110 S8.4 Content-Encoding                                            | | The "Content-Encoding" header field indicates which content codings have  | | been applied to a representation beyond those inherent in its media type. | | It therefore identifies the decoding

**Source:** [include/protocol/http/common/headers/content_encoding.h:122](../../../../../../include/protocol/http/common/headers/content_encoding.h#L122)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::content_encoding*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP content-encoding header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/content_encoding.h:127](../../../../../../include/protocol/http/common/headers/content_encoding.h#L127)
