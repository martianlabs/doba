#!/usr/bin/env bash
set -euo pipefail

source /usr/local/etc/benchmark-targets.env

WEB_FRAMEWORKS="${WEB_FRAMEWORKS:-$DEFAULT_WEB_FRAMEWORKS}"
HTTPARENA_FRAMEWORKS="${HTTPARENA_FRAMEWORKS:-$DEFAULT_HTTPARENA_FRAMEWORKS}"
WEB_FRAMEWORKS_REF="${WEB_FRAMEWORKS_REF:-$DEFAULT_WEB_FRAMEWORKS_REF}"
HTTPARENA_REF="${HTTPARENA_REF:-$DEFAULT_HTTPARENA_REF}"

suite="${1:-}"
mode="${2:-benchmark}"
timestamp="$(date -u +%Y%m%dT%H%M%SZ)"

if [ "$suite" != "web-frameworks" ] && [ "$suite" != "httparena" ] && \
   [ "$suite" != "all" ]; then
  echo "Usage: benchmark-runner {web-frameworks|httparena|all} [benchmark|validate]" >&2
  exit 2
fi

if [ "$mode" != "benchmark" ] && [ "$mode" != "validate" ]; then
  echo "Mode must be benchmark or validate" >&2
  exit 2
fi

copy_doba_adapter() {
  local source="$1"
  local target="$2"
  mkdir -p "$target"
  cp -a "$source"/. "$target"/
}

copy_web_results() {
  local checkout="$1"
  local artifacts="$2"
  local framework
  IFS=',' read -ra frameworks <<< "$WEB_FRAMEWORKS"
  for framework in "${frameworks[@]}"; do
    [ -d "$checkout/cpp/$framework/.results" ] || continue
    mkdir -p "$artifacts/$framework"
    cp -a "$checkout/cpp/$framework/.results" "$artifacts/$framework/"
  done
}

run_web_frameworks() {
  local checkout="/work/web-frameworks-$timestamp"
  local artifacts="/artifacts/web-frameworks/$timestamp"
  local framework
  mkdir -p "$artifacts"
  git clone --branch "$WEB_FRAMEWORKS_REF" --single-branch \
    https://github.com/the-benchmarker/web-frameworks.git "$checkout"
  copy_doba_adapter /adapters/web-frameworks "$checkout/cpp/doba"

  cd "$checkout"
  export BUNDLE_PATH=vendor/bundle
  bundle install
  bundle exec rake config

  IFS=',' read -ra frameworks <<< "$WEB_FRAMEWORKS"
  for framework in "${frameworks[@]}"; do
    [ -f "cpp/$framework/.Makefile" ] || {
      echo "Unknown Web Frameworks target: $framework" >&2
      exit 2
    }
    if [ "$mode" = "validate" ]; then
      make -f "cpp/$framework/.Makefile" build
      if ! make -f "cpp/$framework/.Makefile" test; then
        make -f "cpp/$framework/.Makefile" unbuild || true
        exit 1
      fi
      make -f "cpp/$framework/.Makefile" unbuild
    else
      ./run.sh "cpp/$framework"
    fi
  done
  copy_web_results "$checkout" "$artifacts"
}

copy_httparena_results() {
  local checkout="$1"
  local artifacts="$2"
  [ -d "$checkout/results" ] || return 0
  cp -a "$checkout/results" "$artifacts/results"
}

run_httparena() {
  local checkout="/work/httparena-$timestamp"
  local artifacts="/artifacts/httparena/$timestamp"
  local framework
  mkdir -p "$artifacts"
  git clone https://github.com/MDA2AV/HttpArena.git "$checkout"
  git -C "$checkout" checkout "$HTTPARENA_REF"
  copy_doba_adapter /adapters/httparena "$checkout/frameworks/doba"

  cd "$checkout"
  IFS=',' read -ra frameworks <<< "$HTTPARENA_FRAMEWORKS"
  for framework in "${frameworks[@]}"; do
    [ -f "frameworks/$framework/meta.json" ] || {
      echo "Unknown HttpArena target: $framework" >&2
      exit 2
    }
    if ! ./scripts/validate.sh "$framework" > "$artifacts/$framework.validate.log" 2>&1; then
      cat "$artifacts/$framework.validate.log"
      exit 1
    fi
    cat "$artifacts/$framework.validate.log"
    if [ "$mode" = "benchmark" ]; then
      if ! ./scripts/benchmark.sh "$framework" > "$artifacts/$framework.benchmark.log" 2>&1; then
        cat "$artifacts/$framework.benchmark.log"
        exit 1
      fi
      cat "$artifacts/$framework.benchmark.log"
    fi
  done
  copy_httparena_results "$checkout" "$artifacts"
}

case "$suite" in
  web-frameworks) run_web_frameworks ;;
  httparena) run_httparena ;;
  all)
    run_web_frameworks
    run_httparena
    ;;
esac
