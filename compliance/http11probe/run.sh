#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"
build_directory="$repo_root/build/http11probe"
doba_host="${DOBA_HOST:-host.docker.internal}"
doba_port="${DOBA_PORT:-8080}"
server_pid=""

cleanup() {
  status=$?
  if [[ -n "$server_pid" ]] && kill -0 "$server_pid" 2>/dev/null; then
    kill "$server_pid"
    wait "$server_pid" || true
  fi
  exit "$status"
}

trap cleanup EXIT

cmake -S "$script_dir" -B "$build_directory" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_directory"

"$build_directory/doba_http11probe" &
server_pid="$!"
sleep 1

DOBA_HOST="$doba_host" DOBA_PORT="$doba_port" \
  docker compose -f "$script_dir/docker-compose.yml" run --rm --build http11probe
