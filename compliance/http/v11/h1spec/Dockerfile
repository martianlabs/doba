FROM denoland/deno@sha256:b49ac52f05c3d8d0da890b6628168e9bfb5721f7bccc00305bb3ad29ed0e40af

ARG H1SPEC_SHA=f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e

USER root

RUN apk add --no-cache git \
    && git clone https://github.com/uNetworking/h1spec.git /h1spec \
    && git -C /h1spec checkout --detach "${H1SPEC_SHA}" \
    && test "$(git -C /h1spec rev-parse HEAD)" = "${H1SPEC_SHA}"

WORKDIR /h1spec

ENTRYPOINT ["deno", "run", "--allow-net", "http_test.ts"]
CMD ["host.docker.internal", "8080"]
