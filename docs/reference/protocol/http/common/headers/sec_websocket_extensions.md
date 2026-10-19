<a id="sec-websocket-extensions-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_extensions/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_extensions/index.svg" alt="Index for sec_websocket_extensions.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_extensions/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_extensions/index-dark.svg" alt="Index for sec_websocket_extensions.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/sec_websocket_extensions.h](../../../../../../include/protocol/http/common/headers/sec_websocket_extensions.h)

|                                                 sec-websocket-extensions  | | RFC 6455 S9.1 Sec-WebSocket-Extensions                                    | | The "Sec-WebSocket-Extensions" header field is used during the WebSocket  | | opening handshake to negotiate protocol-level extensions.                 | | A client MAY send this field in its

**protocol::http::headers**

- [sec_websocket_extensions](#type-sec-websocket-extensions) · *type*

**protocol::http::headers::sec_websocket_extensions**

- [check](#method-check) · *method*

<a id="type-sec-websocket-extensions"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sec_websocket_extensions

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class sec_websocket_extensions { ... };
```

|                                                 sec-websocket-extensions  | | RFC 6455 S9.1 Sec-WebSocket-Extensions                                    | | The "Sec-WebSocket-Extensions" header field is used during the WebSocket  | | opening handshake to negotiate protocol-level extensions.                 | | A client MAY send this field in its

**Source:** [include/protocol/http/common/headers/sec_websocket_extensions.h:104](../../../../../../include/protocol/http/common/headers/sec_websocket_extensions.h#L104)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::sec_websocket_extensions*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP sec-websocket-extensions header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/sec_websocket_extensions.h:109](../../../../../../include/protocol/http/common/headers/sec_websocket_extensions.h#L109)
