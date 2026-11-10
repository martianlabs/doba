# HTTPS hello world

Build with `-DDOBA_ENABLE_TLS=ON` and OpenSSL 3.0 or newer, then run:

```text
https_hello_world server-chain.pem server-key.pem
curl --cacert trusted-ca.pem https://localhost:8443/hello
curl --cacert trusted-ca.pem https://localhost:8443/wait/7
```

The response is `hello over TLS`. The key must be unencrypted; credentials are
loaded before listening. Use a certificate valid for the requested hostname.
This example advertises only HTTP/1.1 through ALPN.

`/wait/:id` is a cooperative coroutine handler. It waits ten milliseconds and
returns the route value; `/wait/7` returns `7`. Sync and async handlers use the
same engine automatically. Pipelined responses remain in request order.
TLS forwards processing turns to the existing TCP workers and reuses TCP
unchanged. Sources are consumed completely during their send callback and
must fit the finite plaintext budget for that input or processing call;
see the [TLS contract](../../../../docs/reference/transport/tls.md).

An optional final argument selects the listening port instead of 8443. The
[TLS example test](../../../../tests/integration/tls/examples_wire.cpp) runs the built
executable with an isolated port and a verified client.
