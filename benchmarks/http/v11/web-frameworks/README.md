<a name="web-frameworks-http11-benchmark-adapter"></a>
<h1>
  <a href="../../../../resources/benchmarks/http/v11/web-frameworks/h1-web-frameworks-http11-benchmark-adapter.svg#gh-light-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/web-frameworks/h1-web-frameworks-http11-benchmark-adapter.svg" alt="Web Frameworks">
  </a>
  <a href="../../../../resources/benchmarks/http/v11/web-frameworks/h1-web-frameworks-http11-benchmark-adapter-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/web-frameworks/h1-web-frameworks-http11-benchmark-adapter-dark.svg" alt="Web Frameworks">
  </a>
</h1>

Three routes, plenty of traffic. doba serves them on port 3000; the upstream
runner checks replies, generates load and collects the numbers.

| Route | Reply |
| --- | --- |
| `GET /` | Empty body. |
| `GET /user/:id` | The path parameter. |
| `POST /user` | Empty body. |

The route checks cover all three. The default benchmark times only `GET /`.
The runner setup below matches [upstream revision 50cb022](https://github.com/the-benchmarker/web-frameworks/tree/50cb0223f095878106e3115ed06261056870032b).

<a name="run-it-here"></a>
<h2>
  <a href="../../../../resources/benchmarks/http/v11/web-frameworks/h2-run-it-here.svg#gh-light-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/web-frameworks/h2-run-it-here.svg" alt="Run it here">
  </a>
  <a href="../../../../resources/benchmarks/http/v11/web-frameworks/h2-run-it-here-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/web-frameworks/h2-run-it-here-dark.svg" alt="Run it here">
  </a>
</h2>

With C++20, CMake 3.20+ and Ninja ready, from the repo root in a Unix shell:

```sh
cmake -S benchmarks/http/v11/web-frameworks -B out/web-frameworks -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out/web-frameworks --parallel
./out/web-frameworks/doba_web_frameworks_http_v11
```

<a name="run-it-in-docker"></a>
<h2>
  <a href="../../../../resources/benchmarks/http/v11/web-frameworks/h2-run-it-in-docker.svg#gh-light-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/web-frameworks/h2-run-it-in-docker.svg" alt="Run it in Docker">
  </a>
  <a href="../../../../resources/benchmarks/http/v11/web-frameworks/h2-run-it-in-docker-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/web-frameworks/h2-run-it-in-docker-dark.svg" alt="Run it in Docker">
  </a>
</h2>

```sh
docker build --tag doba-web-frameworks-http-v11 benchmarks/http/v11/web-frameworks
docker run --rm --publish 3000:3000 doba-web-frameworks-http-v11
```

The Dockerfile defaults to the published `7be948e` doba commit used by the
upstream configuration. To try another published revision, add
`--build-arg DOBA_REF=<tag-or-commit>` to `docker build`.
Local library edits need the local build; Docker fetches its copy remotely.

<a name="use-the-upstream-runner"></a>
<h2>
  <a href="../../../../resources/benchmarks/http/v11/web-frameworks/h2-use-the-upstream-runner.svg#gh-light-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/web-frameworks/h2-use-the-upstream-runner.svg" alt="Use the upstream runner">
  </a>
  <a href="../../../../resources/benchmarks/http/v11/web-frameworks/h2-use-the-upstream-runner-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/web-frameworks/h2-use-the-upstream-runner-dark.svg" alt="Use the upstream runner">
  </a>
</h2>

Copy `CMakeLists.txt`, `config.yaml` and `main.cpp` from this folder to
`cpp/doba` in the pinned upstream checkout. Have Ruby, Bundler, Docker,
`jq` and `zrk` 2.4+ ready, then generate the manifests:

```sh
bundle install
CONCURRENCIES=64,256,512 ROUTES='GET:/,GET:/user/0,POST:/user' \
  SERVER_CPUS=0-3 LOAD_CPUS=4-15 bundle exec rake config
```

Run the generated targets:

```sh
make -f cpp/doba/.Makefile build
make -f cpp/doba/.Makefile test
make -f cpp/doba/.Makefile warmup
mkdir -p cpp/doba/.results/{64,256,512}
make -f cpp/doba/.Makefile collect
make -f cpp/doba/.Makefile unbuild
```

Use the same targets and conditions for `cpp/drogon` and `cpp/oatpp`.
By default, collect uses `zrk --closed` for 15 seconds per route and records
request rate, latency and CPU saturation. Keep the upstream checkout,
generated manifests and results outside this repo.
