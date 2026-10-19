<a id="decoder"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/decoder/index.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/decoder/index.svg" alt="decoder">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/decoder/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/decoder/index-dark.svg" alt="decoder">
</a>

**Header:** [include/protocol/http/v11/decoder.h](../../../../../include/protocol/http/v11/decoder.h)

Provides the http 1.1 decoder implementation.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[decoder](#type-decoder)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::decoder

[deserialize](#method-deserialize) ·
[configuration](#method-configuration) ·
[make_response()](#method-make-response) ·
[make_response(head, body)](#method-make-response-2)

<a id="api-details"></a>
<a href="../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-decoder"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> decoder

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
template <contracts::request RQty, contracts::response RSty>
class decoder { ... };
```

Provides the http 1.1 decoder implementation.

**Template parameters:**

- **RQty:** `contracts::request RQty`.
- **RSty:** `contracts::response RSty`.

**Source:** [include/protocol/http/v11/decoder.h:140](../../../../../include/protocol/http/v11/decoder.h#L140)

<a id="method-deserialize"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> deserialize

**METHOD** · *protocol::http::v11::decoder*

```cpp
deserialization_result<RQty, RSty> deserialize(
      std::string_view& input, std::string* reply_head = nullptr,
      std::string* reply_body = nullptr);
```

Consume one message, retaining only an unfinished message across calls.

**Parameters:**

- **input:** Input data to process. Declaration: `std::string_view& input`.
- **reply_head:** Optional output buffer for the response head. Declaration: `std::string* reply_head = nullptr`.
- **reply_body:** Optional output buffer for the response body. Declaration: `std::string* reply_body = nullptr`.

**Returns:** A value of type `deserialization_result<RQty, RSty>`.

**Source:** [include/protocol/http/v11/decoder.h:170](../../../../../include/protocol/http/v11/decoder.h#L170)

<a id="method-configuration"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> configuration

**METHOD** · *protocol::http::v11::decoder*

```cpp
const policies& configuration() const noexcept;
```

Returns the decoder's configuration.

**Returns:** A value of type `const policies&`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/decoder.h:289](../../../../../include/protocol/http/v11/decoder.h#L289)

<a id="method-make-response"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> make_response

**METHOD** · *protocol::http::v11::decoder*

```cpp
RSty make_response();
```

Creates a response that owns its bytes, with the configured limits.

**Returns:** A value of type `RSty`.

**Source:** [include/protocol/http/v11/decoder.h:295](../../../../../include/protocol/http/v11/decoder.h#L295)

<a id="method-make-response-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> make_response

**METHOD** · *protocol::http::v11::decoder*

```cpp
RSty make_response(std::string& head, std::string& body);
```

Creates response.

**Parameters:**

- **head:** HTTP head data or its output buffer. Declaration: `std::string& head`.
- **body:** HTTP body data or its output buffer. Declaration: `std::string& body`.

**Returns:** A value of type `RSty`.

**Source:** [include/protocol/http/v11/decoder.h:299](../../../../../include/protocol/http/v11/decoder.h#L299)
