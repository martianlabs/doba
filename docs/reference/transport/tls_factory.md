# <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tls_factory

**Internal implementation header:**
[include/transport/tls_factory.h](../../../include/transport/tls_factory.h).
Declarations in `detail` are not supported public API.

Creates a `tls_engine` for each connection using the application's engine
factory. Each engine keeps the shared TLS configuration alive.

Use the public [TLS transport](tls.md) to construct a server.
