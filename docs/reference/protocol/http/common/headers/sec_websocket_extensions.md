<a id="sec-websocket-extensions"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_extensions/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_extensions/title.svg" alt="sec_websocket_extensions">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_extensions/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/common/headers/sec_websocket_extensions/title-dark.svg" alt="sec_websocket_extensions">
</a>

**Header:** [include/protocol/http/common/headers/sec_websocket_extensions.h](../../../../../../include/protocol/http/common/headers/sec_websocket_extensions.h)

Requires at least one extension token, optionally followed by semicolon-separated extension parameters. Parameters may be bare tokens or `name=value`, and empty semicolon slots are accepted. It does not negotiate extensions.

<a id="api"></a>
<a id="type-sec-websocket-extensions"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> sec_websocket_extensions

Stateless checker in `martianlabs::doba::protocol::http::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv);
```

Returns `true` when the supplied value passes the checks above, otherwise `false`. The checker borrows the text for the call and does not retain or modify it.

List parsing skips empty comma elements and respects quoted commas. Spaces/tabs are trimmed before commas and after separators, but not uniformly at the outer edges; pass the field value after HTTP field-level trimming.
