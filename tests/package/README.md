<a name="package-tests"></a>
<h1>
  <a href="../../resources/tests/package/h1-package-tests.svg#gh-light-mode-only">
    <img src="../../resources/tests/package/h1-package-tests.svg" alt="Package tests">
  </a>
  <a href="../../resources/tests/package/h1-package-tests-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/package/h1-package-tests-dark.svg" alt="Package tests">
  </a>
</h1>

It works in doba's own build. Does it work for someone else's project?

<a name="a-project-of-its-own"></a>
<h2>
  <a href="../../resources/tests/package/h2-a-project-of-its-own.svg#gh-light-mode-only">
    <img src="../../resources/tests/package/h2-a-project-of-its-own.svg" alt="A project of its own">
  </a>
  <a href="../../resources/tests/package/h2-a-project-of-its-own-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/package/h2-a-project-of-its-own-dark.svg" alt="A project of its own">
  </a>
</h2>

The [consumer project](CMakeLists.txt) tries both an installed package and
`add_subdirectory`. It gets headers, C++20 and link dependencies from
`martianlabs::doba`; TLS uses `martianlabs::doba_tls`.

Adding doba as a subdirectory must not bring its examples and tests into the
parent project. Nobody ordered those.

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

The [consumer](protocol/http/v11/server_tests.cpp) constructs an HTTP server,
a typed controller route, readers and the static-file controller.
`DOBA_PACKAGE_USE_TLS=ON` also requests the installed `TLS` component and
builds `doba_tls_consumer`.

A plain consumer must work without finding OpenSSL, even when doba was installed
with TLS enabled. CI checks that with OpenSSL discovery switched off.

These programs check that consuming the package works. Real requests and
listening sockets belong to the [integration tests](../integration/README.md).
