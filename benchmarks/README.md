# Benchmark runner

This Docker Compose stack runs the Web Frameworks and HttpArena suites without
requiring Ruby, Oha, Bash, Git, or a Docker socket on the host. The `daemon`
service is an isolated Docker daemon. The `runner` service contains every
tool required by the upstream projects and controls that daemon over the
network namespace. This lets Oha reach the private bridge IPs assigned to the
benchmark servers created by the inner daemon.

The two benchmark adapters remain read-only mounts:

- `benchmarks/web-frameworks/http/v11`
- `benchmarks/httparena/http/v11`

## Configure targets

Edit `targets.env` to select the default framework subset for each suite. Use
`docker compose build runner` after editing it so the new defaults are copied
into the image. Use `-e` to override a selection for one invocation without
rebuilding.

```powershell
docker compose run --rm -e WEB_FRAMEWORKS=doba,drogon,oatpp runner web-frameworks
docker compose run --rm -e HTTPARENA_FRAMEWORKS=doba,web-framework-cpp runner httparena
```

`doba` is copied from the mounted adapter into each temporary upstream
checkout. The remaining names must exist in the corresponding upstream
project.

## Run

From `benchmarks/`:

```powershell
docker compose build runner
docker compose run --rm runner web-frameworks
docker compose run --rm runner httparena
docker compose run --rm runner all
```

Each command validates selected targets first. `web-frameworks` then runs its
upstream benchmark workflow. `httparena` runs its upstream benchmark workflow
after validation. To perform validation only:

```powershell
docker compose run --rm runner web-frameworks validate
docker compose run --rm runner httparena validate
docker compose run --rm runner all validate
```

The runner creates its upstream checkouts in the `runner-work` volume. Exported
logs and results are written under `artifacts/`, which is ignored by Git.

## Cleanup

The daemon is isolated from Docker Desktop's image cache and containers. Stop
the stack normally with:

```powershell
docker compose down
```

Remove its cached checkouts and images as well only when they are no longer
needed:

```powershell
docker compose down --volumes
```
