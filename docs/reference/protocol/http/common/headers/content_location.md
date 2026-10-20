<a id="content-location"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/title.svg" alt="content_location">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/content_location/title-dark.svg" alt="content_location">
</a>

**Header:** [include/protocol/http/common/headers/content_location.h](../../../../../../include/protocol/http/common/headers/content_location.h)

|                                                          content-location | | RFC 9110 S8.7 Content-Location                                            | | The "Content-Location" header field references a URI that identifies a    | | resource corresponding to the representation enclosed in the message.     | | The identified resource is not neces

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[content_location](#type-content-location)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::content_location

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-content-location"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> content_location

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class content_location { ... };
```

|                                                          content-location | | RFC 9110 S8.7 Content-Location                                            | | The "Content-Location" header field references a URI that identifies a    | | resource corresponding to the representation enclosed in the message.     | | The identified resource is not neces

**Source:** [include/protocol/http/common/headers/content_location.h:112](../../../../../../include/protocol/http/common/headers/content_location.h#L112)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::content_location*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP content-location header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/content_location.h:117](../../../../../../include/protocol/http/common/headers/content_location.h#L117)
