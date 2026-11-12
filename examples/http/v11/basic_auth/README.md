<a name="basic-authentication"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/basic_auth/h1-basic-authentication.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/basic_auth/h1-basic-authentication.svg" alt="Basic authentication">
  </a>
  <a href="../../../../resources/examples/http/v11/basic_auth/h1-basic-authentication-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/basic_auth/h1-basic-authentication-dark.svg" alt="Basic authentication">
  </a>
</h1>

Register a Basic provider with an application validator, then select
it for **GET /private**. **GET /health** stays public. See the
[source](main.cpp) for the complete server and the
[Basic API reference](../../../../docs/reference/protocol/http/common/auth/basic.md)
for its constructor and authentication method.

The hard-coded credentials are for this demo. Use TLS to protect credentials
in deployments using `https_server<>` with the optional TLS target, or an
external TLS terminator. See the [HTTPS example](../https_hello_world/README.md).
This TCP example binds to loopback for local testing.

<a name="run-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/basic_auth/h2-run-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/basic_auth/h2-run-it.svg" alt="Run it">
  </a>
  <a href="../../../../resources/examples/http/v11/basic_auth/h2-run-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/basic_auth/h2-run-it-dark.svg" alt="Run it">
  </a>
</h2>

From the repository root:

```text
cmake --preset msvc-debug
cmake --build --preset build-debug --target basic_auth
./out/build/msvc-debug/examples/http/v11/basic_auth/basic_auth.exe
```

The server listens on port 8080 until interrupted. Run one authentication
example at a time.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/basic_auth/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/basic_auth/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/basic_auth/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/basic_auth/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/health
curl -i -u alice:secret http://localhost:8080/private
curl -i http://localhost:8080/private
curl -i -u alice:wrong http://localhost:8080/private
```

The first request returns **200** with **ok**. Valid credentials return
**200** with **Authenticated with Basic**. Missing or rejected credentials
return **401** with a **WWW-Authenticate** challenge for Basic; the
protected handler does not run.
