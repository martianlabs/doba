<a name="package-tests"></a>
<h1>
  <a href="../../resources/tests/package/h1-package-tests.svg#gh-light-mode-only">
    <img src="../../resources/tests/package/h1-package-tests.svg" alt="Package tests">
  </a>
  <a href="../../resources/tests/package/h1-package-tests-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/package/h1-package-tests-dark.svg" alt="Package tests">
  </a>
</h1>

Working inside doba's own project is the easy part. These checks put a
separate consumer on the other side of the CMake targets and check what it
actually receives.

<a name="a-project-of-its-own"></a>
<h2>
  <a href="../../resources/tests/package/h2-a-project-of-its-own.svg#gh-light-mode-only">
    <img src="../../resources/tests/package/h2-a-project-of-its-own.svg" alt="A project of its own">
  </a>
  <a href="../../resources/tests/package/h2-a-project-of-its-own-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/package/h2-a-project-of-its-own-dark.svg" alt="A project of its own">
  </a>
</h2>

The [consumer project](CMakeLists.txt) checks both the installed package and
source inclusion through **add_subdirectory**. Its targets take their public
headers, C++20 requirement, and link dependencies from **martianlabs::doba**
and, for TLS, **martianlabs::doba_tls**.

The source-inclusion case also rejects accidental example or test targets
appearing in the parent project. Depending on a library shouldn't quietly
adopt its homework.

<a name="public-types-real-dependencies"></a>
<h2>
  <a href="../../resources/tests/package/h2-public-types-real-dependencies.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/package/h2-public-types-real-dependencies-narrow.svg">
      <img src="../../resources/tests/package/h2-public-types-real-dependencies.svg" alt="Public types, real dependencies">
    </picture>
  </a>
  <a href="../../resources/tests/package/h2-public-types-real-dependencies-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/package/h2-public-types-real-dependencies-narrow-dark.svg">
      <img src="../../resources/tests/package/h2-public-types-real-dependencies-dark.svg" alt="Public types, real dependencies">
    </picture>
  </a>
</h2>

The [HTTP consumer](protocol/http/v11/server_tests.cpp) exercises filesystem
and reader types, an HTTP server, a controller with a typed route parameter,
and the static-file controller.

The [TLS consumer](transport/server/tls_tests.cpp) checks the OpenSSL link
dependency by creating and freeing a context. It constructs an HTTPS server
with fixture credentials and a route, and another with a custom engine.

These are consumption and construction checks. The consumer programs do
not open listening sockets or exchange requests; those behaviors belong to
the [integration suite](../integration/README.md).
