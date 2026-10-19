<a id="sec-websocket-version"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/index.svg" alt="sec_websocket_version">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/index-dark.svg" alt="sec_websocket_version">
</a>

**Header:** [include/protocol/http/common/headers/sec_websocket_version.h](../../../../../../include/protocol/http/common/headers/sec_websocket_version.h)

|                                                     sec-websocket-version | | RFC 6455 S11.3.5 Sec-WebSocket-Version                                    | | The "Sec-WebSocket-Version" header field is used in the WebSocket opening | | handshake to indicate the protocol version used by the client.            | | A client MUST include this header fi

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[sec_websocket_version](#type-sec-websocket-version)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::sec_websocket_version

[check](#method-check)

<a id="type-sec-websocket-version"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sec_websocket_version

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class sec_websocket_version { ... };
```

|                                                     sec-websocket-version | | RFC 6455 S11.3.5 Sec-WebSocket-Version                                    | | The "Sec-WebSocket-Version" header field is used in the WebSocket opening | | handshake to indicate the protocol version used by the client.            | | A client MUST include this header fi

**Source:** [include/protocol/http/common/headers/sec_websocket_version.h:106](../../../../../../include/protocol/http/common/headers/sec_websocket_version.h#L106)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::sec_websocket_version*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP sec-websocket-version header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/sec_websocket_version.h:111](../../../../../../include/protocol/http/common/headers/sec_websocket_version.h#L111)
