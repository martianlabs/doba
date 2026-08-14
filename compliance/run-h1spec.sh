#!/usr/bin/env bash
set -uo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
suite_dir="$script_dir/h1spec"
repo_root="$(cd -- "$script_dir/.." && pwd)"
build_directory="$repo_root/build/h1spec-linux"
doba_host="${DOBA_HOST:-host.docker.internal}"
doba_port="${DOBA_PORT:-8080}"
timestamp="$(date -u +%Y%m%dT%H%M%SZ)"
output_directory="$script_dir/out/h1spec-$timestamp"
summary_path="$output_directory/summary.txt"
log_path="$output_directory/h1spec.log"
server_pid=""

mkdir -p "$output_directory"
exec > >(tee "$log_path") 2>&1

cleanup() {
  status=$?
  if [[ -n "$server_pid" ]] && kill -0 "$server_pid" 2>/dev/null; then
    kill "$server_pid"
    wait "$server_pid" || true
  fi
  exit "$status"
}

write_summary() {
  printf '%s\n' "$1" | tee -a "$summary_path"
}

trap cleanup EXIT

exit_code=0
cmake -S "$suite_dir" -B "$build_directory" -DCMAKE_BUILD_TYPE=Release || exit_code=$?
if [[ "$exit_code" -eq 0 ]]; then
  cmake --build "$build_directory" || exit_code=$?
fi

if [[ "$exit_code" -eq 0 ]]; then
  "$build_directory/doba_h1spec" &
  server_pid="$!"
  sleep 1
  DOBA_HOST="$doba_host" DOBA_PORT="$doba_port" \
    docker compose -f "$suite_dir/docker-compose.yml" run --rm --build h1spec || exit_code=$?
fi

h1spec_result="$(grep -Eo '[0-9]+ out of [0-9]+ tests passed\.' "$log_path" | tail -n 1 || true)"
write_summary ""
write_summary "h1spec execution summary"
write_summary "Target: $doba_host:$doba_port"
if [[ "$h1spec_result" =~ ^([0-9]+)\ out\ of\ ([0-9]+) ]]; then
  passed="${BASH_REMATCH[1]}"
  total="${BASH_REMATCH[2]}"
  failed=$((total - passed))
  write_summary "  Total: $total | Passed: $passed | Failed: $failed | Warnings: 0"
else
  write_summary "  No complete test result"
fi
write_summary "  Runner exit code: $exit_code"
write_summary "Logs: $log_path"

exit "$exit_code"
