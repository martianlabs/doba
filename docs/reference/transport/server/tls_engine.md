# tls_engine

**Internal implementation header:**
[include/transport/server/tls_engine.h](../../../../include/transport/server/tls_engine.h).
Declarations in `detail` are not supported public API.

The private `detail::tls_engine` adapts an application engine to the unchanged
TCP engine contract. `detail::tls_factory` constructs one adapter per connection.
It owns RX plaintext, a bounded BIO pair, one output plaintext record and the
finite ciphertext batch. Application output and sources are copied/consumed
synchronously. TCP alone owns sockets, workers and actual send completion.

No application executor, callback thread, flow-control service or independent
connection mutex is provided. Public construction uses `tls<Engine, Factory>`.

See the [TLS contract](tls.md), [policies](tls_policies.md) and
[validation coverage](tls-validation.md).
