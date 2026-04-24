#!/bin/sh
set -eu

json=0
starter_safe=0
while [ "$#" -gt 0 ]; do
    case "$1" in
        --json)
            json=1
            shift
            ;;
        --starter-safe)
            starter_safe=1
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            break
            ;;
    esac
done

if [ ! -x ./lua-sfs-runner ]; then
    echo "trace-run: lua-sfs-runner is missing; run make lua-runner"
    exit 1
fi

if [ "$#" -eq 0 ]; then
    if [ "$starter_safe" -eq 1 ]; then
        set -- $(awk -F '\t' 'NR > 1 && $4 == "yes" { print $3 }' \
            traces/MANIFEST.tsv)
    else
        set -- $(find traces -type f -name '*.lua' | sort)
    fi
fi

total=0
passed=0
failed=0
results=$(mktemp)
trap 'rm -f "$results"' EXIT HUP INT TERM

for trace in "$@"; do
    total=$((total + 1))
    if output=$(./lua-sfs-runner "$trace" 2>&1); then
        passed=$((passed + 1))
        ok=1
    else
        failed=1
        ok=0
    fi

    printf '%s\t%d\n' "$trace" "$ok" >> "$results"
    if [ "$json" -eq 0 ] && [ -n "$output" ]; then
        printf '%s\n' "$output"
    fi
done

if [ "$json" -eq 1 ]; then
    awk -F '\t' '
        function js(s) {
            gsub(/\\/, "\\\\", s)
            gsub(/"/, "\\\"", s)
            gsub(/\r/, "\\r", s)
            gsub(/\t/, "\\t", s)
            return "\"" s "\""
        }
        NR == FNR {
            if (FNR > 1 && $1 != "") {
                mid[$3] = $1
                mcat[$3] = $2
                msafe[$3] = $4
                mstarter[$3] = $5
                mpurpose[$3] = $6
            }
            next
        }
        {
            path = $1
            ok = $2 + 0
            id = mid[path]
            if (id == "") {
                id = path
                sub(/^.*\//, "", id)
                sub(/\.lua$/, "", id)
            }
            cat = mcat[path]
            if (cat == "") cat = "?"

            n++
            tpath[n] = path
            tid[n] = id
            tcat[n] = cat
            tok[n] = ok
            tsafe[n] = msafe[path]
            tstarter[n] = mstarter[path]
            tpurpose[n] = mpurpose[path]

            total++
            pass += ok
            ctotal[cat]++
            cpass[cat] += ok
        }
        END {
            print "{"
            printf "  \"official_style_traces\": {\"score\": %d, \"max\": %d, \"graded\": false},\n", pass, total
            print "  \"categories\": {"
            printf "    \"A\": {\"score\": %d, \"max\": %d},\n", cpass["A"], ctotal["A"]
            printf "    \"B\": {\"score\": %d, \"max\": %d},\n", cpass["B"], ctotal["B"]
            printf "    \"C\": {\"score\": %d, \"max\": %d}\n", cpass["C"], ctotal["C"]
            print "  },"
            print "  \"traces\": ["
            for (i = 1; i <= n; i++) {
                printf "    {\"id\": %s, \"category\": %s, \"path\": %s, \"starter_safe\": %s, \"starter_status\": %s, \"purpose\": %s, \"passed\": %s}%s\n", \
                    js(tid[i]), js(tcat[i]), js(tpath[i]), js(tsafe[i]), \
                    js(tstarter[i]), js(tpurpose[i]), tok[i] ? "true" : "false", \
                    i == n ? "" : ","
            }
            print "  ]"
            print "}"
        }
    ' traces/MANIFEST.tsv "$results"
else
    printf 'trace-run: %d/%d traces passed\n' "$passed" "$total"
fi

exit "$failed"
