<a id="host"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/host/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/host/title.svg" alt="host">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/host/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/host/title-dark.svg" alt="host">
</a>

**Header:** [include/protocol/http/v11/headers/host.h](../../../../../../include/protocol/http/v11/headers/host.h)

HTTP/1.1 `host` parsing and interpretation.

<a id="api"></a>
<a id="type-host"></a>

## host

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv, parsed_host_port& out);
```

Checks a whole host with optional port, producing borrowed host/port views and a host type. Empty hosts and empty decimal ports are accepted; port numbers have no numeric limit. Output fields may change on failure.

<a id="method-interpret"></a>

## interpret

```cpp
static constexpr verdict interpret(const parsed_host_port&, v11::connection&,
                                     const policies&);
```

Always accepts. Host-presence and request-target consistency checks belong to the decoder.
