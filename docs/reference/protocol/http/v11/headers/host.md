<a id="host-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/v11/headers/host/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/host/index.svg" alt="Index for host.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/headers/host.h](../../../../../../include/protocol/http/v11/headers/host.h)

The "Host" header field in a request provides the host and optional port information from the target URI, allowing the origin server to distinguish among resources while servicing requests for multiple host names.

**protocol::http::v11::headers**

- [host](#type-host) · *type*

**protocol::http::v11::headers::host**

- [check](#method-check) · *method*
- [interpret](#method-interpret) · *method*

<a id="type-host"></a>

## host

🟠 **TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class host { ... };
```

The "Host" header field in a request provides the host and optional port information from the target URI, allowing the origin server to distinguish among resources while servicing requests for multiple host names.

**Source:** [include/protocol/http/v11/headers/host.h:71](../../../../../../include/protocol/http/v11/headers/host.h#L71)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::v11::headers::host*

```cpp
static constexpr bool check(std::string_view sv, parsed_host_port& out);
```

Checks whether a value follows the HTTP host header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_host_port& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/host.h:76](../../../../../../include/protocol/http/v11/headers/host.h#L76)

<a id="method-interpret"></a>

## interpret

🟠 **METHOD** · *protocol::http::v11::headers::host*

```cpp
static constexpr verdict interpret(const parsed_host_port&, v11::connection&,
                                     const policies&);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **parsed_host_port:** Parsed host and port value. Declaration: `const parsed_host_port&`.
- **connection:** Connection being processed. Declaration: `v11::connection&`.
- **policies:** Configuration policies for this component. Declaration: `const policies&`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/host.h:85](../../../../../../include/protocol/http/v11/headers/host.h#L85)
