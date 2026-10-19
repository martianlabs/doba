<a id="static-file-server-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/index.svg" alt="Index for static_file_server.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/static_file_server.h](../../../../../include/protocol/http/v11/static_file_server.h)

Server implementation.

**protocol::http::v11**

- [static_file_server](#type-static-file-server) · *type*

**protocol::http::v11::static_file_server**

- [static_file_server](#constructor-static-file-server) · *constructor*
- [register_routes](#method-register-routes) · *method*

<a id="type-static-file-server"></a>

## static_file_server

🟠 **TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
class static_file_server { ... };
```

Server implementation.

**Source:** [include/protocol/http/v11/static_file_server.h:48](../../../../../include/protocol/http/v11/static_file_server.h#L48)

<a id="constructor-static-file-server"></a>

## static_file_server

🟠 **CONSTRUCTOR** · *protocol::http::v11::static_file_server*

```cpp
static_file_server(std::string_view prefix, const std::filesystem::path& root);
```

Constructs a static_file_server value.

**Parameters:**

- **prefix:** Initial value supplied to the constructor. Declaration: `std::string_view prefix`.
- **root:** Root filesystem path. Declaration: `const std::filesystem::path& root`.

**Errors:** Directly throws `std::filesystem::filesystem_error`, `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/static_file_server.h:53](../../../../../include/protocol/http/v11/static_file_server.h#L53)

<a id="method-register-routes"></a>

## register_routes

🟠 **METHOD** · *protocol::http::v11::static_file_server*

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
