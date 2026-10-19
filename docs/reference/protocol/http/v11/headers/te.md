<a id="te-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/index.svg" alt="Index for te.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/headers/te.h](../../../../../../include/protocol/http/v11/headers/te.h)

The "TE" request header field describes the client's capabilities with regard to transfer codings and trailer sections.

**protocol::http::v11::headers**

- [te](#type-te) · *type*

**protocol::http::v11::headers::te**

- [check](#method-check) · *method*
- [interpret](#method-interpret) · *method*

<a id="type-te"></a>

## te

🟠 **TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class te { ... };
```

The "TE" request header field describes the client's capabilities with regard to transfer codings and trailer sections.

**Source:** [include/protocol/http/v11/headers/te.h:76](../../../../../../include/protocol/http/v11/headers/te.h#L76)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::v11::headers::te*

```cpp
static bool check(std::string_view sv, parsed_parameter_list& out);
```

Checks whether a value follows the HTTP te header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_parameter_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/te.h:81](../../../../../../include/protocol/http/v11/headers/te.h#L81)

<a id="method-interpret"></a>

## interpret

🟠 **METHOD** · *protocol::http::v11::headers::te*

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

**Source:** [include/protocol/http/v11/headers/te.h:94](../../../../../../include/protocol/http/v11/headers/te.h#L94)
