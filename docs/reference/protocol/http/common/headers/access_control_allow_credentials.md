<a id="access-control-allow-credentials-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_credentials/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_credentials/index.svg" alt="Index for access_control_allow_credentials.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/access_control_allow_credentials.h](../../../../../../include/protocol/http/common/headers/access_control_allow_credentials.h)

|                                          access-control-allow-credentials | | Fetch S3.3.3 HTTP responses                                               | | The "Access-Control-Allow-Credentials" response header field indicates    | | whether the response can be shared when the request's credentials mode is | | "include".

**protocol::http::headers**

- [access_control_allow_credentials](#type-access-control-allow-credentials) · *type*

**protocol::http::headers::access_control_allow_credentials**

- [check](#method-check) · *method*

<a id="type-access-control-allow-credentials"></a>

## access_control_allow_credentials

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_allow_credentials { ... };
```

|                                          access-control-allow-credentials | | Fetch S3.3.3 HTTP responses                                               | | The "Access-Control-Allow-Credentials" response header field indicates    | | whether the response can be shared when the request's credentials mode is | | "include".

**Source:** [include/protocol/http/common/headers/access_control_allow_credentials.h:68](../../../../../../include/protocol/http/common/headers/access_control_allow_credentials.h#L68)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::access_control_allow_credentials*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-allow-credentials header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_allow_credentials.h:73](../../../../../../include/protocol/http/common/headers/access_control_allow_credentials.h#L73)
