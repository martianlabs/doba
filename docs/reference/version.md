<a id="version"></a>
<a href="../../resources/docs/reference/version/index.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/version/index.svg" alt="version">
</a>
<a href="../../resources/docs/reference/version/index-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/version/index-dark.svg" alt="version">
</a>

**Header:** [include/version.h](../../include/version.h)

Defines the preprocessor values listed below.

### <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> doba

[DOBA_VERSION_MAJOR](#macro-doba-version-major) ·
[DOBA_VERSION_MINOR](#macro-doba-version-minor) ·
[DOBA_VERSION_PATCH](#macro-doba-version-patch) ·
[DOBA_VERSION_STRINGIFY_IMPL](#macro-doba-version-stringify-impl) ·
[DOBA_VERSION_STRINGIFY](#macro-doba-version-stringify) ·
[DOBA_VERSION](#macro-doba-version)

<a id="api-details"></a>
<a href="../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="macro-doba-version-major"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> DOBA_VERSION_MAJOR

**MACRO** · *doba*

```cpp
#define DOBA_VERSION_MAJOR 0
```

Defines the doba version major preprocessor value.

**Value:** `0`

**Source:** [include/version.h:28](../../include/version.h#L28)

<a id="macro-doba-version-minor"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> DOBA_VERSION_MINOR

**MACRO** · *doba*

```cpp
#define DOBA_VERSION_MINOR 1
```

Defines the doba version minor preprocessor value.

**Value:** `1`

**Source:** [include/version.h:29](../../include/version.h#L29)

<a id="macro-doba-version-patch"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> DOBA_VERSION_PATCH

**MACRO** · *doba*

```cpp
#define DOBA_VERSION_PATCH 0
```

Defines the doba version patch preprocessor value.

**Value:** `0`

**Source:** [include/version.h:30](../../include/version.h#L30)

<a id="macro-doba-version-stringify-impl"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> DOBA_VERSION_STRINGIFY_IMPL

**MACRO** · *doba*

```cpp
#define DOBA_VERSION_STRINGIFY_IMPL(value) #value
```

Defines the doba version stringify impl preprocessor value.

**Value:** `#value`

**Source:** [include/version.h:31](../../include/version.h#L31)

<a id="macro-doba-version-stringify"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> DOBA_VERSION_STRINGIFY

**MACRO** · *doba*

```cpp
#define DOBA_VERSION_STRINGIFY(value) DOBA_VERSION_STRINGIFY_IMPL(value)
```

Defines the doba version stringify preprocessor value.

**Value:** `DOBA_VERSION_STRINGIFY_IMPL(value)`

**Source:** [include/version.h:32](../../include/version.h#L32)

<a id="macro-doba-version"></a>

## <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> DOBA_VERSION

**MACRO** · *doba*

```cpp
#define DOBA_VERSION \
  DOBA_VERSION_STRINGIFY(DOBA_VERSION_MAJOR) "." \
  DOBA_VERSION_STRINGIFY(DOBA_VERSION_MINOR) "." \
  DOBA_VERSION_STRINGIFY(DOBA_VERSION_PATCH)
```

Defines the doba version preprocessor value.

**Value:** `\
  DOBA_VERSION_STRINGIFY(DOBA_VERSION_MAJOR) "." \
  DOBA_VERSION_STRINGIFY(DOBA_VERSION_MINOR) "." \
  DOBA_VERSION_STRINGIFY(DOBA_VERSION_PATCH)`

**Source:** [include/version.h:33](../../include/version.h#L33)
