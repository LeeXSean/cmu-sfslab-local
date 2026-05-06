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

baseline=
baseline_label=

if [ -f "$tarball" ]; then
    cp "$tarball" "$tmp/baseline.tar"
    baseline=$tmp/baseline.tar
    baseline_label="existing $tarball"
elif git rev-parse --is-inside-work-tree >/dev/null 2>&1 &&
    git rev-parse --verify HEAD >/dev/null 2>&1 &&
    git cat-file -e "HEAD:$tarball" >/dev/null 2>&1; then
    git show "HEAD:$tarball" > "$tmp/baseline.tar"
    baseline=$tmp/baseline.tar
    baseline_label="HEAD:$tarball"
fi

make dist >/dev/null
cp "$tarball" "$tmp/first.tar"
first=$(sha256sum "$tmp/first.tar" | awk '{ print $1 }')

if [ -n "$baseline" ]; then
    baseline_hash=$(sha256sum "$baseline" | awk '{ print $1 }')
    if [ "$baseline_hash" != "$first" ]; then
        echo "dist-repro-check: $baseline_label is stale" >&2
        echo "  $baseline_label: $baseline_hash" >&2
        echo "  generated: $first" >&2
        exit 1
    fi
fi

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
