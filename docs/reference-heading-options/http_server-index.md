<a name="http-server"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/heading-options/http-server-index-dark.svg">
    <img src="../../resources/docs/reference/heading-options/http-server-index.svg" alt="Index for http_server.h">
  </picture>
</h1>

**Header:** [include/http_server.h](../../include/http_server.h)

This is a layout sample. The finished page will index every public declaration
in this header. Here, one method is enough to show how the index works:

- **Methods:** [add_route](#add-route)

<a name="add-route"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/reference/heading-options/option-a-method-dark.svg">
    <img src="../../resources/docs/reference/heading-options/option-a-method.svg" alt="Method: add_route">
  </picture>
</h2>

```cpp
template <typename Hty>
  requires protocol::http::router_handler_lambda<Hty>
http_server& add_route(std::string_view method, std::string_view route,
                       Hty handler);
```

Registers a route in the server's router. The handler is moved into the
router. Registration is unavailable while the server is running.

**Parameters:**

- **Hty:** Handler type, deduced from *handler*.
- **method:** HTTP method to match, such as *GET*.
- **route:** Path pattern to match.
- **handler:** Callable to run when the route matches.

**Returns:** A reference to the same *http_server*, so calls can be chained.

**Errors:** Throws *std::runtime_error* when registration is closed. The
router can throw *std::invalid_argument* for an invalid wildcard or a
mismatch between route parameters and handler arguments.
