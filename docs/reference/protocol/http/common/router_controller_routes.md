<a id="router-controller-routes-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/router_controller_routes/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router_controller_routes/index.svg" alt="Index for router_controller_routes.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/router_controller_routes.h](../../../../../include/protocol/http/common/router_controller_routes.h)

Provides the router protocol component.

**protocol::http**

- [router](#type-router) · *type*

**protocol::http::detail**

- [router_controller_routes](#type-router-controller-routes) · *type*

**protocol::http::detail::router_controller_routes**

- [router_controller_routes(instance)](#constructor-router-controller-routes) · *constructor*
- [router_controller_routes(router_controller_routes)](#constructor-router-controller-routes-2) · *constructor*
- [operator=](#operator-operator) · *operator*
- [add](#method-add) · *method*
- [empty](#method-empty) · *method*
- [apply](#method-apply) · *method*

<a id="type-router"></a>

## router

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty>
class router { ... };
```

Provides the router protocol component.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/common/router_controller_routes.h:40](../../../../../include/protocol/http/common/router_controller_routes.h#L40)

<a id="type-router-controller-routes"></a>

## router_controller_routes

🟠 **TYPE** · *protocol::http::detail*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty, typename CTty>
class router_controller_routes { ... };
```

Provides the router controller routes protocol component.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.
- **CTty:** `typename CTty`.

**Source:** [include/protocol/http/common/router_controller_routes.h:54](../../../../../include/protocol/http/common/router_controller_routes.h#L54)

<a id="constructor-router-controller-routes"></a>

## router_controller_routes

🟠 **CONSTRUCTOR** · *protocol::http::detail::router_controller_routes*

```cpp
explicit router_controller_routes(std::shared_ptr<CTty> instance);
```

Constructs a router_controller_routes value.

**Parameters:**

- **instance:** Object instance to bind to the member function. Declaration: `std::shared_ptr<CTty> instance`.

**Source:** [include/protocol/http/common/router_controller_routes.h:59](../../../../../include/protocol/http/common/router_controller_routes.h#L59)

<a id="constructor-router-controller-routes-2"></a>

## router_controller_routes

🟠 **CONSTRUCTOR** · *protocol::http::detail::router_controller_routes*

```cpp
router_controller_routes(const router_controller_routes&) = delete;
```

Disallows this form of protocol::http::detail::router_controller_routes construction.

**Parameters:**

- **router_controller_routes:** Source object to copy or move. Declaration: `const router_controller_routes&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/common/router_controller_routes.h:61](../../../../../include/protocol/http/common/router_controller_routes.h#L61)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::detail::router_controller_routes*

```cpp
router_controller_routes& operator=(const router_controller_routes&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **router_controller_routes:** Source object to copy or move. Declaration: `const router_controller_routes&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/protocol/http/common/router_controller_routes.h:65](../../../../../include/protocol/http/common/router_controller_routes.h#L65)

<a id="method-add"></a>

## add

🟠 **METHOD** · *protocol::http::detail::router_controller_routes*

```cpp
template <typename Mty>
    requires std::is_member_function_pointer_v<Mty>
  void add(std::string_view method, std::string_view route, Mty member);
```

Adds the requested value.

**Template parameters:**

- **Mty:** `typename Mty`.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **route:** Route pattern to match. Declaration: `std::string_view route`.
- **member:** Member function to bind. Declaration: `Mty member`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/common/router_controller_routes.h:71](../../../../../include/protocol/http/common/router_controller_routes.h#L71)

<a id="method-empty"></a>

## empty

🟠 **METHOD** · *protocol::http::detail::router_controller_routes*

```cpp
bool empty() const;
```

Reports whether the empty condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_controller_routes.h:82](../../../../../include/protocol/http/common/router_controller_routes.h#L82)

<a id="method-apply"></a>

## apply

🟠 **METHOD** · *protocol::http::detail::router_controller_routes*

```cpp
void apply(router<RQty, RSty>& target);
```

Applies these rules to the supplied state.

**Parameters:**

- **target:** Target router or target form. Declaration: `router<RQty, RSty>& target`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/common/router_controller_routes.h:86](../../../../../include/protocol/http/common/router_controller_routes.h#L86)
