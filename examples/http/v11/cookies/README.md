# Cookies

This example demonstrates lookup of one request cookie, iteration over all
request cookies, and multiple `Set-Cookie` response fields.

## Route

- `GET /cookies` returns the received cookies and sets `session` and `theme`.

Each cookie set by the response uses a separate `Set-Cookie` field.

## Try it

```text
curl -i \
  --cookie "session=client-session; language=en" \
  http://localhost:8080/cookies
```
