<a id="query-parameter"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/query_parameter/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/query_parameter/title.svg" alt="query_parameter">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/query_parameter/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/query_parameter/title-dark.svg" alt="query_parameter">
</a>

**Header:** [include/protocol/http/common/query_parameter.h](../../../../../include/protocol/http/common/query_parameter.h)

This header declares the types and operations listed below.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[query_parameter](#alias-query-parameter) ·
[query_parameter_view](#alias-query-parameter-view)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="alias-query-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> query_parameter

**ALIAS** · *protocol::http*

```cpp
using query_parameter = std::pair<std::string, std::string>;
```

Owns the parameter name in `first` and value in `second`. The alias itself does not decode text or change the case-sensitive comparisons of `std::string`.

**Source:** [include/protocol/http/common/query_parameter.h](../../../../../include/protocol/http/common/query_parameter.h)

<a id="alias-query-parameter-view"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> query_parameter_view

**ALIAS** · *protocol::http*

```cpp
using query_parameter_view = std::pair<std::string_view, std::string_view>;
```

Borrows the query parameter name and value. Both backing strings must remain alive and unchanged while the views are used. The pair does not validate or normalize either component.

**Source:** [include/protocol/http/common/query_parameter.h](../../../../../include/protocol/http/common/query_parameter.h)
