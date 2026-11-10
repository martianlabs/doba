# tls_engine

**Internal implementation header:**
[include/transport/tls_engine.h](../../../include/transport/tls_engine.h).
Declarations in `detail` are not supported public API.

The private `detail::tls_engine` adapts an application engine to the unchanged
TCP engine contract. [tls_factory](tls_factory.md) constructs one adapter per connection.
It owns RX plaintext, a bounded BIO pair, one output plaintext record and the
finite ciphertext batch. Application output and sources are copied/consumed
synchronously. TCP alone owns sockets, workers and actual send completion.

No application executor, callback thread, flow-control service or independent
connection mutex is provided. Public construction uses `tls<Engine, Factory>`.

See the [TLS contract](tls.md), [policies](tls_policies.md) and
[validation coverage](tls-validation.md).

The adapter returns the last complete application processing_result from both
entry points. Decrypting a TLS control record or incomplete fragment preserves
that snapshot. on_processing_turn invokes the application engine with TCP's
current output-idle observation; pending ciphertext does not block task progress.
All SSL calls execute inside TCP's existing serialized entries, never a timer
callback or external handler thread. There is no TLS scheduler or extra mutex.

A successful RX or turn flushes the final plaintext record and drains the BIO
before submitting its encoded batch to TCP. Readers are consumed synchronously.
No unsubmitted TLS output remains behind TCP's output-idle observation. Every
entry resets the plaintext/ciphertext batch budgets; failures discard only that
current batch. Engine/peer closure withdraws all interests. An unfinished TLS
handshake never receives application processing turns.

Input retention is bounded by the configured plaintext block and BIO pair.
TLS consumes the complete borrowed TCP block before returning; it introduces
no deferred-input queue. The protocol owns input needed across suspensions and
its own finite budget. See the [general contract](../protocol/processing_result.md)
and [processing tests](../../../tests/integration/tls/processing.cpp).
