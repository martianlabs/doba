# Wildcard routes

This example registers a route ending in `*`. The wildcard matches any request
path that starts with the route prefix, including nested segments.

## Route

- `GET /assets/*` returns the part of the path following `/assets/`.

Wildcard handlers receive the complete request path. The example removes the
known prefix to obtain the matched suffix.

## Try it

```text
curl -i http://localhost:8080/assets/images/logo.svg
```
