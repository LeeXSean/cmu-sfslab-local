#!/bin/sh
set -eu

if [ ! -x ./lua-sfs-runner ]; then
    echo "trace-run: lua-sfs-runner is missing; run make lua-runner"
    exit 1
fi

if [ "$#" -gt 0 ]; then
    exec ./lua-sfs-runner "$@"
fi

find traces -type f -name '*.lua' | sort | xargs ./lua-sfs-runner
