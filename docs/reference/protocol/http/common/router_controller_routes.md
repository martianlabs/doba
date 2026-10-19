<a id="router-controller-routes-h"></a>
<h1>
  <a href="../../../../../resources/docs/reference/protocol/http/common/router_controller_routes/index.svg#gh-light-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router_controller_routes/index.svg" alt="Index for router_controller_routes.h">
  </a>
  <a href="../../../../../resources/docs/reference/protocol/http/common/router_controller_routes/index-dark.svg#gh-dark-mode-only">
    <img src="../../../../../resources/docs/reference/protocol/http/common/router_controller_routes/index-dark.svg" alt="Index for router_controller_routes.h">
  </a>
</h1>

**Header:** [include/protocol/http/common/router_controller_routes.h](../../../../../include/protocol/http/common/router_controller_routes.h)

Provides the router protocol component.

**protocol::http**

- [router](#type-router) · *type*

**protocol::http::detail**

- [router_controller_routes](#type-router-controller-routes) · *type*

**protocol::http::detail::router_controller_routes**

- [add](#method-add) · *method*
- [empty](#method-empty) · *method*
- [apply](#method-apply) · *method*

<a id="type-router"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> router

**TYPE** · *protocol::http*

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

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> router_controller_routes

**TYPE** · *protocol::http::detail*

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

<a id="method-add"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> add

**METHOD** · *protocol::http::detail::router_controller_routes*

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

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> empty

**METHOD** · *protocol::http::detail::router_controller_routes*

```cpp
bool empty() const;
```

Reports whether the empty condition holds.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/common/router_controller_routes.h:82](../../../../../include/protocol/http/common/router_controller_routes.h#L82)

<a id="method-apply"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> apply

**METHOD** · *protocol::http::detail::router_controller_routes*

```cpp
void apply(router<RQty, RSty>& target);
```

Applies these rules to the supplied state.

**Parameters:**

- **target:** Target router or target form. Declaration: `router<RQty, RSty>& target`.

**Returns:** No value (`void`).

**Source:** [include/protocol/http/common/router_controller_routes.h:86](../../../../../include/protocol/http/common/router_controller_routes.h#L86)
