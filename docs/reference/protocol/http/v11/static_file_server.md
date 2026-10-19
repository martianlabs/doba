<a id="static-file-server"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/index.svg" alt="static_file_server">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/index-dark.svg" alt="static_file_server">
</a>

**Header:** [include/protocol/http/v11/static_file_server.h](../../../../../include/protocol/http/v11/static_file_server.h)

Server implementation.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[static_file_server](#type-static-file-server)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::static_file_server

[register_routes](#method-register-routes)

<a id="api-details"></a>
<a href="../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-static-file-server"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> static_file_server

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
class static_file_server { ... };
```

Server implementation.

**Source:** [include/protocol/http/v11/static_file_server.h:48](../../../../../include/protocol/http/v11/static_file_server.h#L48)

<a id="method-register-routes"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> register_routes

**METHOD** · *protocol::http::v11::static_file_server*

```cpp
template <typename Rty>
  void register_routes(Rty& routes);
```

Registers the static-file routes.

**Template parameters:**

- **Rty:** `typename Rty`.

**Parameters:**

- **routes:** Route collection used for dispatch. Declaration: `Rty& routes`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/static_file_server.h:79](../../../../../include/protocol/http/v11/static_file_server.h#L79)
