# HTTPS hello world

Build with `-DDOBA_ENABLE_TLS=ON` and OpenSSL 3.0 or newer, then run:

```text
https_hello_world server-chain.pem server-key.pem
curl --cacert trusted-ca.pem https://localhost:8443/hello
```

The response is `hello over TLS`. The key must be unencrypted; credentials are
loaded before listening. Use a certificate valid for the requested hostname.
This example advertises only HTTP/1.1 through ALPN.

TLS reuses TCP unchanged. Engines remain synchronous. Sources are consumed
completely during their send callback and must fit the finite plaintext budget;
see the [TLS contract](../../../../docs/reference/transport/server/tls.md).

An optional final argument selects the listening port instead of 8443. The
[TLS example test](../../../../tests/integration/tls/examples.py) runs the built
executable with an isolated port and a verified client.
