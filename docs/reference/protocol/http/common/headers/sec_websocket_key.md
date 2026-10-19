<a id="sec-websocket-key"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_key/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_key/index.svg" alt="sec_websocket_key">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_key/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_key/index-dark.svg" alt="sec_websocket_key">
</a>

**Header:** [include/protocol/http/common/headers/sec_websocket_key.h](../../../../../../include/protocol/http/common/headers/sec_websocket_key.h)

|                                                         sec-websocket-key | | RFC 6455 S11.3.1 Sec-WebSocket-Key                                        | | The "Sec-WebSocket-Key" header field is used in the WebSocket opening     | | handshake. It is sent from the client to the server to provide part of    | | the information used by the server t

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers

[sec_websocket_key](#type-sec-websocket-key)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::headers::sec_websocket_key

[check](#method-check)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-sec-websocket-key"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sec_websocket_key

**TYPE** · *protocol::http::headers*

**Declaration synopsis:**

```cpp
class sec_websocket_key { ... };
```

|                                                         sec-websocket-key | | RFC 6455 S11.3.1 Sec-WebSocket-Key                                        | | The "Sec-WebSocket-Key" header field is used in the WebSocket opening     | | handshake. It is sent from the client to the server to provide part of    | | the information used by the server t

**Source:** [include/protocol/http/common/headers/sec_websocket_key.h:96](../../../../../../include/protocol/http/common/headers/sec_websocket_key.h#L96)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::headers::sec_websocket_key*

```cpp
static constexpr bool check(std::string_view sv);
```

Checks whether a value follows the HTTP sec-websocket-key header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/headers/sec_websocket_key.h:101](../../../../../../include/protocol/http/common/headers/sec_websocket_key.h#L101)
