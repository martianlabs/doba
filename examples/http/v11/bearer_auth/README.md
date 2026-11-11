<a name="bearer-authentication"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/bearer_auth/h1-bearer-authentication.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/bearer_auth/h1-bearer-authentication.svg" alt="Bearer authentication">
  </a>
  <a href="../../../../resources/examples/http/v11/bearer_auth/h1-bearer-authentication-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/bearer_auth/h1-bearer-authentication-dark.svg" alt="Bearer authentication">
  </a>
</h1>

Register a Bearer provider with an application validator, then select
it for **GET /private**. **GET /health** stays public. See the
[source](main.cpp) for the complete server and the
[Bearer API reference](../../../../docs/reference/protocol/http/common/auth/bearer.md)
for its constructor and authentication method.

The hard-coded credentials are for this demo. Use TLS to protect credentials
in deployments using `https_server<>` with the optional TLS target, or an
external TLS terminator. See the [HTTPS example](../https_hello_world/README.md).
This TCP example binds to loopback for local testing.

<a name="run-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/bearer_auth/h2-run-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/bearer_auth/h2-run-it.svg" alt="Run it">
  </a>
  <a href="../../../../resources/examples/http/v11/bearer_auth/h2-run-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/bearer_auth/h2-run-it-dark.svg" alt="Run it">
  </a>
</h2>

From the repository root:

```text
cmake --preset msvc-debug
cmake --build --preset build-debug --target bearer_auth
./out/build/msvc-debug/examples/http/v11/bearer_auth/bearer_auth.exe
```

The server listens on port 8080 until interrupted. Run one authentication
example at a time.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/bearer_auth/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/bearer_auth/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/bearer_auth/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/bearer_auth/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/health
curl -i -H "Authorization: Bearer demo-token" http://localhost:8080/private
curl -i http://localhost:8080/private
curl -i -H "Authorization: Bearer wrong" http://localhost:8080/private
```

The first request returns **200** with **ok**. Valid credentials return
**200** with **Authenticated with Bearer**. Missing or rejected credentials
return **401** with a **WWW-Authenticate** challenge for Bearer; the
protected handler does not run.
