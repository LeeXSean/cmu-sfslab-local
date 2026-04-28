#!/bin/sh
set -u

strict=0
while [ "$#" -gt 0 ]; do
    case "$1" in
        --strict)
            strict=1
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "lint: unknown option: $1" >&2
            exit 2
            ;;
    esac
done

status=0

require_or_warn() {
    if command -v "$1" >/dev/null 2>&1; then
        return 0
    fi

    if [ "$strict" -eq 1 ]; then
        echo "lint: missing required tool: $1" >&2
        status=1
    else
        echo "lint: $1 not found; skipping $2" >&2
    fi
    return 1
}

scripts=$(find local -type f -name '*.sh' | sort)
c_format_files=${SFS_LINT_C_FORMAT_FILES:-}

echo "lint: checking shell syntax"
for script in $scripts; do
    sh -n "$script" || status=1
done

if require_or_warn shellcheck "ShellCheck"; then
    echo "lint: running ShellCheck"
    for script in $scripts; do
        shellcheck "$script" || status=1
    done
fi

if require_or_warn clang-format "clang-format availability"; then
    if [ -n "$c_format_files" ]; then
        echo "lint: checking configured C formatting"
        for file in $c_format_files; do
            clang-format --dry-run --Werror "$file" || status=1
        done
    else
        echo "lint: clang-format found; no C format files configured"
    fi
fi

if [ "$status" -eq 0 ]; then
    echo "lint: ok"
fi

exit "$status"
