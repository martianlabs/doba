<a name="binary-request-bodies"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/request_body_binary/h1-binary-request-bodies.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_binary/h1-binary-request-bodies.svg" alt="Binary request bodies">
  </a>
  <a href="../../../../resources/examples/http/v11/request_body_binary/h1-binary-request-bodies-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_binary/h1-binary-request-bodies-dark.svg" alt="Binary request bodies">
  </a>
</h1>

This example reads decoded request bytes into a `std::byte` buffer and forwards
each produced span to a raw response body writer without text conversion.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/request_body_binary/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_binary/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/request_body_binary/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_binary/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `POST /echo` returns the received bytes as `application/octet-stream`.

The reader accepts both Content-Length and chunked requests through the same
API. The raw writer preserves every byte and derives the response
`Content-Length` when it is moved into the response.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/request_body_binary/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_binary/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/request_body_binary/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_body_binary/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl --data-binary @input.bin \
  --output echoed.bin \
  http://localhost:8080/echo
```
