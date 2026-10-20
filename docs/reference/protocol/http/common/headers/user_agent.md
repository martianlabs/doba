<a id="user-agent"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/title.svg" alt="user_agent">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/title-dark.svg" alt="user_agent">
</a>

**Header:** [include/protocol/http/common/headers/user_agent.h](../../../../../../include/protocol/http/common/headers/user_agent.h)

|                                                                user-agent | | RFC 9110 S10.1.5 User-Agent                                               | | The "User-Agent" request header field contains information about the user | | agent originating the request. It can be used by servers to identify the  | | scope of reported interoperability p

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[user_agent](#type-user-agent)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::user_agent

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-user-agent"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> user_agent

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class user_agent { ... };
```

|                                                                user-agent | | RFC 9110 S10.1.5 User-Agent                                               | | The "User-Agent" request header field contains information about the user | | agent originating the request. It can be used by servers to identify the  | | scope of reported interoperability p

**Source:** [include/protocol/http/common/headers/user_agent.h:97](../../../../../../include/protocol/http/common/headers/user_agent.h#L97)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::user_agent*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP user-agent header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/user_agent.h:102](../../../../../../include/protocol/http/common/headers/user_agent.h#L102)
