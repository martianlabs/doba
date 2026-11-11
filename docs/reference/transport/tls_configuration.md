# <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tls_configuration

**Internal implementation header:**
[include/transport/tls_configuration.h](../../../include/transport/tls_configuration.h).
Declarations in `detail` are not supported public API.

The private `detail::tls_configuration` owns fixed policies and an SSL_CTX.
Factories and sessions share an immutable owner. It cannot be copied or moved:
the ALPN callback retains its address. Each session retains that owner while
OpenSSL can use the callback.

Credential loading and policy validation happen before TCP construction can
open a listener. This type does not own sockets, execute application work,
monitor certificate files or offer dynamic reconfiguration. Public settings
are supplied through `tls_policies`.

See the [TLS contract](tls.md), [policies](tls_policies.md) and
[validation coverage](tls-validation.md).
