# Custom transport

This example implements an in-memory transport for doba's HTTP/1.1 server. It
creates one protocol engine, passes it a fixed `GET /hello` request, and copies
the response into a string. The executable prints the HTTP response and exits.
No network listener is opened.

Build and run the `custom_transport` target. The output starts with
`HTTP/1.1 200` and ends with the body `hello from memory`.

The transport shows where incoming and outgoing bytes cross the protocol
boundary. It is a single-request example, not a general HTTP middleware API.
