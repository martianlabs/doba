<a id="query-parameter-h"></a>
<h1>
  <a href="../../../../../resources/docs/reference/protocol/http/common/query_parameter/index.svg#gh-light-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/common/query_parameter/index.svg" alt="Index for query_parameter.h">
  </a>
  <a href="../../../../../resources/docs/reference/protocol/http/common/query_parameter/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/common/query_parameter/index-dark.svg" alt="Index for query_parameter.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/query_parameter.h](../../../../../include/protocol/http/common/query_parameter.h)

This header declares the types and operations listed below.

**protocol::http**

- [query_parameter](#alias-query-parameter) · *alias*
- [query_parameter_view](#alias-query-parameter-view) · *alias*

<a id="alias-query-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> query_parameter

**ALIAS** · *protocol::http*

```cpp
using query_parameter = std::pair<std::string, std::string>;
```

A query parameter is a key-value pair of strings. The key is the parameter name (case-insensitive) and the value is the parameter value (case-sensitive).

**Source:** [include/protocol/http/common/query_parameter.h:42](../../../../../include/protocol/http/common/query_parameter.h#L42)

<a id="alias-query-parameter-view"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> query_parameter_view

**ALIAS** · *protocol::http*

```cpp
using query_parameter_view = std::pair<std::string_view, std::string_view>;
```

Alias for `std::pair<std::string_view, std::string_view>`.

**Source:** [include/protocol/http/common/query_parameter.h:43](../../../../../include/protocol/http/common/query_parameter.h#L43)
