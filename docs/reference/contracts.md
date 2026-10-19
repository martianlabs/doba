<a id="contracts"></a>
<a href="../../resources/docs/reference/contracts/index.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/contracts/index.svg" alt="contracts">
</a>
<a href="../../resources/docs/reference/contracts/index-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/contracts/index-dark.svg" alt="contracts">
</a>

**Header:** [include/contracts.h](../../include/contracts.h)

This header declares the types and operations listed below.

### <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> contracts

[http_server](#concept-http-server)

<a id="api-details"></a>
<a href="../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="concept-http-server"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> http_server

**CONCEPT** · *contracts*

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
