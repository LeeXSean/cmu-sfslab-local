#!/bin/sh
set -eu

tarball=${1:-sfslab-handout.tar}
case "$tarball" in
    /*) ;;
    *) tarball=$(pwd)/$tarball ;;
esac

if [ ! -f "$tarball" ]; then
    echo "dist-verify: missing tarball: $tarball" >&2
    exit 1
fi

tmp=$(mktemp -d "${TMPDIR:-/tmp}/sfslab-dist.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

manifest="$tmp/manifest.txt"
tar -tf "$tarball" > "$manifest"

bad_root=$(awk '$0 != "sfslab" && $0 !~ /^sfslab\// { print; exit }' "$manifest")
if [ -n "$bad_root" ]; then
    echo "dist-verify: unexpected top-level tar entry: $bad_root" >&2
    exit 1
fi

if grep -E '(^|/)\.git(/|$)|^\.github/|^docs/|sfslab-handout\.tar$' "$manifest" >/dev/null 2>&1; then
    echo "dist-verify: tarball contains repository-only or generated files" >&2
    grep -E '(^|/)\.git(/|$)|^\.github/|^docs/|sfslab-handout\.tar$' "$manifest" >&2
    exit 1
fi

bad_modes="$tmp/bad-modes.txt"
tar -tvf "$tarball" | awk '
{
    mode = $1
    path = $NF
    if (mode ~ /^d/ && mode != "drwxr-xr-x") {
        print mode, path
    } else if (mode ~ /^-/) {
        expected = "-rw-r--r--"
        if (path ~ /^sfslab\/local\/[^/]+\.sh$/) {
            expected = "-rwxr-xr-x"
        }
        if (mode != expected) {
            print mode, path
        }
    }
}
' > "$bad_modes"
if [ -s "$bad_modes" ]; then
    echo "dist-verify: tarball has unexpected file modes" >&2
    cat "$bad_modes" >&2
    exit 1
fi

tar -xf "$tarball" -C "$tmp"

if [ ! -f "$tmp/sfslab/Makefile" ]; then
    echo "dist-verify: extracted handout is missing sfslab/Makefile" >&2
    exit 1
fi

make -s -C "$tmp/sfslab" check >/dev/null
make -s -C "$tmp/sfslab" report-json > "$tmp/report.json"

if command -v python3 >/dev/null 2>&1; then
    python3 -m json.tool "$tmp/report.json" >/dev/null
else
    echo "dist-verify: python3 missing; skipped JSON syntax validation" >&2
fi

make -s -C "$tmp/sfslab" clean >/dev/null
echo "dist-verify: ok"
