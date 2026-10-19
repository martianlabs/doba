<a id="content-length-h"></a>
<h1>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/content_length/index.svg#gh-light-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/content_length/index.svg" alt="Index for content_length.h">
  </a>
  <a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/content_length/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/content_length/index-dark.svg" alt="Index for content_length.h">
  </a>
</h1>

**Header:** [include/protocol/http/v11/headers/content_length.h](../../../../../../include/protocol/http/v11/headers/content_length.h)

The "Content-Length" header field indicates the associated representation's data length as a decimal non-negative integer number of octets.

**protocol::http::v11::headers**

- [content_length](#type-content-length) · *type*

**protocol::http::v11::headers::content_length**

- [check](#method-check) · *method*
- [interpret](#method-interpret) · *method*

<a id="type-content-length"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> content_length

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class content_length { ... };
```

The "Content-Length" header field indicates the associated representation's data length as a decimal non-negative integer number of octets.

**Source:** [include/protocol/http/v11/headers/content_length.h:70](../../../../../../include/protocol/http/v11/headers/content_length.h#L70)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::content_length*

```cpp
static constexpr bool check(std::string_view sv, std::size_t& out);
```

Checks whether a value follows the HTTP content-length header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `std::size_t& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/content_length.h:75](../../../../../../include/protocol/http/v11/headers/content_length.h#L75)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::content_length*

```cpp
static constexpr verdict interpret(const std::size_t& len, v11::connection&,
                                     const policies& policies);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **len:** Length in elements or bytes, according to the declared type. Declaration: `const std::size_t& len`.
- **connection:** Connection being processed. Declaration: `v11::connection&`.
- **policies:** Configuration policies for this component. Declaration: `const policies& policies`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/content_length.h:84](../../../../../../include/protocol/http/v11/headers/content_length.h#L84)
