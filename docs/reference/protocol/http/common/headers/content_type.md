<a id="content-type-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/content_type/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_type/index.svg" alt="Index for content_type.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/content_type.h](../../../../../../include/protocol/http/common/headers/content_type.h)

|                                                              content-type | | RFC 9110 S8.3 Content-Type                                                | | The "Content-Type" header field indicates the media type of the           | | associated representation. It identifies both the representation's data   | | format and how that data is intended

**protocol::http::headers**

- [content_type](#type-content-type) · *type*

**protocol::http::headers::content_type**

- [check](#method-check) · *method*

<a id="type-content-type"></a>

## content_type

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class content_type { ... };
```

|                                                              content-type | | RFC 9110 S8.3 Content-Type                                                | | The "Content-Type" header field indicates the media type of the           | | associated representation. It identifies both the representation's data   | | format and how that data is intended

**Source:** [include/protocol/http/common/headers/content_type.h:122](../../../../../../include/protocol/http/common/headers/content_type.h#L122)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::content_type*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP content-type header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/content_type.h:127](../../../../../../include/protocol/http/common/headers/content_type.h#L127)
