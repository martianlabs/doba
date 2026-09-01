# Text request bodies

This example reads a request body incrementally into a string. The body reader
exposes decoded payload bytes and hides whether the request used
`Content-Length` or chunked framing.

## Route

- `POST /echo` returns the received text as `text/plain`.

Each call to `read()` reports the produced byte count, completion state, and
any read error.

## Try it

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
