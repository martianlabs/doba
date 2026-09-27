# HTTPS hello world

This example serves `GET /hello` over TLS on port 8443. It accepts a PEM
certificate chain and private key as command-line arguments.

## Build

Enable TLS and build the `https_hello_world` target:

```sh
cmake -S . -B out/build/tls -DDOBA_ENABLE_TLS=ON
cmake --build out/build/tls --target https_hello_world
```

OpenSSL development headers and libraries must be available to CMake.
On Windows with vcpkg, add
`-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake` to the
configure command.

## Run

From the repository root, run the built executable with your certificate and
private key:

```text
https_hello_world server.crt server.key
```

Then request the route:

```sh
curl -i https://localhost:8443/hello
```

The response is `200 OK` with body `hello from doba`. For a self-signed
certificate used only in local testing, add `--insecure` to the `curl` command.
