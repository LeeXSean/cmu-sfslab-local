#!/bin/sh
set -eu

mode=${1:-run}
shift || true

case "$mode" in
    run)
        trace_args=""
        docker_target=docker-trace-run
        label=trace-run
        ;;
    smoke)
        trace_args="--starter-safe"
        docker_target=docker-trace-smoke
        label=trace-smoke
        ;;
    json)
        trace_args="--json"
        docker_target=docker-trace-json
        label=trace-json
        ;;
    starter-safe)
        trace_args="--starter-safe"
        docker_target=docker-trace-smoke
        label=starter-safe
        ;;
    *)
        echo "usage: $0 [run|smoke|json|starter-safe] [TRACE.lua ...]" >&2
        exit 2
        ;;
esac

script_dir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
handout_dir=$(CDPATH='' cd -- "$script_dir/.." && pwd)
repo_root=$(CDPATH='' cd -- "$handout_dir/.." && pwd)

err=$(mktemp)
trap 'rm -f "$err"' EXIT HUP INT TERM

cd "$handout_dir"
if sh local/build-lua-runner.sh > "$err" 2>&1; then
    # shellcheck disable=SC2086
    exec sh local/run-lua-traces.sh $trace_args "$@"
else
    status=$?
fi

if [ "$status" -ne 77 ]; then
    cat "$err" >&2
    exit "$status"
fi

if [ "${SFS_LUA_DOCKER_FALLBACK:-1}" != "0" ] &&
   [ -f "$repo_root/Dockerfile" ] &&
    command -v docker >/dev/null 2>&1; then
    cat "$err" >&2
    echo "$label: local Lua deps unavailable; retrying in Docker" >&2
    cd "$repo_root"
    exec make --no-print-directory "$docker_target"
fi

cat "$err" >&2
echo "$label: install pkg-config, lua5.4, and liblua5.4-dev, or run make $docker_target from the repository root" >&2
exit 77
