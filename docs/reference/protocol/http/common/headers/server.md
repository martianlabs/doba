<a id="server"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/server/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/server/title.svg" alt="server">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/server/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/server/title-dark.svg" alt="server">
</a>

**Header:** [include/protocol/http/common/headers/server.h](../../../../../../include/protocol/http/common/headers/server.h)

|                                                                    server | | RFC 9110 S10.2.4 Server                                                   | | The "Server" header field contains information about the software used by | | the origin server to handle the request. It is commonly used by clients to| | identify the scope of interoperabili

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[server](#type-server)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::server

[check](#method-check)

<a id="api"></a>
<a href="../../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-server"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> server

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class server { ... };
```

|                                                                    server | | RFC 9110 S10.2.4 Server                                                   | | The "Server" header field contains information about the software used by | | the origin server to handle the request. It is commonly used by clients to| | identify the scope of interoperabili

**Source:** [include/protocol/http/common/headers/server.h:98](../../../../../../include/protocol/http/common/headers/server.h#L98)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::server*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP server header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/server.h:103](../../../../../../include/protocol/http/common/headers/server.h#L103)
