<a id="authorization"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/title.svg" alt="authorization">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/authorization/title-dark.svg" alt="authorization">
</a>

**Header:** [include/protocol/http/common/headers/authorization.h](../../../../../../include/protocol/http/common/headers/authorization.h)

|                                                             authorization | | RFC 9110 S11.6.2 Authorization                                            | | The "Authorization" header field allows a user agent to authenticate      | | itself with an origin server, usually after receiving a 401               | | (Unauthorized) response containing o

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[authorization](#type-authorization)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::authorization

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-authorization"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> authorization

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class authorization { ... };
```

|                                                             authorization | | RFC 9110 S11.6.2 Authorization                                            | | The "Authorization" header field allows a user agent to authenticate      | | itself with an origin server, usually after receiving a 401               | | (Unauthorized) response containing o

**Source:** [include/protocol/http/common/headers/authorization.h:109](../../../../../../include/protocol/http/common/headers/authorization.h#L109)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::authorization*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP authorization header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/authorization.h:114](../../../../../../include/protocol/http/common/headers/authorization.h#L114)
