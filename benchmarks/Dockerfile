FROM docker:27-cli AS docker_cli

FROM ghcr.io/hatoo/oha@sha256:57c2247792c1466c88ecc83ddb9253aa0b58dcb852b1d7f2026a0acf7744c965 AS oha

FROM ruby:3.4-trixie

COPY --from=docker_cli /usr/local/bin/docker /usr/local/bin/docker
COPY --from=oha /bin/oha /usr/local/bin/oha

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       bash \
       build-essential \
       ca-certificates \
       cmake \
       git \
       jq \
       libpq-dev \
       make \
    && rm -rf /var/lib/apt/lists/*

COPY run.sh /usr/local/bin/benchmark-runner
COPY targets.env /usr/local/etc/benchmark-targets.env

RUN sed -i 's/\r$//' /usr/local/etc/benchmark-targets.env \
    && chmod +x /usr/local/bin/benchmark-runner

ENTRYPOINT ["/usr/local/bin/benchmark-runner"]
