<a id="authentication-info"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authentication_info/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authentication_info/index.svg" alt="authentication_info">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authentication_info/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authentication_info/index-dark.svg" alt="authentication_info">
</a>

**Header:** [include/protocol/http/common/headers/authentication_info.h](../../../../../../include/protocol/http/common/headers/authentication_info.h)

|                                                       authentication-info | | RFC 9110 S11.6.3 Authentication-Info                                      | | The "Authentication-Info" response header field allows an HTTP            | | authentication scheme to communicate information after the client's       | | authentication credentials have been

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[authentication_info](#type-authentication-info)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::authentication_info

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-authentication-info"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authentication_info

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class authentication_info { ... };
```

|                                                       authentication-info | | RFC 9110 S11.6.3 Authentication-Info                                      | | The "Authentication-Info" response header field allows an HTTP            | | authentication scheme to communicate information after the client's       | | authentication credentials have been

**Source:** [include/protocol/http/common/headers/authentication_info.h:117](../../../../../../include/protocol/http/common/headers/authentication_info.h#L117)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::authentication_info*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP authentication-info header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/authentication_info.h:122](../../../../../../include/protocol/http/common/headers/authentication_info.h#L122)
