<a id="access-control-allow-headers"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_headers/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_headers/title.svg" alt="access_control_allow_headers">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_headers/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_headers/title-dark.svg" alt="access_control_allow_headers">
</a>

**Header:** [include/protocol/http/common/headers/access_control_allow_headers.h](../../../../../../include/protocol/http/common/headers/access_control_allow_headers.h)

|                                              access-control-allow-headers | | Fetch Standard S3.3.3 Access-Control-Allow-Headers                        | | The "Access-Control-Allow-Headers" response header field is used in a     | | CORS-preflight response to indicate which HTTP header fields are          | | supported by the response's URL for

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[access_control_allow_headers](#type-access-control-allow-headers)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::access_control_allow_headers

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-access-control-allow-headers"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_allow_headers

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_allow_headers { ... };
```

|                                              access-control-allow-headers | | Fetch Standard S3.3.3 Access-Control-Allow-Headers                        | | The "Access-Control-Allow-Headers" response header field is used in a     | | CORS-preflight response to indicate which HTTP header fields are          | | supported by the response's URL for

**Source:** [include/protocol/http/common/headers/access_control_allow_headers.h:91](../../../../../../include/protocol/http/common/headers/access_control_allow_headers.h#L91)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::access_control_allow_headers*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-allow-headers header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_allow_headers.h:96](../../../../../../include/protocol/http/common/headers/access_control_allow_headers.h#L96)
