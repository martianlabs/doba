<a id="sec-websocket-version"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/title.svg" alt="sec_websocket_version">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_version/title-dark.svg" alt="sec_websocket_version">
</a>

**Header:** [include/protocol/http/common/headers/sec_websocket_version.h](../../../../../../include/protocol/http/common/headers/sec_websocket_version.h)

Checks comma-separated decimal octets from 0 to 255, rejecting leading zeroes on multi-digit numbers. Empty lists are accepted. It does not require version 13 or establish WebSocket support.

<a id="api"></a>
<a id="type-sec-websocket-version"></a>

## sec_websocket_version

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
