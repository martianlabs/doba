<a id="sec-websocket-accept"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_accept/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_accept/title.svg" alt="sec_websocket_accept">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_accept/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_accept/title-dark.svg" alt="sec_websocket_accept">
</a>

**Header:** [include/protocol/http/common/headers/sec_websocket_accept.h](../../../../../../include/protocol/http/common/headers/sec_websocket_accept.h)

Checks nonempty Base64 text with a length divisible by four and valid final padding placement. It does not check decoded length, unused padding bits, or the expected WebSocket handshake hash.

<a id="api"></a>
<a id="type-sec-websocket-accept"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sec_websocket_accept

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.
