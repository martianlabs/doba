<a name="parametrized-routes"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/parametrized_routes/h1-parametrized-routes.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/parametrized_routes/h1-parametrized-routes.svg" alt="Parametrized routes">
  </a>
  <a href="../../../../resources/examples/http/v11/parametrized_routes/h1-parametrized-routes-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/parametrized_routes/h1-parametrized-routes-dark.svg" alt="Parametrized routes">
  </a>
</h1>

This example registers a route with typed path parameters. Doba converts the
`id` segment to `std::uint64_t` and the `detailed` segment to `bool` before
invoking the handler. A path only matches when every conversion succeeds.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/parametrized_routes/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/parametrized_routes/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/parametrized_routes/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/parametrized_routes/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /resources/:id/:detailed` returns the selected resource.
- `detailed` accepts `true`, `false`, `1`, or `0` case-insensitively.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/parametrized_routes/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/parametrized_routes/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/parametrized_routes/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/parametrized_routes/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/resources/42/true
curl -i http://localhost:8080/resources/42/false
```

An invalid typed segment does not match the route:

```text
curl -i http://localhost:8080/resources/not-a-number/true
```
