<a name="large-response-bodies"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/large_response_bodies/h1-large-response-bodies.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/large_response_bodies/h1-large-response-bodies.svg" alt="Large response bodies">
  </a>
  <a href="../../../../resources/examples/http/v11/large_response_bodies/h1-large-response-bodies-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/large_response_bodies/h1-large-response-bodies-dark.svg" alt="Large response bodies">
  </a>
</h1>

This example configures a raw body writer with a 1024-byte in-memory spill
threshold, then writes an 8192-byte response.
The 1024-byte threshold is specific to this example; the default is 128 KiB.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/large_response_bodies/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/large_response_bodies/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/large_response_bodies/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/large_response_bodies/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /large` returns 8192 `x` bytes as `application/octet-stream`.

When the threshold is exceeded, the writer moves its storage to a temporary
file without changing the response API.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/large_response_bodies/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/large_response_bodies/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/large_response_bodies/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/large_response_bodies/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl --output large.bin http://localhost:8080/large
```
