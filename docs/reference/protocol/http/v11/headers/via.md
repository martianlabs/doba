<a id="via"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/via/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/via/index.svg" alt="via">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/via/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/via/index-dark.svg" alt="via">
</a>

**Header:** [include/protocol/http/v11/headers/via.h](../../../../../../include/protocol/http/v11/headers/via.h)

|                                                                       via | | RFC 9110 S7.6.3 Via                                                       | | The "Via" header field indicates the presence of intermediate protocols   | | and recipients between the user agent and the server on requests, or      | | between the origin server and the cl

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers

[via](#type-via)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::via

[check](#method-check) ·
[interpret](#method-interpret)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-via"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> via

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class via { ... };
```

|                                                                       via | | RFC 9110 S7.6.3 Via                                                       | | The "Via" header field indicates the presence of intermediate protocols   | | and recipients between the user agent and the server on requests, or      | | between the origin server and the cl

**Source:** [include/protocol/http/v11/headers/via.h:64](../../../../../../include/protocol/http/v11/headers/via.h#L64)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::via*

```cpp
static bool check(std::string_view sv, parsed_via_list& out);
```

Checks whether a value follows the HTTP via header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_via_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/via.h:69](../../../../../../include/protocol/http/v11/headers/via.h#L69)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::via*

```cpp
static constexpr verdict interpret(const parsed_via_list& parsed_list,
                                     v11::connection&,
                                     const policies& policies);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **parsed_list:** Parsed list value. Declaration: `const parsed_via_list& parsed_list`.
- **connection:** Connection being processed. Declaration: `v11::connection&`.
- **policies:** Configuration policies for this component. Declaration: `const policies& policies`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/via.h:99](../../../../../../include/protocol/http/v11/headers/via.h#L99)
