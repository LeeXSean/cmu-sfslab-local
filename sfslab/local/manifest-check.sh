#!/bin/sh
set -eu

manifest=traces/MANIFEST.tsv
if [ ! -f "$manifest" ]; then
    echo "manifest-check: missing $manifest"
    exit 1
fi

tmp_manifest=$(mktemp)
tmp_grader=$(mktemp)
tmp_trace_list=$(mktemp)
trap 'rm -f "$tmp_manifest" "$tmp_grader" "$tmp_trace_list"' EXIT HUP INT TERM

awk -F '\t' 'NR > 1 && $1 != "" { print $1 }' "$manifest" | sort > "$tmp_manifest"
./test-sfs --list-traces > "$tmp_trace_list"
awk -F '\t' 'NR > 1 && $1 != "Perf" { print $2 }' "$tmp_trace_list" \
    | sort > "$tmp_grader"

if ! diff -u "$tmp_manifest" "$tmp_grader"; then
    echo "manifest-check: trace manifest differs from local autograder"
    exit 1
fi

if ! awk -F '\t' '
    NR > 1 {
        score[$1] += $4
        total += $4
    }
    END {
        ok = 1
        if (score["A"] != 5) {
            printf "manifest-check: A score changed to %d, expected 5\n", score["A"]
            ok = 0
        }
        if (score["B"] != 4) {
            printf "manifest-check: B score changed to %d, expected 4\n", score["B"]
            ok = 0
        }
        if (score["C"] != 3) {
            printf "manifest-check: C score changed to %d, expected 3\n", score["C"]
            ok = 0
        }
        if (score["Perf"] != 10) {
            printf "manifest-check: Perf score changed to %d, expected 10\n", score["Perf"]
            ok = 0
        }
        if (total != 22) {
            printf "manifest-check: total score changed to %d, expected 22\n", total
            ok = 0
        }
        exit ok ? 0 : 1
    }
' "$tmp_trace_list"; then
    echo "manifest-check: local 22-point score contract changed"
    exit 1
fi

echo "manifest-check: ok"
