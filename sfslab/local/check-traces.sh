#!/bin/sh
set -eu

if command -v luac5.4 >/dev/null 2>&1; then
    LUAC=luac5.4
elif command -v luac >/dev/null 2>&1; then
    LUAC=luac
else
    echo "trace-check: luac not found; skipping"
    exit 0
fi

traces=$(find traces -type f -name '*.lua' | sort)
if [ -z "$traces" ]; then
    echo "trace-check: no traces found"
    exit 0
fi

printf '%s\n' "$traces" | while IFS= read -r trace; do
    printf 'trace-check: %s\n' "$trace"
    "$LUAC" -p "$trace"
done
