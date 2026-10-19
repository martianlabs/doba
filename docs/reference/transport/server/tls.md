<a id="tls"></a>
<a href="../../../../resources/docs/reference/transport/server/tls/index.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls/index.svg" alt="tls">
</a>
<a href="../../../../resources/docs/reference/transport/server/tls/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls/index-dark.svg" alt="tls">
</a>

**Header:** [include/transport/server/tls.h](../../../../include/transport/server/tls.h)

This class provides a TLS transport for an engine. It inherits from `basic_transport` and takes a `tls_policies` configuration and an engine factory when it is created.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[tls](#type-tls)

<a id="api-details"></a>
<a href="../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-tls"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tls

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tls
    : public basic_transport<ENty, FAty, tls_connection<ENty>, tls_policies> { ... };
```

This class provides a TLS transport for an engine. It inherits from `basic_transport` and takes a `tls_policies` configuration and an engine factory when it is created.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.

**Source:** [include/transport/server/tls.h:49](../../../../include/transport/server/tls.h#L49)
