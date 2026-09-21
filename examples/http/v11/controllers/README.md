# Controllers

Migration status: synchronous controller routes run normally. The current engine
returns `501 Not Implemented` for the async route; its intended response and
cancellation behavior below remain pending migration.

This example registers two instances of `counter_controller`, each with a
constructor-supplied prefix. `register_routes(registrar&)` binds member functions
using `registrar.add(method, path, &controller::member)`.

## Run

Build and run `controllers`, then:

```sh
curl http://localhost:8080/first/count
curl http://localhost:8080/first/count
curl http://localhost:8080/second/count
curl http://localhost:8080/first/echo/42
curl http://localhost:8080/first/async/42
```

The bodies are `1`, `2`, `1`, `42`, and `42`. The counter uses an atomic because
requests to one controller can run concurrently.

`server.add_controller<Type>(constructor_arguments...)` constructs one owned
instance per call; controllers do not need to be copyable or movable.
Registration must add at least one route. If construction or registration
throws, none of that controller's routes remain registered. Registration is
rejected while the server runs; stopping and restarting preserves instances.

Synchronous members return `response` and accept `const request&`, followed by
typed route arguments. Asynchronous members return `task<response>` and accept
`shared_ptr<const request>`, `stop_token`, and typed arguments. Both support
`const` and `noexcept` where the implementation permits them.

The async endpoint completes immediately to keep this example focused on
registration. See [asynchronous routes](../asynchronous_routes) for an executor
that actually suspends work. Pending tasks retain the controller until they
finish, even after server shutdown; cancellation remains cooperative.
