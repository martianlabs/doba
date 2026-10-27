<a name="getting-started"></a>
<h1>
  <a href="../../resources/docs/guide/h1-getting-started.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h1-getting-started.svg" alt="Getting started">
  </a>
  <a href="../../resources/docs/guide/h1-getting-started-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h1-getting-started-dark.svg" alt="Getting started">
  </a>
</h1>

Let's build your first doba server. You'll create a tiny project, give it one
route, and make it say hello to **curl**. The whole app is two files.

Want a working example to peek at along the way? The bundled
[hello_world example](../../examples/http/v11/hello_world/README.md) has its
own [source](../../examples/http/v11/hello_world/main.cpp). It uses the same
HTTP server you'll use here.

<a name="before-you-start"></a>
<h2>
  <a href="../../resources/docs/guide/h2-before-you-start.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-before-you-start.svg" alt="Before you start">
  </a>
  <a href="../../resources/docs/guide/h2-before-you-start-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-before-you-start-dark.svg" alt="Before you start">
  </a>
</h2>

You'll need **Git**, **CMake 3.20 or later**, a **C++20 compiler**, and
**curl**. On Windows, run the commands in a terminal with your C++ compiler
available; on Linux, use your usual development shell. This first server uses
plain TCP, so you don't need OpenSSL.

Make a folder for your app and clone doba inside it. Run these commands from
wherever you keep your projects:

```sh
mkdir hello-doba
cd hello-doba
git clone https://github.com/martianlabs/doba.git doba
```

Stay in **hello-doba** for the rest of the chapter. You'll add **main.cpp**
and **CMakeLists.txt** beside the **doba** folder.

<a name="build-it"></a>
<h2>
  <a href="../../resources/docs/guide/h2-build-it.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-build-it.svg" alt="Build it">
  </a>
  <a href="../../resources/docs/guide/h2-build-it-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-build-it-dark.svg" alt="Build it">
  </a>
</h2>

Create **CMakeLists.txt** in **hello-doba**:

```cmake
cmake_minimum_required(VERSION 3.20)
project(hello_doba LANGUAGES CXX)

add_subdirectory(doba)

add_executable(hello_world main.cpp)
target_link_libraries(hello_world PRIVATE martianlabs::doba)
```

Now create **main.cpp** in the same folder:

```cpp
#include "common/signaler.h"
#include "http_server.h"

using martianlabs::doba::common::signaler;
using martianlabs::doba::http_server;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;

int main() {
  http_server<> server({.ip = "127.0.0.1", .port = "8080"});

  server.add_route("GET", "/hello", [](const request&, response& res) {
    res.ok_200()
        .add_header("Content-Type", "text/plain; charset=utf-8")
        .set_body("Hello, world!");
  });

  server.start();
  signaler::wait();
}
```

On Linux with a single-configuration generator, configure and build your app:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target hello_world
```

On Windows with Visual Studio, select **Release** when you build:

```powershell
cmake -S . -B build
cmake --build build --config Release --target hello_world
```

CMake builds your app; you don't need to build doba's examples or tests.

<a name="run-it"></a>
<h2>
  <a href="../../resources/docs/guide/h2-run-it.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-run-it.svg" alt="Run it">
  </a>
  <a href="../../resources/docs/guide/h2-run-it-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-run-it-dark.svg" alt="Run it">
  </a>
</h2>

On Linux with a single-configuration generator, start the server with:

```sh
./build/hello_world
```

On Windows with Visual Studio, use:

```powershell
.\build\Release\hello_world.exe
```

Leave it running. In a second terminal, from either platform, send a request:

```sh
curl -i http://127.0.0.1:8080/hello
```

You should see **200 OK**, a **Content-Type: text/plain; charset=utf-8**
header, and **Hello, world!** in the response body. Nice: your server is
talking. Stop it with Ctrl+C.

If port **8080** is already taken, change it in **main.cpp**, rebuild, and use
the same new port in the **curl** URL.

<a name="under-the-hood"></a>
<h2>
  <a href="../../resources/docs/guide/h2-under-the-hood.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-under-the-hood.svg" alt="Under the hood">
  </a>
  <a href="../../resources/docs/guide/h2-under-the-hood-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-under-the-hood-dark.svg" alt="Under the hood">
  </a>
</h2>

In **CMakeLists.txt**, *add_subdirectory* brings in your local doba checkout.
Linking *martianlabs::doba* gives the app doba's headers, its C++20 setting,
and the system threading dependency.

In **main.cpp**, *http_server<>* pairs HTTP/1.1 with doba's default TCP
transport. *add_route* handles **GET /hello** and builds the response.
*start()* opens the listener, while *signaler::wait()* keeps the process alive
until you stop it.

That's your first doba app. The [core model](02-core-model.md) shows how the
protocol and transport fit together; the
[guide index](README.md) has the rest of the tour.
