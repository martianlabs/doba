# Binary request bodies

This example reads decoded request bytes into a `std::byte` buffer and forwards
each produced span to a raw response body writer without text conversion.

## Route

- `POST /echo` returns the received bytes as `application/octet-stream`.

The reader accepts both Content-Length and chunked requests through the same
API. The raw writer preserves every byte and derives the response
`Content-Length` when it is moved into the response.

## Try it

```text
curl --data-binary @input.bin \
  --output echoed.bin \
  http://localhost:8080/echo
```
