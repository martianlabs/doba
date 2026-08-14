# Response status and headers

This example combines status helpers with response header creation,
replacement, lookup, iteration, and removal.

## Routes

- `POST /resources` returns `201 Created` with `Location: /resources/1`.
- `GET /redirect` returns a `307 Temporary Redirect` to `/json`.
- `GET /json` returns JSON with CORS and cache-control fields.
- `GET /html` returns a small HTML representation.
- `GET /headers` demonstrates `add_header()`, `set_header()`,
  `remove_header()`, `get_header()`, and `get_headers_length()`.
- `DELETE /resources` returns `204 No Content`.

## Try it

```text
curl -i -X POST http://localhost:8080/resources
curl -i http://localhost:8080/redirect
curl -i http://localhost:8080/headers
curl -i -X DELETE http://localhost:8080/resources
```
