# Request headers

This example demonstrates case-insensitive header-name lookup and indexed
iteration over all received header fields.

## Route

- `GET /headers` returns `User-Agent` separately, followed by all headers.

## Try it

```text
curl -i \
  -H "User-Agent: doba-example" \
  -H "X-Example: value" \
  http://localhost:8080/headers
```
