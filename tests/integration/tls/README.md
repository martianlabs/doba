<a name="tls-integration"></a>
<h1>
  <a href="../../../resources/tests/integration/tls/h1-tls-integration.svg#gh-light-mode-only">
    <img src="../../../resources/tests/integration/tls/h1-tls-integration.svg" alt="TLS tests">
  </a>
  <a href="../../../resources/tests/integration/tls/h1-tls-integration-dark.svg#gh-dark-mode-only">
    <img src="../../../resources/tests/integration/tls/h1-tls-integration-dark.svg" alt="TLS tests">
  </a>
</h1>

Turn on `DOBA_ENABLE_TLS=ON` and have OpenSSL 3.0+ ready. The clients use
OpenSSL directly; you don't need its command-line program or an interpreter.

| CTest group | What we throw at it |
| --- | --- |
| `doba_tls_core_tests` | TLS 1.2/1.3, tiny BIOs, KeyUpdate and damaged encrypted records. |
| `doba_tls_tests` | Real sockets, pipelines, slow readers, limits, restarts and ALPN. |
| `doba_tls_processing_tests` | 114 in-memory cases: busy output, processing turns, readers and exact budgets. |
| `doba_tls_processing_wire_tests` | Mixed HTTPS handlers, ordered replies and cancellation over real sockets. |
| `doba_tls_configuration_tests` | Bad settings, missing or mismatched credentials and encrypted keys. No password prompts. |
| `doba_tls_examples_tests` | Both TLS examples, including the coroutine route. Needs examples enabled. |

Clients check the certificate and hostname. Corrupt records must deliver no
plaintext. Incomplete handshakes must let a healthy peer progress. Tiny
encrypted and plaintext buffers must still carry the exact reply.
Every group and socket wait has a deadline.

The checked-in `cert.pem` and `key.pem` are public test credentials.
Keep them out of deployments.

For the supported behavior, see the [TLS test map](../../../docs/reference/transport/tls-validation.md).
The handshake test doesn't add a timeout to the library; connection exhaustion
and longer stress runs still need separate checks.
