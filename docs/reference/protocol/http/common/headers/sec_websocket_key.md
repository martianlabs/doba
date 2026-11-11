<a id="sec-websocket-key"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_key/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_key/title.svg" alt="sec_websocket_key">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_key/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_key/title-dark.svg" alt="sec_websocket_key">
</a>

**Header:** [include/protocol/http/common/headers/sec_websocket_key.h](../../../../../../include/protocol/http/common/headers/sec_websocket_key.h)

Checks nonempty Base64 text with a length divisible by four and valid final padding placement. It does not check decoded length, unused padding bits, or require the 16-byte WebSocket nonce.

<a id="api"></a>
<a id="type-sec-websocket-key"></a>

## sec_websocket_key

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
