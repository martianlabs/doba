<a id="access-control-allow-origin-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/index.svg" alt="Index for access_control_allow_origin.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/access_control_allow_origin.h](../../../../../../include/protocol/http/common/headers/access_control_allow_origin.h)

|                                               access-control-allow-origin | | WHATWG Fetch S3.3.3 Access-Control-Allow-Origin                           | | The "Access-Control-Allow-Origin" response header field indicates whether | | a response can be shared with requesting code from the given origin.      | | A server MAY return the literal valu

**protocol::http::headers**

- [access_control_allow_origin](#type-access-control-allow-origin) · *type*

**protocol::http::headers::access_control_allow_origin**

- [check](#method-check) · *method*

<a id="type-access-control-allow-origin"></a>

## access_control_allow_origin

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_allow_origin { ... };
```

|                                               access-control-allow-origin | | WHATWG Fetch S3.3.3 Access-Control-Allow-Origin                           | | The "Access-Control-Allow-Origin" response header field indicates whether | | a response can be shared with requesting code from the given origin.      | | A server MAY return the literal valu

**Source:** [include/protocol/http/common/headers/access_control_allow_origin.h:124](../../../../../../include/protocol/http/common/headers/access_control_allow_origin.h#L124)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::access_control_allow_origin*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-allow-origin header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_allow_origin.h:129](../../../../../../include/protocol/http/common/headers/access_control_allow_origin.h#L129)
