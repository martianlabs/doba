# Response headers

This example adds, replaces, reads, and removes response fields. Header-name
comparison is case-insensitive.

## Route

- `GET /headers` returns `X-Example: replaced`; `X-Remove` is absent.

## Try it

```text
curl -i http://localhost:8080/headers
```

