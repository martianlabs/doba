<a id="https-server-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/https_server/index-dark.svg">
    <img src="../../resources/docs/reference/https_server/index.svg" alt="Index for https_server.h">
  </picture>
</h1>

**Header:** [include/https_server.h](../../include/https_server.h)

Composes a TLS HTTP/1.1 routing with a protocol engine and transport.

**doba**

- [https_server](#type-https-server) · *type*

**https_server**

- [https_server](#constructor-https-server) · *constructor*

<a id="type-https-server"></a>

## https_server

🟠 **TYPE** · *doba*

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

Composes a TLS HTTP/1.1 routing with a protocol engine and transport.

**Template parameters:**

- **RQty:** `typename RQty = protocol::http::v11::request`.
- **RSty:** `typename RSty = protocol::http::v11::response`.
- **ROty:** `typename ROty = protocol::http::router<RQty, RSty>`.
- **ENty:** `protocol::contracts::engine ENty = protocol::http::v11::engine<RQty, RSty, ROty>`.
- **AUty:** `typename... AUty`.

**Source:** [include/https_server.h:45](../../include/https_server.h#L45)

<a id="constructor-https-server"></a>

## https_server

🟠 **CONSTRUCTOR** · *https_server*

```cpp
using http_server<RQty, RSty, ROty, ENty,
                    transport::server::tls, AUty...>::http_server;
```

Inherits the HTTP server constructors with the TLS transport selected. See [http_server.h](http_server.md) for parameters and registration behavior.

**Source:** [include/https_server.h:52](../../include/https_server.h#L52)
