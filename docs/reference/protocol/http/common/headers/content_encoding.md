<a id="content-encoding-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/content_encoding/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_encoding/index.svg" alt="Index for content_encoding.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/content_encoding.h](../../../../../../include/protocol/http/common/headers/content_encoding.h)

|                                                          content-encoding | | RFC 9110 S8.4 Content-Encoding                                            | | The "Content-Encoding" header field indicates which content codings have  | | been applied to a representation beyond those inherent in its media type. | | It therefore identifies the decoding

**protocol::http::headers**

- [content_encoding](#type-content-encoding) · *type*

**protocol::http::headers::content_encoding**

- [check](#method-check) · *method*

<a id="type-content-encoding"></a>

## content_encoding

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class content_encoding { ... };
```

|                                                          content-encoding | | RFC 9110 S8.4 Content-Encoding                                            | | The "Content-Encoding" header field indicates which content codings have  | | been applied to a representation beyond those inherent in its media type. | | It therefore identifies the decoding

**Source:** [include/protocol/http/common/headers/content_encoding.h:122](../../../../../../include/protocol/http/common/headers/content_encoding.h#L122)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::content_encoding*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP content-encoding header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/content_encoding.h:127](../../../../../../include/protocol/http/common/headers/content_encoding.h#L127)
