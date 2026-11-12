<a name="query-parameters"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/query_parameters/h1-query-parameters.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/query_parameters/h1-query-parameters.svg" alt="Query parameters">
  </a>
  <a href="../../../../resources/examples/http/v11/query_parameters/h1-query-parameters-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/query_parameters/h1-query-parameters-dark.svg" alt="Query parameters">
  </a>
</h1>

This example demonstrates optional lookup by parameter name and indexed
iteration over every parsed name-value pair.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/query_parameters/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/query_parameters/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/query_parameters/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/query_parameters/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /search` returns `q` separately, followed by all query parameters.

Lookup by name returns the first matching parameter. Indexed iteration also
exposes repeated names.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/query_parameters/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/query_parameters/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/query_parameters/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/query_parameters/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i "http://localhost:8080/search?q=doba&page=2&q=again"
```
