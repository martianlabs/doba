<a name="response-headers"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/response_headers/h1-response-headers.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_headers/h1-response-headers.svg" alt="Response headers">
  </a>
  <a href="../../../../resources/examples/http/v11/response_headers/h1-response-headers-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_headers/h1-response-headers-dark.svg" alt="Response headers">
  </a>
</h1>

This example adds, replaces, reads, and removes response fields. Header-name
comparison is case-insensitive.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/response_headers/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_headers/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/response_headers/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_headers/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /headers` returns `X-Example: replaced`; `X-Remove` is absent.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/response_headers/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_headers/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/response_headers/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_headers/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/headers
```

