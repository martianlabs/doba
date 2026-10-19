<a id="sec-websocket-version-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/index.svg" alt="Index for sec_websocket_version.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/headers/sec_websocket_version.h](../../../../../../include/protocol/http/common/headers/sec_websocket_version.h)

|                                                     sec-websocket-version | | RFC 6455 S11.3.5 Sec-WebSocket-Version                                    | | The "Sec-WebSocket-Version" header field is used in the WebSocket opening | | handshake to indicate the protocol version used by the client.            | | A client MUST include this header fi

**protocol::http::headers**

- [sec_websocket_version](#type-sec-websocket-version) · *type*

**protocol::http::headers::sec_websocket_version**

- [check](#method-check) · *method*

<a id="type-sec-websocket-version"></a>

## sec_websocket_version

🟠 **TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class sec_websocket_version { ... };
```

|                                                     sec-websocket-version | | RFC 6455 S11.3.5 Sec-WebSocket-Version                                    | | The "Sec-WebSocket-Version" header field is used in the WebSocket opening | | handshake to indicate the protocol version used by the client.            | | A client MUST include this header fi

**Source:** [include/protocol/http/common/headers/sec_websocket_version.h:106](../../../../../../include/protocol/http/common/headers/sec_websocket_version.h#L106)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::headers::sec_websocket_version*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP sec-websocket-version header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/sec_websocket_version.h:111](../../../../../../include/protocol/http/common/headers/sec_websocket_version.h#L111)
