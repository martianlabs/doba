<a id="access-control-allow-methods"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_methods/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_methods/index.svg" alt="access_control_allow_methods">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_methods/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_methods/index-dark.svg" alt="access_control_allow_methods">
</a>

**Header:** [include/protocol/http/common/headers/access_control_allow_methods.h](../../../../../../include/protocol/http/common/headers/access_control_allow_methods.h)

|                                              access-control-allow-methods | | Fetch Standard S3.3.3 HTTP responses                                      | | The "Access-Control-Allow-Methods" response header field indicates which  | | HTTP methods are supported by the response URL for the purposes of the    | | CORS protocol.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[access_control_allow_methods](#type-access-control-allow-methods)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::access_control_allow_methods

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-access-control-allow-methods"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_allow_methods

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_allow_methods { ... };
```

|                                              access-control-allow-methods | | Fetch Standard S3.3.3 HTTP responses                                      | | The "Access-Control-Allow-Methods" response header field indicates which  | | HTTP methods are supported by the response URL for the purposes of the    | | CORS protocol.

**Source:** [include/protocol/http/common/headers/access_control_allow_methods.h:98](../../../../../../include/protocol/http/common/headers/access_control_allow_methods.h#L98)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::access_control_allow_methods*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-allow-methods header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_allow_methods.h:103](../../../../../../include/protocol/http/common/headers/access_control_allow_methods.h#L103)
