# Development

[Index](HANDOFF.md)

## Contents

- [Requirements and layout](#requirements-and-layout)
- [Build and tests](#build-and-tests)
- [CMake consumption](#cmake-consumption)
- [Repository style](#repository-style)
- [Change validation](#change-validation)

## Requirements and layout

- C++20; backends are available for Windows and Linux.
- CMake 3.20 or later for manual configuration.
- CMake 3.25 or later for schema 6 MSVC presets.
- Ninja and an initialized MSVC development environment for Windows presets.

The library resides in `include/`. Component tests mirror the header path
under `tests/unit`, `tests/integration`, or `tests/package`.
Shared helpers and `CMakeLists.txt` files remain at each suite's root.
See the [architectural principles](ARCHITECTURE.md) and
[examples](../examples/README.md).

## Build and tests

Run commands from the project root. With a single-configuration generator and
an available C++20 compiler:

```sh
cmake -S . -B out/build/debug -DCMAKE_BUILD_TYPE=Debug -DDOBA_ENABLE_STRICT_WARNINGS=ON
cmake --build out/build/debug
ctest --test-dir out/build/debug --output-on-failure
```

For Release, use a separate directory and `-DCMAKE_BUILD_TYPE=Release`.
On Windows, run the following in an MSVC development environment:

```sh
cmake --preset msvc-debug -DDOBA_ENABLE_STRICT_WARNINGS=ON
cmake --build --preset build-debug
ctest --test-dir out/build/msvc-debug --output-on-failure

cmake --preset msvc-release -DDOBA_ENABLE_STRICT_WARNINGS=ON
cmake --build --preset build-release
ctest --test-dir out/build/msvc-release --output-on-failure
```

[CMakeLists.txt](../CMakeLists.txt) provides these options:

| Option | Default | Effect |
| --- | --- | --- |
| `DOBA_BUILD_EXAMPLES` | ON as the main project; OFF as a subproject | Builds examples. |
| `DOBA_BUILD_TESTS` | ON as the main project; OFF as a subproject | Enables the project's test suites. |
| `DOBA_ENABLE_STRICT_WARNINGS` | OFF | Enables strict warnings in the doba build tree. |

The [CI gates](QUALITY.md#enforce-the-ci-gates) define the compiler and
sanitizer validations. Exact commands are maintained in the
[CI workflow](../.github/workflows/ci.yml).

## CMake consumption

Installation and consumers are documented in the [README](../README.md).
Locate the package with `find_package(doba CONFIG REQUIRED)` and link
against `martianlabs::doba`.

The target exports includes, C++20, and Threads. Internal warning and
sanitizer flags are not part of its installed interface. When using
`add_subdirectory`, doba's tests and examples are disabled by default,
even when the parent project enables its own testing.

## Repository style

All repository documentation must be written in English.

Equivalent files in the same module define the code style. Preserve naming,
declaration order, includes, namespaces, signatures, indentation, and the
structure of classes and tests.

- UTF-8 without BOM, ASCII content, and CRLF endings in the working tree.
- A final newline is required; no trailing whitespace.
- C++ lines must be at most 80 columns.
- Every new C++ file uses the Apache/doba header from an equivalent file.
- Local changes preserve existing encoding, formatting, and file headers.
- Comments are concise and explain necessary constraints.
- RFC references use `S`, for example `RFC 9110 S7.6.1`.
- Preserve width and closing alignment in boxed comments.

CMake passes `/utf-8` to MSVC to fix source and execution character sets.
Check these conventions in tests and documentation as well, regardless of
local editor configuration.

## Change validation

1. Inspect the component, equivalent files, and its tests.
2. Identify the cause and applicable contract; cite the RFC section when it
   determines HTTP behavior.
3. Keep the change localized, preserving the API, ownership, and
   protocol-transport boundary.
4. For a bug, add a regression that fails before the fix and passes after it,
   including relevant boundary cases.
5. Run focused tests and the affected suite. Use the CI matrix to validate
   the relevant platforms and configurations.
6. Review the diff, documentation links, and formatting. Run
   `git diff --check`; check CRLF with
   `git ls-files --eol -- <file>`, and verify ASCII, absence of BOM, and a
   final newline.
7. Record which tests ran, their results, and any limitations.

Protocol changes preserve the distinction between syntax and semantics.
Performance changes require reproducible measurements and must not weaken
correctness. Documentation changes are validated through content, references,
and formatting; they do not require running CTest on their own.
