<a id="access-control-expose-headers-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_expose_headers/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_expose_headers/index.svg" alt="Index for access_control_expose_headers.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/access_control_expose_headers.h](../../../../../../include/protocol/http/common/headers/access_control_expose_headers.h)

|                                             access-control-expose-headers | | Fetch Standard S3.3.3 Access-Control-Expose-Headers                       | | The "Access-Control-Expose-Headers" response header indicates which       | | response headers can be exposed to frontend JavaScript as part of a CORS  | | response.

**protocol::http::headers**

- [access_control_expose_headers](#type-access-control-expose-headers) · *type*

**protocol::http::headers::access_control_expose_headers**

- [check](#method-check) · *method*

<a id="type-access-control-expose-headers"></a>

## access_control_expose_headers

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_expose_headers { ... };
```

|                                             access-control-expose-headers | | Fetch Standard S3.3.3 Access-Control-Expose-Headers                       | | The "Access-Control-Expose-Headers" response header indicates which       | | response headers can be exposed to frontend JavaScript as part of a CORS  | | response.

**Source:** [include/protocol/http/common/headers/access_control_expose_headers.h:96](../../../../../../include/protocol/http/common/headers/access_control_expose_headers.h#L96)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::access_control_expose_headers*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-expose-headers header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_expose_headers.h:101](../../../../../../include/protocol/http/common/headers/access_control_expose_headers.h#L101)
