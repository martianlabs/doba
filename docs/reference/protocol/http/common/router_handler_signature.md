<a id="router-handler-signature-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/router_handler_signature/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router_handler_signature/index.svg" alt="Index for router_handler_signature.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/router_handler_signature.h](../../../../../include/protocol/http/common/router_handler_signature.h)

Provides the router handler signature protocol component.

**protocol::http**

- [router_handler_signature](#type-router-handler-signature) · *type*
- [router_handler_signature_base](#type-router-handler-signature-base) · *type*

**protocol::http::router_handler_signature_base**

- [return_type](#alias-return-type) · *alias*
- [request_type](#alias-request-type) · *alias*
- [response_type](#alias-response-type) · *alias*
- [parameter_count](#constant-parameter-count) · *constant*
- [bind](#method-bind) · *method*
- [make_parametrized](#method-make-parametrized) · *method*
- [make_parametrized_async](#method-make-parametrized-async) · *method*
- [check](#method-check) · *method*

**protocol::http**

- [router_handler_signature](#type-router-handler-signature-retty-cty-reqty-resty-args-const) · *type*
- [router_handler_signature](#type-router-handler-signature-retty-cty-reqty-resty-args-const-noexcept) · *type*
- [router_handler_signature](#type-router-handler-signature-retty-cty-reqty-resty-args) · *type*
- [router_handler_signature](#type-router-handler-signature-retty-cty-reqty-resty-args-noexcept) · *type*
- [router_handler_lambda](#concept-router-handler-lambda) · *concept*

<a id="type-router-handler-signature"></a>

## router_handler_signature

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename>
struct router_handler_signature { ... };
```

Provides the router handler signature protocol component.

**Template parameters:**

- **typename:** `typename`.

**Source:** [include/protocol/http/common/router_handler_signature.h:46](../../../../../include/protocol/http/common/router_handler_signature.h#L46)

<a id="type-router-handler-signature-base"></a>

## router_handler_signature_base

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename LOty, typename LQty, typename LSty, typename... Args>
struct router_handler_signature_base { ... };
```

Provides the router handler signature base protocol component.

**Template parameters:**

- **LOty:** `typename LOty`.
- **LQty:** `typename LQty`.
- **LSty:** `typename LSty`.
- **Args:** `typename... Args`.

**Source:** [include/protocol/http/common/router_handler_signature.h:65](../../../../../include/protocol/http/common/router_handler_signature.h#L65)

<a id="alias-return-type"></a>

## return_type

🟠 **ALIAS** · *protocol::http::router_handler_signature_base*

```cpp
using return_type = LOty;
```

Alias for `LOty`.

**Source:** [include/protocol/http/common/router_handler_signature.h:69](../../../../../include/protocol/http/common/router_handler_signature.h#L69)

<a id="alias-request-type"></a>

## request_type

🟠 **ALIAS** · *protocol::http::router_handler_signature_base*

```cpp
using request_type = LQty;
```

Alias for `LQty`.

**Source:** [include/protocol/http/common/router_handler_signature.h:70](../../../../../include/protocol/http/common/router_handler_signature.h#L70)

<a id="alias-response-type"></a>

## response_type

🟠 **ALIAS** · *protocol::http::router_handler_signature_base*

```cpp
using response_type = LSty;
```

Alias for `LSty`.

**Source:** [include/protocol/http/common/router_handler_signature.h:71](../../../../../include/protocol/http/common/router_handler_signature.h#L71)

<a id="constant-parameter-count"></a>

## parameter_count

🟠 **CONSTANT** · *protocol::http::router_handler_signature_base*

```cpp
static constexpr std::size_t parameter_count = sizeof...(Args);
```

Defines the parameter count constant.

**Value:** `sizeof...(Args)`

**Source:** [include/protocol/http/common/router_handler_signature.h:75](../../../../../include/protocol/http/common/router_handler_signature.h#L75)

<a id="method-bind"></a>

## bind

🟠 **METHOD** · *protocol::http::router_handler_signature_base*

```cpp
template <typename Cty, typename Mty>
  static auto bind(std::shared_ptr<Cty> instance, Mty method);
```

This static method binds a controller instance and a member function to create a router handler. It returns a lambda that captures the instance and method, and when invoked, it calls the member function on the instance with the provided request and routing arguments. The lambda is noexcept if the member function is noexcept when invoked with the

**Template parameters:**

- **Cty:** `typename Cty`.
- **Mty:** `typename Mty`.

**Parameters:**

- **instance:** Object instance to bind to the member function. Declaration: `std::shared_ptr<Cty> instance`.
- **method:** HTTP method to match. Declaration: `Mty method`.

**Returns:** A callable that captures the instance and invokes the selected member function.

**Source:** [include/protocol/http/common/router_handler_signature.h:90](../../../../../include/protocol/http/common/router_handler_signature.h#L90)

<a id="method-make-parametrized"></a>

## make_parametrized

🟠 **METHOD** · *protocol::http::router_handler_signature_base*

```cpp
template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized(std::string_view pattern, Hty&& handler);
```

This static method creates a parametrized router handler using the provided pattern and handler. It takes a string_view pattern and a handler of type Hty, and returns a router_handler_parametrized object that is constructed with the given pattern and handler. The method uses std::forward to perfectly forward the handler argument to the constructor

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **Hty:** `typename Hty`.

**Parameters:**

- **pattern:** Pattern used for matching. Declaration: `std::string_view pattern`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty&& handler`.

**Returns:** The corresponding parametrized route handler wrapper.

**Source:** [include/protocol/http/common/router_handler_signature.h:113](../../../../../include/protocol/http/common/router_handler_signature.h#L113)

<a id="method-make-parametrized-async"></a>

## make_parametrized_async

🟠 **METHOD** · *protocol::http::router_handler_signature_base*

```cpp
template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized_async(std::string_view pattern, Hty&& handler);
```

Creates parametrized async.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **Hty:** `typename Hty`.

**Parameters:**

- **pattern:** Pattern used for matching. Declaration: `std::string_view pattern`.
- **handler:** Callable invoked for a matching route. Declaration: `Hty&& handler`.

**Returns:** The corresponding parametrized route handler wrapper.

**Source:** [include/protocol/http/common/router_handler_signature.h:118](../../../../../include/protocol/http/common/router_handler_signature.h#L118)

<a id="method-check"></a>

## check

🟠 **METHOD** · *protocol::http::router_handler_signature_base*

```cpp
template <typename RQty, typename RSty>
  static void check();
```

This static method checks if the types of the handler's arguments match the expected types. It takes two template parameters, RQty and RSty, which represent the expected request and response types. It requires void(const RQty&, RSty&, ...).

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/common/router_handler_signature.h:131](../../../../../include/protocol/http/common/router_handler_signature.h#L131)

<a id="type-router-handler-signature-retty-cty-reqty-resty-args-const"></a>

## router_handler_signature

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...) const>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> { ... };
```

Provides the router handler signature<retty (cty::*)(reqty, resty, args...) const> protocol component.

**Template parameters:**

- **Cty:** `typename Cty`.
- **Retty:** `typename Retty`.
- **Reqty:** `typename Reqty`.
- **Resty:** `typename Resty`.
- **Args:** `typename... Args`.

**Source:** [include/protocol/http/common/router_handler_signature.h:166](../../../../../include/protocol/http/common/router_handler_signature.h#L166)

<a id="type-router-handler-signature-retty-cty-reqty-resty-args-const-noexcept"></a>

## router_handler_signature

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...)
                                    const noexcept>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> { ... };
```

Provides the router handler signature<retty (cty::*)(reqty, resty, args...)
                                    const noexcept> protocol component.

**Template parameters:**

- **Cty:** `typename Cty`.
- **Retty:** `typename Retty`.
- **Reqty:** `typename Reqty`.
- **Resty:** `typename Resty`.
- **Args:** `typename... Args`.

**Source:** [include/protocol/http/common/router_handler_signature.h:170](../../../../../include/protocol/http/common/router_handler_signature.h#L170)

<a id="type-router-handler-signature-retty-cty-reqty-resty-args"></a>

## router_handler_signature

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...)>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> { ... };
```

Provides the router handler signature<retty (cty::*)(reqty, resty, args...)> protocol component.

**Template parameters:**

- **Cty:** `typename Cty`.
- **Retty:** `typename Retty`.
- **Reqty:** `typename Reqty`.
- **Resty:** `typename Resty`.
- **Args:** `typename... Args`.

**Source:** [include/protocol/http/common/router_handler_signature.h:175](../../../../../include/protocol/http/common/router_handler_signature.h#L175)

<a id="type-router-handler-signature-retty-cty-reqty-resty-args-noexcept"></a>

## router_handler_signature

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...) noexcept>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> { ... };
```

Provides the router handler signature<retty (cty::*)(reqty, resty, args...) noexcept> protocol component.

**Template parameters:**

- **Cty:** `typename Cty`.
- **Retty:** `typename Retty`.
- **Reqty:** `typename Reqty`.
- **Resty:** `typename Resty`.
- **Args:** `typename... Args`.

**Source:** [include/protocol/http/common/router_handler_signature.h:179](../../../../../include/protocol/http/common/router_handler_signature.h#L179)

<a id="concept-router-handler-lambda"></a>

## router_handler_lambda

🟠 **CONCEPT** · *protocol::http*

```cpp
template <typename Hty>
concept router_handler_lambda = requires {
  typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type;
  requires std::is_lvalue_reference_v<typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type>;
  requires std::is_const_v<
      std::remove_reference_t<typename router_handler_signature<
          decltype(&std::decay_t<Hty>::operator())>::request_type>>;
  requires std::same_as<
               typename router_handler_signature<
                   decltype(&std::decay_t<Hty>::operator())>::return_type,
               void> ||
               std::same_as<
                   typename router_handler_signature<
                       decltype(&std::decay_t<Hty>::operator())>::return_type,
                   common::task<void>>;
  requires std::is_lvalue_reference_v<typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::response_type>;
  requires !std::is_const_v<
      std::remove_reference_t<typename router_handler_signature<
          decltype(&std::decay_t<Hty>::operator())>::response_type>>;
};
```

Checks the compile-time requirements for router handler lambda.

**Template parameters:**

- **Hty:** `typename Hty`.

**Source:** [include/protocol/http/common/router_handler_signature.h:196](../../../../../include/protocol/http/common/router_handler_signature.h#L196)
