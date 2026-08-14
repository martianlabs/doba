#!/usr/bin/env bash
set -uo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
suite_dir="$script_dir/http11probe"
repo_root="$(cd -- "$script_dir/.." && pwd)"
build_directory="$repo_root/build/http11probe-linux"
doba_host="${DOBA_HOST:-host.docker.internal}"
doba_port="${DOBA_PORT:-8080}"
timestamp="$(date -u +%Y%m%dT%H%M%SZ)"
output_directory="$script_dir/out/http11probe-$timestamp"
summary_path="$output_directory/summary.txt"
log_path="$output_directory/http11probe.log"
report_path="$suite_dir/out/http11probe/results.json"
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

summary_value() {
  grep -m 1 -E "^[[:space:]]*\"$1\"[[:space:]]*:" "$report_path" |
    sed -E 's/.*:[[:space:]]*([0-9.]+).*/\1/'
}

verdict_count() {
  awk -v verdict="$1" '
    /^[[:space:]]*"verdict":/ {
      line = $0
      sub(/^[^:]*:[[:space:]]*"/, "", line)
      sub(/"[[:space:]]*,?[[:space:]]*$/, "", line)
      if (line == verdict) count++
    }
    END { print count + 0 }
  ' "$report_path"
}

scored_verdict_count() {
  awk -v wanted_scored="$1" -v wanted_verdict="$2" '
    /^[[:space:]]*"scored":/ {
      scored = $0
      sub(/^[^:]*:[[:space:]]*/, "", scored)
      sub(/,[[:space:]]*$/, "", scored)
    }
    /^[[:space:]]*"verdict":/ {
      verdict = $0
      sub(/^[^:]*:[[:space:]]*"/, "", verdict)
      sub(/"[[:space:]]*,?[[:space:]]*$/, "", verdict)
      if (scored == wanted_scored && verdict == wanted_verdict) count++
    }
    END { print count + 0 }
  ' "$report_path"
}

trap cleanup EXIT

exit_code=0
cmake -S "$suite_dir" -B "$build_directory" -DCMAKE_BUILD_TYPE=Release || exit_code=$?
if [[ "$exit_code" -eq 0 ]]; then
  cmake --build "$build_directory" || exit_code=$?
fi

if [[ "$exit_code" -eq 0 ]]; then
  "$build_directory/doba_http11probe" &
  server_pid="$!"
  sleep 1
  DOBA_HOST="$doba_host" DOBA_PORT="$doba_port" \
    docker compose -f "$suite_dir/docker-compose.yml" run --rm --build http11probe || exit_code=$?
fi

write_summary ""
write_summary "Http11Probe execution summary"
write_summary "Target: $doba_host:$doba_port"
if [[ "$exit_code" -eq 0 && -f "$report_path" ]]; then
  total="$(summary_value total)"
  passed="$(verdict_count Pass)"
  failed="$(verdict_count Fail)"
  warnings="$(verdict_count Warn)"
  errors="$(summary_value errors)"
  scored_passed="$(scored_verdict_count true Pass)"
  scored_failed="$(scored_verdict_count true Fail)"
  scored_warnings="$(scored_verdict_count true Warn)"
  unscored_passed="$(scored_verdict_count false Pass)"
  unscored_failed="$(scored_verdict_count false Fail)"
  unscored_warnings="$(scored_verdict_count false Warn)"
  write_summary "  Total: $total | Passed: $passed | Failed: $failed | Warnings: $warnings | Errors: $errors"
  write_summary "  Scored: Passed: $scored_passed | Failed: $scored_failed | Warnings: $scored_warnings"
  write_summary "  Unscored: Passed: $unscored_passed | Failed: $unscored_failed | Warnings: $unscored_warnings"
else
  write_summary "  No complete test result"
fi
write_summary "  Runner exit code: $exit_code"
write_summary "Logs: $log_path"
write_summary "Http11Probe report: $report_path"

exit "$exit_code"
