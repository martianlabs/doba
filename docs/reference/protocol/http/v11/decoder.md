<a id="decoder-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/decoder/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/decoder/index.svg" alt="Index for decoder.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/decoder.h](../../../../../include/protocol/http/v11/decoder.h)

Provides the http 1.1 decoder implementation.

**protocol::http::v11**

- [decoder](#type-decoder) · *type*

**protocol::http::v11::decoder**

- [decoder(configuration)](#constructor-decoder) · *constructor*
- [decoder(decoder)](#constructor-decoder-2) · *constructor*
- [decoder(decoder)](#constructor-decoder-3) · *constructor*
- [~decoder](#destructor-destructor-decoder) · *destructor*
- [operator=(decoder)](#operator-operator) · *operator*
- [operator=(decoder)](#operator-operator-2) · *operator*
- [deserialize](#method-deserialize) · *method*
- [configuration](#method-configuration) · *method*
- [make_response()](#method-make-response) · *method*
- [make_response(head, body)](#method-make-response-2) · *method*

<a id="type-decoder"></a>

## decoder

🟠 **TYPE** · *protocol::http::v11*

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

<a id="constructor-decoder"></a>

## decoder

🟠 **CONSTRUCTOR** · *protocol::http::v11::decoder*

```cpp
explicit decoder(policies configuration = {});
```

Constructs a decoder value.

**Parameters:**

- **configuration:** Configuration for this component. Declaration: `policies configuration = {}`.

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/decoder.h:150](../../../../../include/protocol/http/v11/decoder.h#L150)

<a id="constructor-decoder-2"></a>

## decoder

🟠 **CONSTRUCTOR** · *protocol::http::v11::decoder*

```cpp
decoder(const decoder&) = delete;
```

Disallows this form of protocol::http::v11::decoder construction.

**Parameters:**

- **decoder:** Source object to copy or move. Declaration: `const decoder&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/decoder.h:157](../../../../../include/protocol/http/v11/decoder.h#L157)

<a id="constructor-decoder-3"></a>

## decoder

🟠 **CONSTRUCTOR** · *protocol::http::v11::decoder*

```cpp
decoder(decoder&&) noexcept = delete;
```

Disallows this form of protocol::http::v11::decoder construction.

**Parameters:**

- **decoder:** Source object to copy or move. Declaration: `decoder&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/decoder.h:158](../../../../../include/protocol/http/v11/decoder.h#L158)

<a id="destructor-destructor-decoder"></a>

## ~decoder

🟠 **DESTRUCTOR** · *protocol::http::v11::decoder*

```cpp
~decoder() = default;
```

Destroys the decoder object.

**Source:** [include/protocol/http/v11/decoder.h:159](../../../../../include/protocol/http/v11/decoder.h#L159)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::decoder*

```cpp
decoder& operator=(const decoder&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **decoder:** Source object to copy or move. Declaration: `const decoder&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/decoder.h:163](../../../../../include/protocol/http/v11/decoder.h#L163)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::decoder*

```cpp
decoder& operator=(decoder&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **decoder:** Source object to copy or move. Declaration: `decoder&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/decoder.h:164](../../../../../include/protocol/http/v11/decoder.h#L164)

<a id="method-deserialize"></a>

## deserialize

🟠 **METHOD** · *protocol::http::v11::decoder*

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

## configuration

🟠 **METHOD** · *protocol::http::v11::decoder*

```cpp
const policies& configuration() const noexcept;
```

Returns the decoder's configuration.

**Returns:** A value of type `const policies&`.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/decoder.h:289](../../../../../include/protocol/http/v11/decoder.h#L289)

<a id="method-make-response"></a>

## make_response

🟠 **METHOD** · *protocol::http::v11::decoder*

```cpp
RSty make_response();
```

Creates a response that owns its bytes, with the configured limits.

**Returns:** A value of type `RSty`.

**Source:** [include/protocol/http/v11/decoder.h:295](../../../../../include/protocol/http/v11/decoder.h#L295)

<a id="method-make-response-2"></a>

## make_response

🟠 **METHOD** · *protocol::http::v11::decoder*

```cpp
RSty make_response(std::string& head, std::string& body);
```

Creates response.

**Parameters:**

- **head:** HTTP head data or its output buffer. Declaration: `std::string& head`.
- **body:** HTTP body data or its output buffer. Declaration: `std::string& body`.

**Returns:** A value of type `RSty`.

**Source:** [include/protocol/http/v11/decoder.h:299](../../../../../include/protocol/http/v11/decoder.h#L299)
