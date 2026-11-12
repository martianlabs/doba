<a name="text-request-bodies"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/request_body_text/h1-text-request-bodies.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_text/h1-text-request-bodies.svg" alt="Text request bodies">
  </a>
  <a href="../../../../resources/examples/http/v11/request_body_text/h1-text-request-bodies-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_text/h1-text-request-bodies-dark.svg" alt="Text request bodies">
  </a>
</h1>

This example reads a request body incrementally into a string. The body reader
exposes decoded payload bytes and hides whether the request used
`Content-Length` or chunked framing.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/request_body_text/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_text/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/request_body_text/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_text/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `POST /echo` returns the received text as `text/plain`.

Each call to `read()` reports the produced byte count, completion state, and
any read error.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/request_body_text/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_text/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/request_body_text/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_text/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

Content-Length request:

```text
curl -i --data-binary "hello doba" http://localhost:8080/echo
```

Chunked request:

```text
curl -i --http1.1 \
  -H "Transfer-Encoding: chunked" \
  --data-binary "hello doba" \
  http://localhost:8080/echo
```
