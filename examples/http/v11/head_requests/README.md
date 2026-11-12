<a name="head-requests"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/head_requests/h1-head-requests.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/head_requests/h1-head-requests.svg" alt="HEAD requests">
  </a>
  <a href="../../../../resources/examples/http/v11/head_requests/h1-head-requests-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/head_requests/h1-head-requests-dark.svg" alt="HEAD requests">
  </a>
</h1>

This example registers the same representation handler for `GET` and `HEAD`.
The server preserves representation headers while suppressing the `HEAD` body.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/head_requests/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/head_requests/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/head_requests/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/head_requests/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/resource
curl -I http://localhost:8080/resource
```

The `HEAD` response has the same `Content-Length` as `GET` and no message body.

