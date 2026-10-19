<a id="x-forwarded-host-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_host/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_host/index.svg" alt="Index for x_forwarded_host.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/headers/x_forwarded_host.h](../../../../../../include/protocol/http/v11/headers/x_forwarded_host.h)

|                                                          x-forwarded-host | | Non-standard / de-facto X-Forwarded-Host                                  | | The "X-Forwarded-Host" header field is a de-facto request header used by  | | proxies, reverse proxies, load balancers, and CDNs to preserve the        | | original Host value requested by the

**protocol::http::v11::headers**

- [x_forwarded_host](#type-x-forwarded-host) · *type*

**protocol::http::v11::headers::x_forwarded_host**

- [check](#method-check) · *method*
- [interpret](#method-interpret) · *method*

<a id="type-x-forwarded-host"></a>

## x_forwarded_host

🟠 **TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class x_forwarded_host { ... };
```

|                                                          x-forwarded-host | | Non-standard / de-facto X-Forwarded-Host                                  | | The "X-Forwarded-Host" header field is a de-facto request header used by  | | proxies, reverse proxies, load balancers, and CDNs to preserve the        | | original Host value requested by the

**Source:** [include/protocol/http/v11/headers/x_forwarded_host.h:65](../../../../../../include/protocol/http/v11/headers/x_forwarded_host.h#L65)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::v11::headers::x_forwarded_host*

```cpp
static constexpr bool check(std::string_view sv, parsed_host_port& out);
```

Checks whether a value follows the HTTP x-forwarded-host header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_host_port& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/x_forwarded_host.h:70](../../../../../../include/protocol/http/v11/headers/x_forwarded_host.h#L70)

<a id="method-interpret"></a>

## interpret

🟠 **METHOD** · *protocol::http::v11::headers::x_forwarded_host*

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

**Source:** [include/protocol/http/v11/headers/x_forwarded_host.h:79](../../../../../../include/protocol/http/v11/headers/x_forwarded_host.h#L79)
