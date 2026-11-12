<a id="x-forwarded-host"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_host/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_host/title.svg" alt="x_forwarded_host">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_host/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_host/title-dark.svg" alt="x_forwarded_host">
</a>

**Header:** [include/protocol/http/v11/headers/x_forwarded_host.h](../../../../../../include/protocol/http/v11/headers/x_forwarded_host.h)

HTTP/1.1 `x-forwarded-host` parsing and interpretation.

<a id="api"></a>
<a id="type-x-forwarded-host"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> x_forwarded_host

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv, parsed_host_port& out);
```

Checks one whole host with optional port, producing borrowed views and a host type. Empty hosts and empty decimal ports are accepted; port numbers have no numeric limit. Output fields may change on failure.

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

```cpp
static constexpr verdict interpret(const parsed_host_port&, v11::connection&,
                                     const policies&);
```

Always accepts without changing the connection. It does not replace the request authority or establish proxy trust.
