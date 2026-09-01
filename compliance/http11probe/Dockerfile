FROM mcr.microsoft.com/dotnet/sdk@sha256:e1fc6e423f543119c406d24e2e687d67c569f18f04a37a8b0005d80ad0dcee80

ARG HTTP11PROBE_SHA=59513c793ca1f148c82cfedc4b5d75b81d47c591

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
