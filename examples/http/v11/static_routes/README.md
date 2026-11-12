<a name="static-routes"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/static_routes/h1-static-routes.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/static_routes/h1-static-routes.svg" alt="Static routes">
  </a>
  <a href="../../../../resources/examples/http/v11/static_routes/h1-static-routes-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/static_routes/h1-static-routes-dark.svg" alt="Static routes">
  </a>
</h1>

This example registers independent handlers for several HTTP methods on the
same static path. It also demonstrates the predefined method-name constants
and common response status helpers.

<a name="routes"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/static_routes/h2-routes.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/static_routes/h2-routes.svg" alt="Routes">
  </a>
  <a href="../../../../resources/examples/http/v11/static_routes/h2-routes-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/static_routes/h2-routes-dark.svg" alt="Routes">
  </a>
</h2>

- `GET /resources` returns a resource list.
- `POST /resources` returns `201 Created` with a `Location` header.
- `PUT /resources` returns the replaced representation.
- `DELETE /resources` returns `204 No Content`.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/static_routes/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/static_routes/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/static_routes/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/static_routes/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/resources
curl -i -X POST http://localhost:8080/resources
curl -i -X PUT http://localhost:8080/resources
curl -i -X DELETE http://localhost:8080/resources
```
