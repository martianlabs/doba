#!/usr/bin/env bash
set -uo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
compose_path="$script_dir/compose.yaml"
mode="${BENCHMARK_MODE:-benchmark}"
frameworks="${HTTPARENA_FRAMEWORKS:-}"

if [[ "$mode" != "benchmark" && "$mode" != "validate" ]]; then
  echo "BENCHMARK_MODE must be benchmark or validate" >&2
  exit 2
fi

arguments=(compose -f "$compose_path" run --rm --build)
if [[ -n "$frameworks" ]]; then
  arguments+=(-e "HTTPARENA_FRAMEWORKS=$frameworks")
fi
arguments+=(runner httparena "$mode")

cleanup() {
  status=$?
  trap - EXIT
  docker compose -f "$compose_path" down
  down_status=$?
  if [[ "$status" -eq 0 && "$down_status" -ne 0 ]]; then
    status="$down_status"
  fi
  exit "$status"
}

trap cleanup EXIT
docker "${arguments[@]}"
