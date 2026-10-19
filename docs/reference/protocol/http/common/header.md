<a id="header"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/header/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/header/title.svg" alt="header">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/header/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/header/title-dark.svg" alt="header">
</a>

**Header:** [include/protocol/http/common/header.h](../../../../../include/protocol/http/common/header.h)

This header declares the types and operations listed below.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[header](#alias-header) ·
[header_view](#alias-header-view)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="alias-header"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> header

**ALIAS** · *protocol::http*

```cpp
using header = std::pair<std::string, std::string>;
```

A header is a key-value pair of strings. The key is the header name

**Source:** [include/protocol/http/common/header.h:41](../../../../../include/protocol/http/common/header.h#L41)

<a id="alias-header-view"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> header_view

**ALIAS** · *protocol::http*

```cpp
using header_view = std::pair<std::string_view, std::string_view>;
```

Alias for `std::pair<std::string_view, std::string_view>`.

**Source:** [include/protocol/http/common/header.h:42](../../../../../include/protocol/http/common/header.h#L42)
