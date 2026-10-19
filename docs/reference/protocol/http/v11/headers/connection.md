<a id="connection"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/connection/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/connection/index.svg" alt="connection">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/connection/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/connection/index-dark.svg" alt="connection">
</a>

**Header:** [include/protocol/http/v11/headers/connection.h](../../../../../../include/protocol/http/v11/headers/connection.h)

The "Connection" header field allows the sender to list control options that are specific to the current connection.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers

[connection](#type-connection)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::connection

[check](#method-check) ·
[interpret](#method-interpret)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-connection"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> connection

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class connection { ... };
```

The "Connection" header field allows the sender to list control options that are specific to the current connection.

**Source:** [include/protocol/http/v11/headers/connection.h:99](../../../../../../include/protocol/http/v11/headers/connection.h#L99)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::connection*

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Checks whether a value follows the HTTP connection header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_token_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/connection.h:104](../../../../../../include/protocol/http/v11/headers/connection.h#L104)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::connection*

```cpp
static constexpr verdict interpret(const parsed_token_list& token_list,
                                     v11::connection& connection_out,
                                     const policies&);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **token_list:** Parsed token list to process. Declaration: `const parsed_token_list& token_list`.
- **connection_out:** Output connection object. Declaration: `v11::connection& connection_out`.
- **policies:** Configuration policies for this component. Declaration: `const policies&`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/connection.h:116](../../../../../../include/protocol/http/v11/headers/connection.h#L116)
