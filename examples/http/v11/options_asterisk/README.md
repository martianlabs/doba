# OPTIONS asterisk

This example has no routes. `OPTIONS *` addresses the server as a whole and is
handled directly with `200 OK`.

## Try it

```text
curl -i --request-target "*" -X OPTIONS http://localhost:8080
```

