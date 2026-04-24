#!/bin/sh
set -eu

traces=$(find traces -type f -name '*.lua' | sort)
if [ -z "$traces" ]; then
    echo "trace-check: no traces found"
    exit 0
fi

manifest=traces/MANIFEST.tsv
listed=""
if [ -f "$manifest" ]; then
    first=1
    while IFS="$(printf '\t')" read -r id category file purpose; do
        if [ "$first" = 1 ]; then
            first=0
            continue
        fi
        if [ -z "$id" ]; then
            continue
        fi
        if [ ! -f "$file" ]; then
            echo "trace-check: manifest file missing: $file"
            exit 1
        fi
        case "$(basename "$file")" in
            "$id"_*.lua | "$id".lua) ;;
            *)
                echo "trace-check: $file does not match id $id"
                exit 1
                ;;
        esac
        listed="${listed}${file}
"
    done < "$manifest"

    for trace in $traces; do
        if ! printf '%s' "$listed" | grep -Fx "$trace" >/dev/null 2>&1; then
            echo "trace-check: unlisted trace: $trace"
            exit 1
        fi
    done
fi

if command -v luac5.4 >/dev/null 2>&1; then
    LUAC=luac5.4
elif command -v luac >/dev/null 2>&1; then
    LUAC=luac
else
    echo "trace-check: luac not found; syntax check skipped"
    exit 0
fi

printf '%s\n' "$traces" | while IFS= read -r trace; do
    printf 'trace-check: %s\n' "$trace"
    "$LUAC" -p "$trace"
done
