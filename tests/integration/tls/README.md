# TLS integration

Enable `DOBA_ENABLE_TLS=ON`. Production consumers need OpenSSL 3.0 or newer;
these tests additionally require Python 3.9 or newer with TLS 1.3 in its ssl
module. No third-party Python package or OpenSSL executable is required.

| CTest group | Responsibility |
| --- | --- |
| `doba_tls_core_tests` | In-memory OpenSSL peer, verified TLS 1.2/1.3, seven-byte plaintext RX, bounded BIO progress, KeyUpdate and stable configuration ownership. |
| `doba_tls_tests` | Native wire tests: fragments, pipeline, source ownership and budgets, failed readers, slow peers, shutdown, admission/recovery, restart, ALPN and authenticated HTTPS. |
| `doba_tls_configuration_tests` | Policies, invalid and mismatched credentials, encrypted-key rejection without prompting and excluded OpenSSL capabilities. Temporary files are removed individually. |
| `doba_tls_header_guard_tests` | An isolated consumer must fail compilation with the explicit diagnostic when it uses HTTPS without the TLS target's definition. |
| `doba_tls_examples_tests` | Both actual TLS example binaries exchange data with verified clients. Registered when examples are enabled. Windows uses a hidden private console for the signal-based examples. |

Every group has an external CTest deadline. Wire operations and child processes
also have bounded waits. Configuration checks require the intended exception or
diagnostic, so an unrelated listener failure cannot satisfy the rejection test.
Assertions remain enabled in Release; Python optimization is rejected explicitly.

`cert.pem` and `key.pem` are public test-only self-signed credentials. Never use
the key for deployment. Clients verify the certificate and hostname. Generated
mismatched/encrypted keys belong to unique temporary directories; cleanup is not
recursive and never alters the source credentials.

See the [contract coverage matrix](../../../docs/reference/transport/server/tls-validation.md).
The tests do not claim a library timeout, mTLS, arbitrary source streaming,
session resumption or authenticated raw TCP EOF notification.
