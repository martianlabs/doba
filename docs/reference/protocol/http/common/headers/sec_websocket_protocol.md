<a id="sec-websocket-protocol"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_protocol/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_protocol/title.svg" alt="sec_websocket_protocol">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_protocol/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_protocol/title-dark.svg" alt="sec_websocket_protocol">
</a>

**Header:** [include/protocol/http/common/headers/sec_websocket_protocol.h](../../../../../../include/protocol/http/common/headers/sec_websocket_protocol.h)

Requires at least one comma-separated protocol token. It does not select a protocol or reject duplicates.

<a id="api"></a>
<a id="type-sec-websocket-protocol"></a>

## sec_websocket_protocol

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
