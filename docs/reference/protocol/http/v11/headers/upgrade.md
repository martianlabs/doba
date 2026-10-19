<a id="upgrade"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/upgrade/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/upgrade/index.svg" alt="upgrade">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/upgrade/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/upgrade/index-dark.svg" alt="upgrade">
</a>

**Header:** [include/protocol/http/v11/headers/upgrade.h](../../../../../../include/protocol/http/v11/headers/upgrade.h)

|                                                                   upgrade | | RFC 9110 S7.8 Upgrade                                                     | | The "Upgrade" header field provides a mechanism for transitioning from    | | HTTP/1.1 to another protocol on the same connection.                      | | A client MAY send a list of protocol

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers

[upgrade](#type-upgrade)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::upgrade

[check](#method-check) ·
[interpret](#method-interpret)

<a id="type-upgrade"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> upgrade

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class upgrade { ... };
```

|                                                                   upgrade | | RFC 9110 S7.8 Upgrade                                                     | | The "Upgrade" header field provides a mechanism for transitioning from    | | HTTP/1.1 to another protocol on the same connection.                      | | A client MAY send a list of protocol

**Source:** [include/protocol/http/v11/headers/upgrade.h:69](../../../../../../include/protocol/http/v11/headers/upgrade.h#L69)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::upgrade*

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Checks whether a value follows the HTTP upgrade header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_token_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/upgrade.h:74](../../../../../../include/protocol/http/v11/headers/upgrade.h#L74)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::upgrade*

```cpp
static constexpr verdict interpret(const parsed_token_list& token_list,
                                     v11::connection& conn,
                                     const policies& pol);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **token_list:** Parsed token list to process. Declaration: `const parsed_token_list& token_list`.
- **conn:** Connection being processed. Declaration: `v11::connection& conn`.
- **pol:** Policies controlling this operation. Declaration: `const policies& pol`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/upgrade.h:87](../../../../../../include/protocol/http/v11/headers/upgrade.h#L87)
