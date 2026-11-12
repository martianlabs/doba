<a name="controllers"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/controllers/h1-controllers.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/controllers/h1-controllers.svg" alt="Controllers">
  </a>
  <a href="../../../../resources/examples/http/v11/controllers/h1-controllers-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/controllers/h1-controllers-dark.svg" alt="Controllers">
  </a>
</h1>

This example registers two instances of `counter_controller`, each with a
constructor-supplied prefix. `register_routes(registrar&)` binds member functions
using `registrar.add(method, path, &controller::member)`.

<a name="run"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/controllers/h2-run.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/controllers/h2-run.svg" alt="Run">
  </a>
  <a href="../../../../resources/examples/http/v11/controllers/h2-run-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/controllers/h2-run-dark.svg" alt="Run">
  </a>
</h2>

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

Members return `void` and accept `const request&`, then `response&`, followed
by typed route arguments. They support `const` and `noexcept` where the
implementation permits them.
