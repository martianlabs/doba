# Controllers

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
```

The bodies are `1`, `2`, `1`, and `42`. The counter uses an atomic because
requests to one controller can run concurrently.

`server.add_controller<Type>(constructor_arguments...)` constructs one owned
instance per call; controllers do not need to be copyable or movable.
Registration must add at least one route. If construction or registration
throws, none of that controller's routes remain registered. Registration is
rejected while the server runs; stopping and restarting preserves instances.

Members return `response` and accept `const request&`, followed by typed route
arguments. They support `const` and `noexcept` where the implementation permits
them.
