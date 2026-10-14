# Bearer authentication

Register a Bearer provider with an application validator, then select
it for **GET /private**. **GET /health** stays public. See the
[source](main.cpp) for the complete server and the
[authentication reference](../../../../docs/reference/http/authentication.md)
for the provider contract.

The hard-coded credentials are for this demo. Use TLS to protect credentials
in deployments; the [HTTPS example](../https_hello_world/README.md) shows
transport setup. This example binds to loopback for local testing.

## Run it

From the repository root:

```text
cmake --preset msvc-debug
cmake --build --preset build-debug --target bearer_auth
./out/build/msvc-debug/examples/http/v11/bearer_auth/bearer_auth.exe
```

The server listens on port 8080 until interrupted. Run one authentication
example at a time.

## Try it

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
