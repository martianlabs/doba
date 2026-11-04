# Custom protocol over TLS

Build with `-DDOBA_ENABLE_TLS=ON`, then run:

```text
tls_echo server-chain.pem server-key.pem
openssl s_client -connect localhost:8443 -servername localhost \
  -CAfile trusted-ca.pem -verify_return_error -verify_hostname localhost
```

Enter `hello` and a newline; the server returns `echo: hello`. The engine owns
line framing and its 4096-byte line limit. TLS handles records and credentials;
TCP handles sockets and worker ownership. No HTTP, executor or coroutine is
involved. The private key must be unencrypted.

An optional final argument selects the listening port instead of 8443. The
[TLS example test](../../../tests/integration/tls/examples.py) runs the built
executable with an isolated port and a verified client.

The engine contract returns processing_result from both input and processing
entry points. This synchronous example returns the empty result and requests
no additional processing. Input bytes remain borrowed only for the call.
Linux and Windows TCP support native continuation turns; TLS forwards them.
This particular example requests none.
