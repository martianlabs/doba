# Query parameters

This example demonstrates optional lookup by parameter name and indexed
iteration over every parsed name-value pair.

## Route

- `GET /search` returns `q` separately, followed by all query parameters.

Lookup by name returns the first matching parameter. Indexed iteration also
exposes repeated names.

## Try it

```text
curl -i "http://localhost:8080/search?q=doba&page=2&q=again"
```
