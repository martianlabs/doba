<a id="https-server"></a>
<a href="../../resources/docs/reference/https_server/title.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/https_server/title.svg" alt="https_server">
</a>
<a href="../../resources/docs/reference/https_server/title-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/https_server/title-dark.svg" alt="https_server">
</a>

**Header:** [include/https_server.h](../../include/https_server.h)

Compatibility declaration for the TLS server type. TLS is unsupported by the
current Linux and Windows backends; constructing it with the bundled TLS
transport fails explicitly.
Use [http_server](http_server.md) for synchronous TCP. This declaration does not
provide an HTTPS backend or an enable-TLS procedure.

### <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> doba

[https_server](#type-https-server)

<a id="api"></a>
<a href="../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-https-server"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> https_server

**TYPE** · *doba*

**Declaration synopsis:**

```cpp
template <typename RQty = protocol::http::v11::request,
          typename RSty = protocol::http::v11::response,
          typename ROty = protocol::http::router<RQty, RSty>,
          protocol::contracts::engine ENty =
              protocol::http::v11::engine<RQty, RSty, ROty>,
          typename... AUty>
class https_server
    : public http_server<RQty, RSty, ROty, ENty, transport::server::tls,
                         AUty...> { ... };
```

Compatibility declaration for the TLS server type. TLS is unsupported by the
current Linux and Windows backends; constructing it with the bundled TLS
transport fails explicitly.
Use [http_server](http_server.md) for synchronous TCP. This declaration does not
provide an HTTPS backend or an enable-TLS procedure.

**Template parameters:**

- **RQty:** `typename RQty = protocol::http::v11::request`.
- **RSty:** `typename RSty = protocol::http::v11::response`.
- **ROty:** `typename ROty = protocol::http::router<RQty, RSty>`.
- **ENty:** `protocol::contracts::engine ENty = protocol::http::v11::engine<RQty, RSty, ROty>`.
- **AUty:** `typename... AUty`.

**Source:** [include/https_server.h:45](../../include/https_server.h#L45)
