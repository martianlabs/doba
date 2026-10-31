FROM ubuntu:24.04 AS build

ARG DOBA_REF=main

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       ca-certificates \
       cmake \
       g++ \
       make \
    && rm -rf /var/lib/apt/lists/*

ADD "https://github.com/martianlabs/doba.git#${DOBA_REF}" /doba

WORKDIR /source
COPY CMakeLists.txt main.cpp ./

RUN cmake -S . -B /build \
      -DCMAKE_BUILD_TYPE=Release \
      -DDOBA_INCLUDE_DIR=/doba/include \
    && cmake --build /build --config Release --parallel

FROM ubuntu:24.04

COPY --from=build /build/doba_httparena_http_v11 /server

EXPOSE 8080

ENTRYPOINT ["/server"]
