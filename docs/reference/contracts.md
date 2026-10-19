<a id="contracts-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/contracts/index-dark.svg">
    <img src="../../resources/docs/reference/contracts/index.svg" alt="Index for contracts.h">
  </picture>
</h1>

**Header:** [include/contracts.h](../../include/contracts.h)

This header declares the types and operations listed below.

**contracts**

- [http_server](#concept-http-server) · *concept*

<a id="concept-http-server"></a>

## http_server

🟠 **CONCEPT** · *contracts*

```cpp
template <typename RQty, typename RSty, typename ROty, typename ENty,
          template <typename, typename> class TRty>
concept http_server =
    protocol::http::v11::contracts::request<RQty> &&
    protocol::http::v11::contracts::response<RSty> &&
    protocol::http::v11::contracts::router<ROty, RQty, RSty> &&
    std::constructible_from<ENty, const typename ENty::policies_type&,
                            const ROty&> &&
    transport::server::contracts::transport<
        TRty<ENty, protocol::http::v11::engine_factory<ENty, ROty>>, ENty,
        protocol::http::v11::engine_factory<ENty, ROty>>;
```

HTTP/1.1 server composition contract.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **ROty:** `typename ROty`.
- **ENty:** `typename ENty`.
- **TRty:** `template <typename, typename> class TRty`.

**Source:** [include/contracts.h:43](../../include/contracts.h#L43)
