<a id="transfer-encoding-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/transfer_encoding/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/transfer_encoding/index.svg" alt="Index for transfer_encoding.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/transfer_encoding/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/transfer_encoding/index-dark.svg" alt="Index for transfer_encoding.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/headers/transfer_encoding.h](../../../../../../include/protocol/http/v11/headers/transfer_encoding.h)

The "Transfer-Encoding" header field lists the transfer coding names corresponding to the sequence of transfer codings that have been, or will be, applied to the content in order to form the message body.

**protocol::http::v11::headers**

- [transfer_encoding](#type-transfer-encoding) · *type*

**protocol::http::v11::headers::transfer_encoding**

- [check](#method-check) · *method*
- [interpret](#method-interpret) · *method*

<a id="type-transfer-encoding"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transfer_encoding

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class transfer_encoding { ... };
```

The "Transfer-Encoding" header field lists the transfer coding names corresponding to the sequence of transfer codings that have been, or will be, applied to the content in order to form the message body.

**Source:** [include/protocol/http/v11/headers/transfer_encoding.h:62](../../../../../../include/protocol/http/v11/headers/transfer_encoding.h#L62)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::transfer_encoding*

```cpp
static bool check(std::string_view sv, parsed_parameter_list& out);
```

Checks whether a value follows the HTTP transfer-encoding header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_parameter_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/transfer_encoding.h:67](../../../../../../include/protocol/http/v11/headers/transfer_encoding.h#L67)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::transfer_encoding*

```cpp
static constexpr verdict interpret(
      const parsed_parameter_list& parameters_list, v11::connection& conn,
      const policies& pol);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **parameters_list:** Parsed parameter list. Declaration: `const parsed_parameter_list& parameters_list`.
- **conn:** Connection being processed. Declaration: `v11::connection& conn`.
- **pol:** Policies controlling this operation. Declaration: `const policies& pol`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/transfer_encoding.h:80](../../../../../../include/protocol/http/v11/headers/transfer_encoding.h#L80)
