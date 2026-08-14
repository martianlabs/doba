# Response body values

This example demonstrates the direct `set_body()` overloads for text, binary
data with an explicit size, and arithmetic values.

## Routes

- `GET /text` sets a body from `std::string`.
- `GET /binary` preserves an embedded zero byte through `std::string_view`.
- `GET /integer` sets the body from an `int`.
- `GET /floating-point` sets the body from a `double`.

Arithmetic values use `std::to_string`; the floating-point route therefore
returns `3.140000`.

## Try it

```text
curl -i http://localhost:8080/integer
curl -i http://localhost:8080/floating-point
curl --output response.bin http://localhost:8080/binary
```
