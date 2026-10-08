# Custom protocol

This example implements a line-based protocol engine and runs it on doba's TCP
transport. Each connection has its own engine. Complete lines are returned as
`echo: <line>`; an incomplete line remains in the transport's receive buffer
until more bytes arrive. A line longer than the receive buffer closes the
connection.

Build the `custom_protocol` target, run it, and connect to `localhost:8080`.
For example, send `hello\n` with a TCP client to receive `echo: hello\n`.
Stop the server with Ctrl+C.
