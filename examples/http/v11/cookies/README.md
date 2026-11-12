<a name="cookies"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/cookies/h1-cookies.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/cookies/h1-cookies.svg" alt="Cookies">
  </a>
  <a href="../../../../resources/examples/http/v11/cookies/h1-cookies-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/cookies/h1-cookies-dark.svg" alt="Cookies">
  </a>
</h1>

This example demonstrates lookup of one request cookie, iteration over all
request cookies, and multiple `Set-Cookie` response fields.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/cookies/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/cookies/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/cookies/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/cookies/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /cookies` returns the received cookies and sets `session` and `theme`.

Each cookie set by the response uses a separate `Set-Cookie` field.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/cookies/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/cookies/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/cookies/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/cookies/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i \
  --cookie "session=client-session; language=en" \
  http://localhost:8080/cookies
```
