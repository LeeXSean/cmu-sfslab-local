#!/bin/sh
set -eu

tarball=${1:-sfslab-handout.tar}

if [ ! -d sfslab ] || [ ! -f Makefile ]; then
    echo "dist-repro-check: run from the repository root" >&2
    exit 1
fi

if ! command -v sha256sum >/dev/null 2>&1; then
    echo "dist-repro-check: sha256sum is required" >&2
    exit 1
fi

tmp=$(mktemp -d "${TMPDIR:-/tmp}/sfslab-repro.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

make dist >/dev/null
cp "$tarball" "$tmp/first.tar"
first=$(sha256sum "$tmp/first.tar" | awk '{ print $1 }')

make dist >/dev/null
cp "$tarball" "$tmp/second.tar"
second=$(sha256sum "$tmp/second.tar" | awk '{ print $1 }')

if [ "$first" != "$second" ]; then
    echo "dist-repro-check: tarball is not reproducible" >&2
    echo "  first:  $first" >&2
    echo "  second: $second" >&2
    exit 1
fi

echo "dist-repro-check: ok ($first)"
