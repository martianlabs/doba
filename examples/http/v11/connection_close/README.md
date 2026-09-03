# Connection close

This example shows the server's handling of a client request to close the HTTP
connection. It adds `Connection: close` to the response and closes the channel
after that response.

## Try it

```text
curl -i -H "Connection: close" http://localhost:8080/resource
```

