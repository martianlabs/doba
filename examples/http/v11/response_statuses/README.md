<a name="response-statuses"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/response_statuses/h1-response-statuses.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_statuses/h1-response-statuses.svg" alt="Response statuses">
  </a>
  <a href="../../../../resources/examples/http/v11/response_statuses/h1-response-statuses-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_statuses/h1-response-statuses-dark.svg" alt="Response statuses">
  </a>
</h1>

This example uses response status helpers. Headers appear only where the status
semantics require them.

<a name="routes"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/response_statuses/h2-routes.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_statuses/h2-routes.svg" alt="Routes">
  </a>
  <a href="../../../../resources/examples/http/v11/response_statuses/h2-routes-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_statuses/h2-routes-dark.svg" alt="Routes">
  </a>
</h2>

- `POST /resources` returns `201 Created` and `Location: /resources/1`.
- `GET /redirect` returns `307 Temporary Redirect` and a location.
- `DELETE /resources/1` returns `204 No Content` without a body.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/response_statuses/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/response_statuses/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/response_statuses/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/response_statuses/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i -X POST http://localhost:8080/resources
curl -i http://localhost:8080/redirect
curl -i -X DELETE http://localhost:8080/resources/1
```

