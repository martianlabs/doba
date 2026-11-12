<a name="request-headers"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/request_headers/h1-request-headers.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_headers/h1-request-headers.svg" alt="Request headers">
  </a>
  <a href="../../../../resources/examples/http/v11/request_headers/h1-request-headers-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_headers/h1-request-headers-dark.svg" alt="Request headers">
  </a>
</h1>

This example demonstrates case-insensitive header-name lookup and indexed
iteration over all received header fields.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/request_headers/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_headers/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/request_headers/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_headers/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /headers` returns `User-Agent` separately, followed by all headers.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/request_headers/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_headers/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/request_headers/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_headers/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i \
  -H "User-Agent: doba-example" \
  -H "X-Example: value" \
  http://localhost:8080/headers
```
