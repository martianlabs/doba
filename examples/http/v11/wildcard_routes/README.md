# Wildcard routes

This example registers a route whose final segment is a wildcard. The handler
receives every request below `/assets/` and can inspect the complete path
through the request object.

## Route

- `GET /assets/*` returns the requested asset path.
- `/assets/`, `/assets/logo.svg`, and nested paths match the route.
- `/assets` does not match because the wildcard follows a slash.

## Try it

```text
curl -i http://localhost:8080/assets/logo.svg
curl -i http://localhost:8080/assets/images/logo.svg
```
