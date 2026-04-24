#!/bin/sh
set -eu

if ! command -v pkg-config >/dev/null 2>&1; then
    echo "lua-runner: pkg-config not found"
    echo "lua-runner: install pkg-config, lua5.4, and liblua5.4-dev"
    exit 77
fi

pkg=
for candidate in lua5.4 lua-5.4 lua; do
    if pkg-config --exists "$candidate"; then
        pkg=$candidate
        break
    fi
done

if [ -z "$pkg" ]; then
    echo "lua-runner: Lua development package not found"
    echo "lua-runner: install pkg-config, lua5.4, and liblua5.4-dev"
    exit 77
fi

cc=${CC:-gcc}
cflags=${CFLAGS:-"-std=c11 -O2 -g -pthread -Werror -Wall -Wextra -Wpedantic -Wconversion -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings -Wno-unused-parameter"}
cppflags=${CPPFLAGS:-"-I. -D_GNU_SOURCE=1 -D_FORTIFY_SOURCE=2"}
ldflags=${LDFLAGS:-"-O2 -g -pthread"}

$cc $cflags $cppflags $(pkg-config --cflags "$pkg") \
    -o lua-sfs-runner local/lua-sfs-runner.c sfs-disk.c sfs-support.c \
    $ldflags $(pkg-config --libs "$pkg")
