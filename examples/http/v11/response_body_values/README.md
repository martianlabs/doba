<a name="response-body-values"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/response_body_values/h1-response-body-values.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_values/h1-response-body-values.svg" alt="Response body values">
  </a>
  <a href="../../../../resources/examples/http/v11/response_body_values/h1-response-body-values-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_values/h1-response-body-values-dark.svg" alt="Response body values">
  </a>
</h1>

This example demonstrates the direct `set_body()` overloads for text, binary
data with an explicit size, and arithmetic values.

<a name="routes"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/response_body_values/h2-routes.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_values/h2-routes.svg" alt="Routes">
  </a>
  <a href="../../../../resources/examples/http/v11/response_body_values/h2-routes-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_values/h2-routes-dark.svg" alt="Routes">
  </a>
</h2>

- `GET /text` sets a body from `std::string`.
- `GET /binary` preserves an embedded zero byte through `std::string_view`.
- `GET /integer` sets the body from an `int`.
- `GET /floating-point` sets the body from a `double`.

Arithmetic values use `std::to_string`; the floating-point route therefore
returns `3.140000`.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/response_body_values/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_values/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/response_body_values/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_body_values/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/integer
curl -i http://localhost:8080/floating-point
curl --output response.bin http://localhost:8080/binary
```
