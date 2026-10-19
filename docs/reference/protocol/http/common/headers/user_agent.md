<a id="user-agent-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/user_agent/index.svg" alt="Index for user_agent.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/user_agent.h](../../../../../../include/protocol/http/common/headers/user_agent.h)

|                                                                user-agent | | RFC 9110 S10.1.5 User-Agent                                               | | The "User-Agent" request header field contains information about the user | | agent originating the request. It can be used by servers to identify the  | | scope of reported interoperability p

**protocol::http::headers**

- [user_agent](#type-user-agent) · *type*

**protocol::http::headers::user_agent**

- [check](#method-check) · *method*

<a id="type-user-agent"></a>

## user_agent

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class user_agent { ... };
```

|                                                                user-agent | | RFC 9110 S10.1.5 User-Agent                                               | | The "User-Agent" request header field contains information about the user | | agent originating the request. It can be used by servers to identify the  | | scope of reported interoperability p

**Source:** [include/protocol/http/common/headers/user_agent.h:97](../../../../../../include/protocol/http/common/headers/user_agent.h#L97)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::user_agent*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP user-agent header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/user_agent.h:102](../../../../../../include/protocol/http/common/headers/user_agent.h#L102)
