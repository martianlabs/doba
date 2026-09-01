# Large response bodies

This example configures a raw body writer with a 1024-byte in-memory spill
threshold, then writes an 8192-byte response.

## Route

- `GET /large` returns 8192 `x` bytes as `application/octet-stream`.

When the threshold is exceeded, the writer moves its storage to a temporary
file without changing the response API.

## Try it

```text
curl --output large.bin http://localhost:8080/large
```
