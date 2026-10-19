<a id="sec-websocket-accept-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_accept/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_accept/index.svg" alt="Index for sec_websocket_accept.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/sec_websocket_accept.h](../../../../../../include/protocol/http/common/headers/sec_websocket_accept.h)

|                                                      sec-websocket-accept | | RFC 6455 S11.3.3 Sec-WebSocket-Accept                                     | | The "Sec-WebSocket-Accept" header field is used in the WebSocket opening  | | handshake. It is sent by the server to prove that it received the         | | client's Sec-WebSocket-Key value and

**protocol::http::headers**

- [sec_websocket_accept](#type-sec-websocket-accept) · *type*

**protocol::http::headers::sec_websocket_accept**

- [check](#method-check) · *method*

<a id="type-sec-websocket-accept"></a>

## sec_websocket_accept

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class sec_websocket_accept { ... };
```

|                                                      sec-websocket-accept | | RFC 6455 S11.3.3 Sec-WebSocket-Accept                                     | | The "Sec-WebSocket-Accept" header field is used in the WebSocket opening  | | handshake. It is sent by the server to prove that it received the         | | client's Sec-WebSocket-Key value and

**Source:** [include/protocol/http/common/headers/sec_websocket_accept.h:76](../../../../../../include/protocol/http/common/headers/sec_websocket_accept.h#L76)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::sec_websocket_accept*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP sec-websocket-accept header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/sec_websocket_accept.h:81](../../../../../../include/protocol/http/common/headers/sec_websocket_accept.h#L81)
