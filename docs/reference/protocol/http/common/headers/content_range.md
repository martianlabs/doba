<a id="content-range-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_range/index.svg" alt="Index for content_range.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/content_range.h](../../../../../../include/protocol/http/common/headers/content_range.h)

|                                                             content-range | | RFC 9110 S14.4 Content-Range                                              | | The "Content-Range" header field identifies which range of a selected     | | representation is enclosed in a message or body part.                     | | It is sent in a single-part 206 (Par

**protocol::http::headers**

- [content_range](#type-content-range) · *type*

**protocol::http::headers::content_range**

- [check](#method-check) · *method*

<a id="type-content-range"></a>

## content_range

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class content_range { ... };
```

|                                                             content-range | | RFC 9110 S14.4 Content-Range                                              | | The "Content-Range" header field identifies which range of a selected     | | representation is enclosed in a message or body part.                     | | It is sent in a single-part 206 (Par

**Source:** [include/protocol/http/common/headers/content_range.h:111](../../../../../../include/protocol/http/common/headers/content_range.h#L111)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::content_range*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP content-range header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/content_range.h:116](../../../../../../include/protocol/http/common/headers/content_range.h#L116)
