<a id="engine-sync-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/engine_sync/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/engine_sync/index.svg" alt="Index for engine_sync.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/engine_sync.h](../../../../../include/protocol/http/v11/engine_sync.h)

The synchronous engine handles the processing of a single request and response in a blocking manner. It is designed for environments where asynchronous processing is not required or desired. It provides methods for executing the request against the router, building error responses, and serializing the response.

**protocol::http::v11**

- [engine_sync](#type-engine-sync) · *type*

**protocol::http::v11::engine_sync**

- [engine_sync()](#constructor-engine-sync) · *constructor*
- [engine_sync(engine_sync)](#constructor-engine-sync-2) · *constructor*
- [engine_sync(engine_sync)](#constructor-engine-sync-3) · *constructor*
- [~engine_sync](#destructor-destructor-engine-sync) · *destructor*
- [operator=(engine_sync)](#operator-operator) · *operator*
- [operator=(engine_sync)](#operator-operator-2) · *operator*
- [handle_request](#method-handle-request) · *method*
- [handle_response](#method-handle-response) · *method*
- [execute](#method-execute) · *method*
- [authenticate](#method-authenticate) · *method*
- [build_error_response](#method-build-error-response) · *method*
- [serialize(response, close)](#method-serialize) · *method*
- [serialize(request, response, close)](#method-serialize-2) · *method*

<a id="type-engine-sync"></a>

## engine_sync

🟠 **TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty, typename ROty>
struct engine_sync { ... };
```

The synchronous engine handles the processing of a single request and response in a blocking manner. It is designed for environments where asynchronous processing is not required or desired. It provides methods for executing the request against the router, building error responses, and serializing the response.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **ROty:** `typename ROty`.

**Source:** [include/protocol/http/v11/engine_sync.h:54](../../../../../include/protocol/http/v11/engine_sync.h#L54)

<a id="constructor-engine-sync"></a>

## engine_sync

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine_sync*

```cpp
engine_sync() = default;
```

Constructs a engine_sync value.

**Source:** [include/protocol/http/v11/engine_sync.h:58](../../../../../include/protocol/http/v11/engine_sync.h#L58)

<a id="constructor-engine-sync-2"></a>

## engine_sync

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine_sync*

```cpp
engine_sync(const engine_sync&) = delete;
```

Disallows this form of protocol::http::v11::engine_sync construction.

**Parameters:**

- **engine_sync:** Source object to copy or move. Declaration: `const engine_sync&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine_sync.h:59](../../../../../include/protocol/http/v11/engine_sync.h#L59)

<a id="constructor-engine-sync-3"></a>

## engine_sync

🟠 **CONSTRUCTOR** · *protocol::http::v11::engine_sync*

```cpp
engine_sync(engine_sync&&) noexcept = delete;
```

Disallows this form of protocol::http::v11::engine_sync construction.

**Parameters:**

- **engine_sync:** Source object to copy or move. Declaration: `engine_sync&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine_sync.h:60](../../../../../include/protocol/http/v11/engine_sync.h#L60)

<a id="destructor-destructor-engine-sync"></a>

## ~engine_sync

🟠 **DESTRUCTOR** · *protocol::http::v11::engine_sync*

```cpp
~engine_sync() = default;
```

Destroys the engine_sync object.

**Source:** [include/protocol/http/v11/engine_sync.h:61](../../../../../include/protocol/http/v11/engine_sync.h#L61)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::engine_sync*

```cpp
engine_sync& operator=(const engine_sync&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **engine_sync:** Source object to copy or move. Declaration: `const engine_sync&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine_sync.h:65](../../../../../include/protocol/http/v11/engine_sync.h#L65)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::engine_sync*

```cpp
engine_sync& operator=(engine_sync&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **engine_sync:** Source object to copy or move. Declaration: `engine_sync&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/v11/engine_sync.h:66](../../../../../include/protocol/http/v11/engine_sync.h#L66)

<a id="method-handle-request"></a>

## handle_request

🟠 **METHOD** · *protocol::http::v11::engine_sync*

```cpp
static bool handle_request(decoder<RQty, RSty>& owner, const ROty& router,
                             const RQty& request,
                             const router_match_result<RQty, RSty>& match,
                             const protocol::send_delegate& on_send,
                             std::string& head, std::string& body);
```

Handles an incoming request synchronously. It executes the request against the router, builds the response, serializes it, and submits it to the transport. It returns true if the connection should be closed after the response is sent.

**Parameters:**

- **owner:** Owning decoder or context. Declaration: `decoder<RQty, RSty>& owner`.
- **router:** Router used for request dispatch. Declaration: `const ROty& router`.
- **request:** Request being processed. Declaration: `const RQty& request`.
- **match:** Match result to inspect or update. Declaration: `const router_match_result<RQty, RSty>& match`.
- **on_send:** Callback for the named event. Declaration: `const protocol::send_delegate& on_send`.
- **head:** HTTP head data or its output buffer. Declaration: `std::string& head`.
- **body:** HTTP body data or its output buffer. Declaration: `std::string& body`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/engine_sync.h:75](../../../../../include/protocol/http/v11/engine_sync.h#L75)

<a id="method-handle-response"></a>

## handle_response

🟠 **METHOD** · *protocol::http::v11::engine_sync*

```cpp
static void handle_response(RSty& response, bool close,
                              const protocol::send_delegate& on_send);
```

Handles a response synchronously. It serializes the response and submits it to the transport. It returns true if the connection should be closed after the response is sent.

**Parameters:**

- **response:** Response being produced. Declaration: `RSty& response`.
- **close:** Callback that closes the connection. Declaration: `bool close`.
- **on_send:** Callback for the named event. Declaration: `const protocol::send_delegate& on_send`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine_sync.h:94](../../../../../include/protocol/http/v11/engine_sync.h#L94)

<a id="method-execute"></a>

## execute

🟠 **METHOD** · *protocol::http::v11::engine_sync*

```cpp
static void execute(
      const ROty& router, const RQty& request, RSty& response, bool& close,
      const router_match_result<RQty, RSty>* selected = nullptr);
```

Executes the request against the router and builds the response. It handles different request targets and sets the appropriate response. If an exception occurs during execution, it builds an error response.

**Parameters:**

- **router:** Router used for request dispatch. Declaration: `const ROty& router`.
- **request:** Request being processed. Declaration: `const RQty& request`.
- **response:** Response being produced. Declaration: `RSty& response`.
- **close:** Callback that closes the connection. Declaration: `bool& close`.
- **selected:** Selected route match, when available. Declaration: `const router_match_result<RQty, RSty>* selected = nullptr`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine_sync.h:106](../../../../../include/protocol/http/v11/engine_sync.h#L106)

<a id="method-authenticate"></a>

## authenticate

🟠 **METHOD** · *protocol::http::v11::engine_sync*

```cpp
static bool authenticate(const router_match_result<RQty, RSty>& match,
                           const RQty& request, RSty& response);
```

Authenticates the supplied request.

**Parameters:**

- **match:** Match result to inspect or update. Declaration: `const router_match_result<RQty, RSty>& match`.
- **request:** Request being processed. Declaration: `const RQty& request`.
- **response:** Response being produced. Declaration: `RSty& response`.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/v11/engine_sync.h:153](../../../../../include/protocol/http/v11/engine_sync.h#L153)

<a id="method-build-error-response"></a>

## build_error_response

🟠 **METHOD** · *protocol::http::v11::engine_sync*

```cpp
static void build_error_response(RSty& response);
```

Builds a generic error response with a 500 Internal Server Error status. It sets the response body to "Internal Server Error". This is used when an exception occurs during request processing.

**Parameters:**

- **response:** Response being produced. Declaration: `RSty& response`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/v11/engine_sync.h:190](../../../../../include/protocol/http/v11/engine_sync.h#L190)

<a id="method-serialize"></a>

## serialize

🟠 **METHOD** · *protocol::http::v11::engine_sync*

```cpp
static serialization_result serialize(RSty& response, bool close);
```

Serializes the response. If the connection should be closed, it sets the "Connection: close" header. It returns the serialized result.

**Parameters:**

- **response:** Response being produced. Declaration: `RSty& response`.
- **close:** Callback that closes the connection. Declaration: `bool close`.

**Returns:** A `serialization_result` describing the serialized output.

**Source:** [include/protocol/http/v11/engine_sync.h:201](../../../../../include/protocol/http/v11/engine_sync.h#L201)

<a id="method-serialize-2"></a>

## serialize

🟠 **METHOD** · *protocol::http::v11::engine_sync*

```cpp
static serialization_result serialize(const RQty& request, RSty& response,
                                        bool& close);
```

Serializes the response based on the request. If the connection should be closed, it sets the "Connection: close" header. It returns the serialized result.

**Parameters:**

- **request:** Request being processed. Declaration: `const RQty& request`.
- **response:** Response being produced. Declaration: `RSty& response`.
- **close:** Callback that closes the connection. Declaration: `bool& close`.

**Returns:** A `serialization_result` describing the serialized output.

**Source:** [include/protocol/http/v11/engine_sync.h:212](../../../../../include/protocol/http/v11/engine_sync.h#L212)
