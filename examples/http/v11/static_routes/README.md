# Static routes

This example registers independent handlers for several HTTP methods on the
same static path. It also demonstrates the predefined method-name constants
and common response status helpers.

## Routes

- `GET /resources` returns a resource list.
- `POST /resources` returns `201 Created` with a `Location` header.
- `PUT /resources` returns the replaced representation.
- `DELETE /resources` returns `204 No Content`.

## Try it

```text
curl -i http://localhost:8080/resources
curl -i -X POST http://localhost:8080/resources
curl -i -X PUT http://localhost:8080/resources
curl -i -X DELETE http://localhost:8080/resources
```
