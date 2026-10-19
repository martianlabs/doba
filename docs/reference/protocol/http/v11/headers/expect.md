<a id="expect-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/v11/headers/expect/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/expect/index.svg" alt="Index for expect.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/headers/expect.h](../../../../../../include/protocol/http/v11/headers/expect.h)

The "Expect" header field in a request indicates a set of behaviors ("expectations") that need to be supported by the server in order to properly handle the request.

**protocol::http::v11::headers**

- [expect](#type-expect) · *type*

**protocol::http::v11::headers::expect**

- [check](#method-check) · *method*
- [interpret](#method-interpret) · *method*

<a id="type-expect"></a>

## expect

🟠 **TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class expect { ... };
```

The "Expect" header field in a request indicates a set of behaviors ("expectations") that need to be supported by the server in order to properly handle the request.

**Source:** [include/protocol/http/v11/headers/expect.h:111](../../../../../../include/protocol/http/v11/headers/expect.h#L111)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::v11::headers::expect*

```cpp
static bool check(std::string_view sv, parsed_parameter_list& out);
```

Checks whether a value follows the HTTP expect header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_parameter_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/expect.h:116](../../../../../../include/protocol/http/v11/headers/expect.h#L116)

<a id="method-interpret"></a>

## interpret

🟠 **METHOD** · *protocol::http::v11::headers::expect*

```cpp
static constexpr verdict interpret(
      const parsed_parameter_list& parameters_list, v11::connection& conn,
      const policies&);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **parameters_list:** Parsed parameter list. Declaration: `const parsed_parameter_list& parameters_list`.
- **conn:** Connection being processed. Declaration: `v11::connection& conn`.
- **policies:** Configuration policies for this component. Declaration: `const policies&`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/expect.h:129](../../../../../../include/protocol/http/v11/headers/expect.h#L129)
