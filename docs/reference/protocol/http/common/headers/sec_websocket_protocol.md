<a id="sec-websocket-protocol-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_protocol/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_protocol/index.svg" alt="Index for sec_websocket_protocol.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_protocol/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_protocol/index-dark.svg" alt="Index for sec_websocket_protocol.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/headers/sec_websocket_protocol.h](../../../../../../include/protocol/http/common/headers/sec_websocket_protocol.h)

|                                                    sec-websocket-protocol | | RFC 6455 S11.3.4 Sec-WebSocket-Protocol                                   | | The "Sec-WebSocket-Protocol" header field is used in the WebSocket        | | opening handshake to negotiate the application-level subprotocol layered  | | over the WebSocket connection.

**protocol::http::headers**

- [sec_websocket_protocol](#type-sec-websocket-protocol) · *type*

**protocol::http::headers::sec_websocket_protocol**

- [check](#method-check) · *method*

<a id="type-sec-websocket-protocol"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sec_websocket_protocol

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class sec_websocket_protocol { ... };
```

|                                                    sec-websocket-protocol | | RFC 6455 S11.3.4 Sec-WebSocket-Protocol                                   | | The "Sec-WebSocket-Protocol" header field is used in the WebSocket        | | opening handshake to negotiate the application-level subprotocol layered  | | over the WebSocket connection.

**Source:** [include/protocol/http/common/headers/sec_websocket_protocol.h:101](../../../../../../include/protocol/http/common/headers/sec_websocket_protocol.h#L101)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::sec_websocket_protocol*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP sec-websocket-protocol header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/sec_websocket_protocol.h:106](../../../../../../include/protocol/http/common/headers/sec_websocket_protocol.h#L106)
