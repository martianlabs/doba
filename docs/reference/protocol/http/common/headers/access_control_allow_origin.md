<a id="access-control-allow-origin"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/index.svg" alt="access_control_allow_origin">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/access_control_allow_origin/index-dark.svg" alt="access_control_allow_origin">
</a>

**Header:** [include/protocol/http/common/headers/access_control_allow_origin.h](../../../../../../include/protocol/http/common/headers/access_control_allow_origin.h)

|                                               access-control-allow-origin | | WHATWG Fetch S3.3.3 Access-Control-Allow-Origin                           | | The "Access-Control-Allow-Origin" response header field indicates whether | | a response can be shared with requesting code from the given origin.      | | A server MAY return the literal valu

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[access_control_allow_origin](#type-access-control-allow-origin)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::access_control_allow_origin

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-access-control-allow-origin"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> access_control_allow_origin

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class access_control_allow_origin { ... };
```

|                                               access-control-allow-origin | | WHATWG Fetch S3.3.3 Access-Control-Allow-Origin                           | | The "Access-Control-Allow-Origin" response header field indicates whether | | a response can be shared with requesting code from the given origin.      | | A server MAY return the literal valu

**Source:** [include/protocol/http/common/headers/access_control_allow_origin.h:124](../../../../../../include/protocol/http/common/headers/access_control_allow_origin.h#L124)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::access_control_allow_origin*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP access-control-allow-origin header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/access_control_allow_origin.h:129](../../../../../../include/protocol/http/common/headers/access_control_allow_origin.h#L129)
