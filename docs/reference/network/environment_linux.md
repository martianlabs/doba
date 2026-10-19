<a id="environment-linux-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/network/environment_linux/index-dark.svg">
    <img src="../../../resources/docs/reference/network/environment_linux/index.svg" alt="Index for environment_linux.h">
  </picture>
</h1>

**Header:** [include/network/environment_linux.h](../../../include/network/environment_linux.h)

Provides the environment type.

**network::detail**

- [environment](#type-environment) · *type*

**network::detail::environment**

- [environment()](#constructor-environment) · *constructor*
- [environment(environment)](#constructor-environment-2) · *constructor*
- [environment(environment)](#constructor-environment-3) · *constructor*
- [~environment](#destructor-destructor-environment) · *destructor*
- [operator=(environment)](#operator-operator) · *operator*
- [operator=(environment)](#operator-operator-2) · *operator*

<a id="type-environment"></a>

## environment

🟠 **TYPE** · *network::detail*

**Declaration synopsis:**

```cpp
class environment { ... };
```

Provides the environment type.

**Source:** [include/network/environment_linux.h:34](../../../include/network/environment_linux.h#L34)

<a id="constructor-environment"></a>

## environment

🟠 **CONSTRUCTOR** · *network::detail::environment*

```cpp
environment() = default;
```

Constructs a environment value.

**Source:** [include/network/environment_linux.h:39](../../../include/network/environment_linux.h#L39)

<a id="constructor-environment-2"></a>

## environment

🟠 **CONSTRUCTOR** · *network::detail::environment*

```cpp
environment(const environment&) = delete;
```

Disallows this form of network::detail::environment construction.

**Parameters:**

- **environment:** Source object to copy or move. Declaration: `const environment&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/network/environment_linux.h:40](../../../include/network/environment_linux.h#L40)

<a id="constructor-environment-3"></a>

## environment

🟠 **CONSTRUCTOR** · *network::detail::environment*

```cpp
environment(environment&&) noexcept = delete;
```

Disallows this form of network::detail::environment construction.

**Parameters:**

- **environment:** Source object to copy or move. Declaration: `environment&&`.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/network/environment_linux.h:41](../../../include/network/environment_linux.h#L41)

<a id="destructor-destructor-environment"></a>

## ~environment

🟠 **DESTRUCTOR** · *network::detail::environment*

```cpp
~environment() = default;
```

Destroys the environment object.

**Source:** [include/network/environment_linux.h:42](../../../include/network/environment_linux.h#L42)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *network::detail::environment*

```cpp
environment& operator=(const environment&) = delete;
```

Disallows this operator= operation.

**Parameters:**

- **environment:** Source object to copy or move. Declaration: `const environment&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/network/environment_linux.h:46](../../../include/network/environment_linux.h#L46)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *network::detail::environment*

```cpp
environment& operator=(environment&&) noexcept = delete;
```

Disallows this operator= operation.

**Parameters:**

- **environment:** Source object to copy or move. Declaration: `environment&&`.

**Returns:** Unavailable: this operation is deleted.

**Errors:** The operation cannot be called; it is deleted.

**Source:** [include/network/environment_linux.h:47](../../../include/network/environment_linux.h#L47)
