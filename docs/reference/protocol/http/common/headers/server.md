<a id="server-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/server/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/server/index.svg" alt="Index for server.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/server/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/server/index-dark.svg" alt="Index for server.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/server.h](../../../../../../include/protocol/http/common/headers/server.h)

|                                                                    server | | RFC 9110 S10.2.4 Server                                                   | | The "Server" header field contains information about the software used by | | the origin server to handle the request. It is commonly used by clients to| | identify the scope of interoperabili

**protocol::http::headers**

- [server](#type-server) · *type*

**protocol::http::headers::server**

- [check](#method-check) · *method*

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
