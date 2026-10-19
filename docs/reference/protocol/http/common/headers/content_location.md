<a id="content-location-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/index.svg" alt="Index for content_location.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/content_location.h](../../../../../../include/protocol/http/common/headers/content_location.h)

|                                                          content-location | | RFC 9110 S8.7 Content-Location                                            | | The "Content-Location" header field references a URI that identifies a    | | resource corresponding to the representation enclosed in the message.     | | The identified resource is not neces

**protocol::http::headers**

- [content_location](#type-content-location) · *type*

**protocol::http::headers::content_location**

- [check](#method-check) · *method*

<a id="type-content-location"></a>

## content_location

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class content_location { ... };
```

|                                                          content-location | | RFC 9110 S8.7 Content-Location                                            | | The "Content-Location" header field references a URI that identifies a    | | resource corresponding to the representation enclosed in the message.     | | The identified resource is not neces

**Source:** [include/protocol/http/common/headers/content_location.h:112](../../../../../../include/protocol/http/common/headers/content_location.h#L112)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::content_location*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP content-location header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/content_location.h:117](../../../../../../include/protocol/http/common/headers/content_location.h#L117)
