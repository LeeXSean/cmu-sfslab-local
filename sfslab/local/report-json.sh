#!/bin/sh
set -u

grader_json=$(mktemp)
grader_err=$(mktemp)
trace_json=$(mktemp)
trace_err=$(mktemp)
trap 'rm -f "$grader_json" "$grader_err" "$trace_json" "$trace_err"' EXIT HUP INT TERM

./test-sfs --json > "$grader_json" 2> "$grader_err"
grader_status=$?

trace_available=true
trace_status=0
if sh local/build-lua-runner.sh > /dev/null 2> "$trace_err"; then
    sh local/run-lua-traces.sh --json > "$trace_json" 2> "$trace_err"
    trace_status=$?
else
    trace_available=false
    trace_status=77
fi

printf '{\n'
printf '  "local_autograder": {\n'
printf '    "exit_status": %d,\n' "$grader_status"
if [ -s "$grader_json" ]; then
    printf '    "result": '
    sed '1s/^//; 2,$s/^/    /' "$grader_json"
    printf '\n'
else
    printf '    "result": null\n'
fi
printf '  },\n'
printf '  "official_style_traces": {\n'
printf '    "available": %s,\n' "$trace_available"
printf '    "exit_status": %d,\n' "$trace_status"
if [ "$trace_available" = true ]; then
    printf '    "result": '
    sed '1s/^//; 2,$s/^/    /' "$trace_json"
    printf '\n'
else
    printf '    "result": null\n'
fi
printf '  },\n'
printf '  "combined": {\n'
printf '    "graded_total_uses": "local_autograder",\n'
printf '    "trace_coverage_is_diagnostic": true,\n'
if [ "$grader_status" -eq 0 ] && [ "$trace_status" -eq 0 ]; then
    printf '    "passed": true\n'
else
    printf '    "passed": false\n'
fi
printf '  }\n'
printf '}\n'

if [ "$grader_status" -ne 0 ]; then
    exit "$grader_status"
fi
if [ "$trace_status" -ne 0 ] && [ "$trace_status" -ne 77 ]; then
    exit "$trace_status"
fi
exit 0
