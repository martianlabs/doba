<a id="te"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/index.svg" alt="te">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/index-dark.svg" alt="te">
</a>

**Header:** [include/protocol/http/v11/headers/te.h](../../../../../../include/protocol/http/v11/headers/te.h)

The "TE" request header field describes the client's capabilities with regard to transfer codings and trailer sections.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers

[te](#type-te)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::te

[check](#method-check) ·
[interpret](#method-interpret)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-te"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> te

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class te { ... };
```

The "TE" request header field describes the client's capabilities with regard to transfer codings and trailer sections.

**Source:** [include/protocol/http/v11/headers/te.h:76](../../../../../../include/protocol/http/v11/headers/te.h#L76)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::te*

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

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::te*

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
