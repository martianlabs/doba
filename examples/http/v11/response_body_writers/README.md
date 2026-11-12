<a name="response-body-writers"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/response_body_writers/h1-response-body-writers.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_writers/h1-response-body-writers.svg" alt="Response body writers">
  </a>
  <a href="../../../../resources/examples/http/v11/response_body_writers/h1-response-body-writers-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_writers/h1-response-body-writers-dark.svg" alt="Response body writers">
  </a>
</h1>

This example builds response bodies incrementally with raw and chunked
`body_writer` instances. Writers accept text and `std::byte` spans.

<a name="routes"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/response_body_writers/h2-routes.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_writers/h2-routes.svg" alt="Routes">
  </a>
  <a href="../../../../resources/examples/http/v11/response_body_writers/h2-routes-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_writers/h2-routes-dark.svg" alt="Routes">
  </a>
</h2>

- `GET /raw` writes two text parts and produces a Content-Length response.
- `GET /binary` writes a binary span through a raw writer.
- `GET /chunked` emits two chunks and the terminating chunk.

Moving a writer into the response selects the matching framing headers.
Chunked writers must be finalized with `end()` after the last write.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/response_body_writers/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_writers/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/response_body_writers/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_writers/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/raw
curl --output response.bin http://localhost:8080/binary
curl --raw -i http://localhost:8080/chunked
```
