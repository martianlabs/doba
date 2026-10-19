<a id="tls-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tls/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tls/index.svg" alt="Index for tls.h">
  </picture>
</h1>

**Header:** [include/transport/server/tls.h](../../../../include/transport/server/tls.h)

This class provides a TLS transport for an engine. It inherits from `basic_transport` and takes a `tls_policies` configuration and an engine factory when it is created.

**transport::server**

- [tls](#type-tls) · *type*

**transport::server::tls**

- [tls](#constructor-tls) · *constructor*

<a id="type-tls"></a>

## tls

🟠 **TYPE** · *transport::server*

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

<a id="constructor-tls"></a>

## tls

🟠 **CONSTRUCTOR** · *transport::server::tls*

```cpp
explicit tls(tls_policies configuration, FAty create_engine);
```

| CONSTRUCTORs/DESTRUCTORs                                     ( public ) |

**Parameters:**

- **configuration:** Configuration for this component. Declaration: `tls_policies configuration`.
- **create_engine:** Initial value supplied to the constructor. Declaration: `FAty create_engine`.

**Source:** [include/transport/server/tls.h:55](../../../../include/transport/server/tls.h#L55)
