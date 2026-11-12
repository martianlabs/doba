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

Owns the header name in `first` and value in `second`. The pair itself does not validate, trim, or compare names case-insensitively.

**Source:** [include/protocol/http/common/header.h](../../../../../include/protocol/http/common/header.h)

<a id="alias-header-view"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> header_view

**ALIAS** · *protocol::http*

```cpp
using header_view = std::pair<std::string_view, std::string_view>;
```

Borrows the header name and value. Both backing strings must remain alive and unchanged while the views are used. The pair does not validate or normalize either component.

**Source:** [include/protocol/http/common/header.h](../../../../../include/protocol/http/common/header.h)
