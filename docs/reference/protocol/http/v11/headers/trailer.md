<a id="trailer-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../../resources/docs/reference/protocol/http/v11/headers/trailer/index-dark.svg">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/trailer/index.svg" alt="Index for trailer.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/headers/trailer.h](../../../../../../include/protocol/http/v11/headers/trailer.h)

The "Trailer" header field provides a list of field names that the sender anticipates sending as trailer fields within the same message.

**protocol::http::v11::headers**

- [trailer](#type-trailer) · *type*

**protocol::http::v11::headers::trailer**

- [check](#method-check) · *method*
- [interpret](#method-interpret) · *method*

<a id="type-trailer"></a>

## trailer

🟠 **TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class trailer { ... };
```

The "Trailer" header field provides a list of field names that the sender anticipates sending as trailer fields within the same message.

**Source:** [include/protocol/http/v11/headers/trailer.h:58](../../../../../../include/protocol/http/v11/headers/trailer.h#L58)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::v11::headers::trailer*

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Checks whether a value follows the HTTP trailer header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_token_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/trailer.h:63](../../../../../../include/protocol/http/v11/headers/trailer.h#L63)

<a id="method-interpret"></a>

## interpret

🟠 **METHOD** · *protocol::http::v11::headers::trailer*

```cpp
static constexpr verdict interpret(const parsed_token_list& token_list,
                                     v11::connection& conn, const policies&);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **token_list:** Parsed token list to process. Declaration: `const parsed_token_list& token_list`.
- **conn:** Connection being processed. Declaration: `v11::connection& conn`.
- **policies:** Configuration policies for this component. Declaration: `const policies&`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/trailer.h:75](../../../../../../include/protocol/http/v11/headers/trailer.h#L75)
