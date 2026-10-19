<a id="access-control-allow-credentials"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_credentials/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_credentials/index.svg" alt="access_control_allow_credentials">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_credentials/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_credentials/index-dark.svg" alt="access_control_allow_credentials">
</a>

**Header:** [include/protocol/http/common/headers/access_control_allow_credentials.h](../../../../../../include/protocol/http/common/headers/access_control_allow_credentials.h)

|                                          access-control-allow-credentials | | Fetch S3.3.3 HTTP responses                                               | | The "Access-Control-Allow-Credentials" response header field indicates    | | whether the response can be shared when the request's credentials mode is | | "include".

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[access_control_allow_credentials](#type-access-control-allow-credentials)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::access_control_allow_credentials

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-access-control-allow-credentials"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_allow_credentials

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_allow_credentials { ... };
```

|                                          access-control-allow-credentials | | Fetch S3.3.3 HTTP responses                                               | | The "Access-Control-Allow-Credentials" response header field indicates    | | whether the response can be shared when the request's credentials mode is | | "include".

**Source:** [include/protocol/http/common/headers/access_control_allow_credentials.h:68](../../../../../../include/protocol/http/common/headers/access_control_allow_credentials.h#L68)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::access_control_allow_credentials*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-allow-credentials header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_allow_credentials.h:73](../../../../../../include/protocol/http/common/headers/access_control_allow_credentials.h#L73)
