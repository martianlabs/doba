<a id="last-modified"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/title.svg" alt="last_modified">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/last_modified/title-dark.svg" alt="last_modified">
</a>

**Header:** [include/protocol/http/common/headers/last_modified.h](../../../../../../include/protocol/http/common/headers/last_modified.h)

|                                                             last-modified | | RFC 9110 S8.8.2 Last-Modified                                             | | The "Last-Modified" header field in a response provides the date and time | | at which the origin server believes the selected representation was last  | | modified, as determined when request

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[last_modified](#type-last-modified)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::last_modified

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-last-modified"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> last_modified

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class last_modified { ... };
```

|                                                             last-modified | | RFC 9110 S8.8.2 Last-Modified                                             | | The "Last-Modified" header field in a response provides the date and time | | at which the origin server believes the selected representation was last  | | modified, as determined when request

**Source:** [include/protocol/http/common/headers/last_modified.h:132](../../../../../../include/protocol/http/common/headers/last_modified.h#L132)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::last_modified*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP last-modified header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/last_modified.h:137](../../../../../../include/protocol/http/common/headers/last_modified.h#L137)
