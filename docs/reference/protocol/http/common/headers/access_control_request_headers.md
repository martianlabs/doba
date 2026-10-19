<a id="access-control-request-headers-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_headers/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_headers/index.svg" alt="Index for access_control_request_headers.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_headers/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_headers/index-dark.svg" alt="Index for access_control_request_headers.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/access_control_request_headers.h](../../../../../../include/protocol/http/common/headers/access_control_request_headers.h)

|                                            access-control-request-headers | | Fetch Standard S3.3.4 Access-Control-Request-Headers                      | | The "Access-Control-Request-Headers" header field is used by a user       | | agent when issuing a CORS-preflight request. It informs the server which  | | HTTP header field names the user age

**protocol::http::headers**

- [access_control_request_headers](#type-access-control-request-headers) · *type*

**protocol::http::headers::access_control_request_headers**

- [check](#method-check) · *method*

<a id="type-access-control-request-headers"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_request_headers

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_request_headers { ... };
```

|                                            access-control-request-headers | | Fetch Standard S3.3.4 Access-Control-Request-Headers                      | | The "Access-Control-Request-Headers" header field is used by a user       | | agent when issuing a CORS-preflight request. It informs the server which  | | HTTP header field names the user age

**Source:** [include/protocol/http/common/headers/access_control_request_headers.h:91](../../../../../../include/protocol/http/common/headers/access_control_request_headers.h#L91)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::access_control_request_headers*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-request-headers header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_request_headers.h:96](../../../../../../include/protocol/http/common/headers/access_control_request_headers.h#L96)
