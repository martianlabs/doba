<a id="access-control-request-method"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_method/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_method/index.svg" alt="access_control_request_method">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_method/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_request_method/index-dark.svg" alt="access_control_request_method">
</a>

**Header:** [include/protocol/http/common/headers/access_control_request_method.h](../../../../../../include/protocol/http/common/headers/access_control_request_method.h)

|                                             access-control-request-method | | Fetch S3.3.2 Access-Control-Request-Method                                | | The "Access-Control-Request-Method" request header field is used by a     | | user agent in a CORS-preflight request to indicate which HTTP method a    | | future CORS request to the same reso

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[access_control_request_method](#type-access-control-request-method)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::access_control_request_method

[check](#method-check)

<a id="type-access-control-request-method"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_request_method

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_request_method { ... };
```

|                                             access-control-request-method | | Fetch S3.3.2 Access-Control-Request-Method                                | | The "Access-Control-Request-Method" request header field is used by a     | | user agent in a CORS-preflight request to indicate which HTTP method a    | | future CORS request to the same reso

**Source:** [include/protocol/http/common/headers/access_control_request_method.h:85](../../../../../../include/protocol/http/common/headers/access_control_request_method.h#L85)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::access_control_request_method*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-request-method header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_request_method.h:90](../../../../../../include/protocol/http/common/headers/access_control_request_method.h#L90)
