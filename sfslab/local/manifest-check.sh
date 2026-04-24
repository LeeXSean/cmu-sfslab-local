#!/bin/sh
set -eu

manifest=traces/MANIFEST.tsv
if [ ! -f "$manifest" ]; then
    echo "manifest-check: missing $manifest"
    exit 1
fi

tmp_manifest=$(mktemp)
tmp_grader=$(mktemp)
trap 'rm -f "$tmp_manifest" "$tmp_grader"' EXIT HUP INT TERM

awk -F '\t' 'NR > 1 && $1 != "" { print $1 }' "$manifest" | sort > "$tmp_manifest"
./test-sfs --list-traces \
    | awk -F '\t' 'NR > 1 && $1 != "Perf" { print $2 }' \
    | sort > "$tmp_grader"

if ! diff -u "$tmp_manifest" "$tmp_grader"; then
    echo "manifest-check: trace manifest differs from local autograder"
    exit 1
fi

echo "manifest-check: ok"
