<a id="https-server"></a>
<a href="../../resources/docs/reference/https_server/title.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/https_server/title.svg" alt="https_server">
</a>
<a href="../../resources/docs/reference/https_server/title-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/https_server/title-dark.svg" alt="https_server">
</a>

**Header:** [include/https_server.h](../../include/https_server.h)

HTTP/1.1 server using the separate TCP-based TLS transport on Linux and Windows.
Link `martianlabs::doba_tls`, configure `tls_policies` with a PEM chain and key,
and advertise only `http/1.1` through ALPN. Other configured tokens are
rejected before transport construction. Route and engine APIs match
[http_server](http_server.md). See the [TLS runtime contract](transport/tls.md)
for finite source limits, shutdown behavior and excluded features.

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
    : public http_server<RQty, RSty, ROty, ENty, transport::tls,
                         AUty...> { ... };
```

HTTP/1.1 server using the separate TCP-based TLS transport on Linux and Windows.
Link `martianlabs::doba_tls`, configure `tls_policies` with a PEM chain and key,
and advertise only `http/1.1` through ALPN. Other configured tokens are
rejected before transport construction. Route and engine APIs match
[http_server](http_server.md). See the [TLS runtime contract](transport/tls.md)
for finite source limits, shutdown behavior and excluded features.

**Template parameters:**

- **RQty:** `typename RQty = protocol::http::v11::request`.
- **RSty:** `typename RSty = protocol::http::v11::response`.
- **ROty:** `typename ROty = protocol::http::router<RQty, RSty>`.
- **ENty:** `protocol::contracts::engine ENty = protocol::http::v11::engine<RQty, RSty, ROty>`.
- **AUty:** `typename... AUty`.

**Source:** [include/https_server.h:49](../../include/https_server.h#L49)

**Constructors:**

```cpp
explicit https_server(transport::tls_policies configuration = {},
                      typename ENty::policies_type engine_configuration = {})
  requires(sizeof...(AUty) == 0);
https_server(transport::tls_policies configuration,
             typename ENty::policies_type engine_configuration,
             std::tuple<AUty...> authentication);
```

The second overload owns the supplied authentication providers. Route
registration and authentication APIs follow `http_server`; the TLS wire tests
exercise both construction forms and authenticated requests. Empty ALPN
configuration selects `http/1.1`; other tokens are rejected. The transport is
created on first start, so credential errors can surface from `start()` before
any listener is opened. Configuration and administrative calls remain sequential
owner operations outside worker callbacks.
