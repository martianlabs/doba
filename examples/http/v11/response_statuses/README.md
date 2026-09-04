# Response statuses

This example uses response status helpers. Headers appear only where the status
semantics require them.

## Routes

- `POST /resources` returns `201 Created` and `Location: /resources/1`.
- `GET /redirect` returns `307 Temporary Redirect` and a location.
- `DELETE /resources/1` returns `204 No Content` without a body.

## Try it

```text
curl -i -X POST http://localhost:8080/resources
curl -i http://localhost:8080/redirect
curl -i -X DELETE http://localhost:8080/resources/1
```

