#!/bin/sh
set -eu

if [ ! -x ./lua-sfs-runner ]; then
    echo "trace-run: lua-sfs-runner is missing; run make lua-runner"
    exit 1
fi

if [ "$#" -eq 0 ]; then
    set -- $(find traces -type f -name '*.lua' | sort)
fi

total=0
passed=0
failed=0

for trace in "$@"; do
    total=$((total + 1))
    if ./lua-sfs-runner "$trace"; then
        passed=$((passed + 1))
    else
        failed=1
    fi
done

printf 'trace-run: %d/%d traces passed\n' "$passed" "$total"
exit "$failed"
