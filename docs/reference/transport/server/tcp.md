<a id="tcp"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp/title.svg" alt="tcp">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp/title-dark.svg" alt="tcp">
</a>

**Header:** [include/transport/server/tcp.h](../../../../include/transport/server/tcp.h)

The public TCP transport. Platform workers, connection state and dispatch helpers are internal implementation details.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[tcp](#type-tcp)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-tcp"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tcp

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tcp
    : public detail::basic_transport<ENty, FAty, detail::tcp_connection<ENty>, tcp_policies> { ... };
```

This class provides a TCP transport for an engine. It inherits from `detail::basic_transport` and takes a `tcp_policies` configuration and an engine factory when it is created.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **FAty:** `protocol::contracts::engine_factory<ENty> FAty`.

**Source:** [include/transport/server/tcp.h:79](../../../../include/transport/server/tcp.h#L79)
