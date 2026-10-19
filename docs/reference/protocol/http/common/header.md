<a id="header-h"></a>
<h1>
  <a href="../../../../../resources/docs/reference/protocol/http/common/header/index.svg#gh-light-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/common/header/index.svg" alt="Index for header.h">
  </a>
  <a href="../../../../../resources/docs/reference/protocol/http/common/header/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/common/header/index-dark.svg" alt="Index for header.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/header.h](../../../../../include/protocol/http/common/header.h)

This header declares the types and operations listed below.

**protocol::http**

- [header](#alias-header) · *alias*
- [header_view](#alias-header-view) · *alias*

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
