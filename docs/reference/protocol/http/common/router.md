<a id="router-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/router/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router/index.svg" alt="Index for router.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/router.h](../../../../../include/protocol/http/common/router.h)

Provides the http 1.1 router implementation.

**protocol::http**

- [router](#type-router) · *type*

**protocol::http::router**

- [router()](#constructor-router) · *constructor*
- [router(router)](#constructor-router-2) · *constructor*
- [router(router)](#constructor-router-3) · *constructor*
- [~router](#destructor-destructor-router) · *destructor*
- [operator=(router)](#operator-operator) · *operator*
- [operator=(router)](#operator-operator-2) · *operator*
- [add&lt;Hty&gt;(method, route, handler)](#method-add) · *method*
- [add&lt;Hty&gt;(method, route, handler, authenticate)](#method-add-2) · *method*
- [add_controller](#method-add-controller) · *method*
- [match](#method-match) · *method*
- [allowed_methods](#method-allowed-methods) · *method*

<a id="type-router"></a>

## router

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty>
class router { ... };
```

Provides the http 1.1 router implementation.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router.h:58](../../../../../include/protocol/http/common/router.h#L58)

<a id="constructor-router"></a>

## router

🟠 **CONSTRUCTOR** · *protocol::http::router*

```cpp
router() = default;
```

Constructs a router value.

**Source:** [include/protocol/http/common/router.h:63](../../../../../include/protocol/http/common/router.h#L63)

<a id="constructor-router-2"></a>

## router

🟠 **CONSTRUCTOR** · *protocol::http::router*

```cpp
router(const router&) = delete;
```

Disallows this form of protocol::http::router construction.

**Parameters:**

- **router:** Router used for request dispatch. Declaration: `const router&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/common/router.h:64](../../../../../include/protocol/http/common/router.h#L64)

<a id="constructor-router-3"></a>

## router

🟠 **CONSTRUCTOR** · *protocol::http::router*

```cpp
router(router&&) noexcept = delete;
```

Disallows this form of protocol::http::router construction.

**Parameters:**

- **router:** Router used for request dispatch. Declaration: `router&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/common/router.h:65](../../../../../include/protocol/http/common/router.h#L65)

<a id="destructor-destructor-router"></a>

## ~router

🟠 **DESTRUCTOR** · *protocol::http::router*

```cpp
~router() = default;
```

Destroys the router object.

**Source:** [include/protocol/http/common/router.h:66](../../../../../include/protocol/http/common/router.h#L66)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::router*

```cpp
router& operator=(const router&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **router:** Router used for request dispatch. Declaration: `const router&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/common/router.h:70](../../../../../include/protocol/http/common/router.h#L70)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::router*

```cpp
router& operator=(router&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **router:** Router used for request dispatch. Declaration: `router&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/common/router.h:71](../../../../../include/protocol/http/common/router.h#L71)

<a id="method-add"></a>

## add

🟠 **METHOD** · *protocol::http::router*

```cpp
template <typename Hty>
    requires router_handler_lambda<Hty>
  void add(std::string_view method, std::string_view route, Hty handler);
```

Adds a route to the router with the specified HTTP method, route path, and handler.

**Template parameters:**

- **Hty:** `typename Hty`.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **route:** Route pattern to match. Declaration: `std::string_view route`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty handler`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/common/router.h:80](../../../../../include/protocol/http/common/router.h#L80)

<a id="method-add-2"></a>

## add

🟠 **METHOD** · *protocol::http::router*

```cpp
template <typename Hty>
    requires router_handler_lambda<Hty>
  void add(std::string_view method, std::string_view route, Hty handler,
           std::function<auth::authentication_result(const RQty&)>
               authenticate);
```

Adds the requested value.

**Template parameters:**

- **Hty:** `typename Hty`.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **route:** Route pattern to match. Declaration: `std::string_view route`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty handler`.
- **authenticate:** Authentication callback for the route. Declaration: `std::function<auth::authentication_result(const RQty&)> authenticate`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/common/router.h:89](../../../../../include/protocol/http/common/router.h#L89)

<a id="method-add-controller"></a>

## add_controller

🟠 **METHOD** · *protocol::http::router*

```cpp
template <typename CTty, typename... Args>
  void add_controller(Args&&... args);
```

Adds a controller to the router. The controller must implement the 'register_routes' method to register its routes with the router.

**Template parameters:**

- **CTty:** `typename CTty`.
- **Args:** `typename... Args`.

**Parameters:**

- **args:** Arguments forwarded to the selected constructor or handler. Declaration: `Args&&... args`.

**Returns:** No value (`void`).

**Errors:** Directly throws `std::invalid_argument`. Calls made by this operation may propagate additional errors.

**Source:** [include/protocol/http/common/router.h:180](../../../../../include/protocol/http/common/router.h#L180)

<a id="method-match"></a>

## match

🟠 **METHOD** · *protocol::http::router*

```cpp
[[nodiscard]]
  router_match_result<RQty, RSty> match(std::string_view method,
                                        std::string_view path) const;
```

Matches the given HTTP method and path against the registered routes.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** A value of type `router_match_result<RQty, RSty>`.

**Source:** [include/protocol/http/common/router.h:213](../../../../../include/protocol/http/common/router.h#L213)

<a id="method-allowed-methods"></a>

## allowed_methods

🟠 **METHOD** · *protocol::http::router*

```cpp
[[nodiscard]]
  std::string allowed_methods(std::string_view path) const;
```

Returns a comma-separated string of allowed HTTP methods for the given path.

**Parameters:**

- **path:** Filesystem path or route path, according to the call. Declaration: `std::string_view path`.

**Returns:** A value of type `std::string`.

**Source:** [include/protocol/http/common/router.h:269](../../../../../include/protocol/http/common/router.h#L269)
