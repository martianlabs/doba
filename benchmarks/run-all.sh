#!/usr/bin/env bash
set -uo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
mode="${BENCHMARK_MODE:-benchmark}"

if [[ "$mode" != "benchmark" && "$mode" != "validate" ]]; then
  echo "BENCHMARK_MODE must be benchmark or validate" >&2
  exit 2
fi

web_frameworks_exit_code=0
http_arena_exit_code=0

printf '\n=== Web Frameworks ===\n'
BENCHMARK_MODE="$mode" bash "$script_dir/run-web-frameworks.sh" ||
  web_frameworks_exit_code=$?

printf '\n=== HttpArena ===\n'
BENCHMARK_MODE="$mode" bash "$script_dir/run-httparena.sh" ||
  http_arena_exit_code=$?

if [[ "$web_frameworks_exit_code" -ne 0 ||
      "$http_arena_exit_code" -ne 0 ]]; then
  exit 1
fi
