<a name="wildcard-routes"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/wildcard_routes/h1-wildcard-routes.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/wildcard_routes/h1-wildcard-routes.svg" alt="Wildcard routes">
  </a>
  <a href="../../../../resources/examples/http/v11/wildcard_routes/h1-wildcard-routes-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/wildcard_routes/h1-wildcard-routes-dark.svg" alt="Wildcard routes">
  </a>
</h1>

This example registers a route whose final segment is a wildcard. The handler
receives every request below `/assets/` and can inspect the complete path
through the request object.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/wildcard_routes/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/wildcard_routes/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/wildcard_routes/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/wildcard_routes/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /assets/*` returns the requested asset path.
- `/assets/`, `/assets/logo.svg`, and nested paths match the route.
- `/assets` does not match because the wildcard follows a slash.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/wildcard_routes/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/wildcard_routes/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/wildcard_routes/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/wildcard_routes/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/assets/logo.svg
curl -i http://localhost:8080/assets/images/logo.svg
```
