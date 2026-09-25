FROM mcr.microsoft.com/dotnet/sdk@sha256:e1fc6e423f543119c406d24e2e687d67c569f18f04a37a8b0005d80ad0dcee80

ARG HTTP11PROBE_SHA=c799bdba77e26f85344e181cf6676c0ac1e5ee7a

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       ca-certificates \
       git \
    && rm -rf /var/lib/apt/lists/* \
    && git clone https://github.com/MDA2AV/Http11Probe.git /probe \
    && git -C /probe checkout --detach "${HTTP11PROBE_SHA}" \
    && test "$(git -C /probe rev-parse HEAD)" = "${HTTP11PROBE_SHA}"

WORKDIR /probe

RUN dotnet restore src/Http11Probe.Cli \
    && dotnet build --configuration Release --no-restore src/Http11Probe.Cli

ENTRYPOINT ["dotnet", "run", "--configuration", "Release", "--no-build", "--project", "src/Http11Probe.Cli", "--"]
CMD ["--host", "host.docker.internal", "--port", "8080", "--output", "/results/results.json"]
