#!/bin/sh
set -u

status=0

say_ok() {
    printf '  OK   %s\n' "$1"
}

say_warn() {
    printf '  WARN %s\n' "$1"
}

say_fail() {
    printf '  FAIL %s\n' "$1"
    status=1
}

have() {
    command -v "$1" >/dev/null 2>&1
}

check_cmd() {
    if have "$1"; then
        say_ok "$1 found"
    else
        say_fail "$1 missing"
    fi
}

printf 'SFS lab environment doctor\n'
printf '==========================\n'

printf '\nCore build tools:\n'
for tool in sh make gcc awk sed grep sort tar mktemp; do
    check_cmd "$tool"
done

printf '\nOptional trace tools:\n'
lua_ready=0
if have lua5.4; then
    say_ok "lua5.4 found"
else
    say_warn "lua5.4 missing"
fi

if have luac; then
    say_ok "luac found"
else
    say_warn "luac missing; make trace-check will skip Lua syntax checks"
fi

if have pkg-config; then
    say_ok "pkg-config found"
    if pkg-config --exists lua5.4 >/dev/null 2>&1; then
        say_ok "lua5.4 development package found"
        if have lua5.4; then
            lua_ready=1
        fi
    else
        say_warn "lua5.4 development package missing"
    fi
else
    say_warn "pkg-config missing; local Lua runner build will be unavailable"
fi

printf '\nOptional lint tools:\n'
if have shellcheck; then
    say_ok "shellcheck found"
else
    say_warn "shellcheck missing; make lint will skip ShellCheck"
fi

if have clang-format; then
    say_ok "clang-format found"
else
    say_warn "clang-format missing; make lint will skip C format checks"
fi

printf '\nDocker fallback:\n'
docker_ready=0
if have docker; then
    say_ok "docker command found"
    if docker info >/dev/null 2>&1; then
        say_ok "docker daemon reachable"
        docker_ready=1
    else
        say_warn "docker daemon not reachable"
    fi
else
    say_warn "docker command missing"
fi

printf '\nRecommended commands:\n'
printf '  make check          basic package health\n'
printf '  make lint           syntax and optional style checks\n'
printf '  make lint-strict    require shellcheck and clang-format\n'
printf '  make report-json    JSON score report, exits 0 when report generation works\n'
printf '  make trace-smoke    Lua starter-safe traces, using Docker fallback if needed\n'
printf '  make dist-verify    rebuild and unpack-check the handout tarball\n'

if [ "$lua_ready" -eq 0 ] && [ "$docker_ready" -eq 0 ]; then
    say_fail "neither local Lua development tools nor Docker fallback are ready"
    printf '\nInstall on Ubuntu/WSL with:\n'
    printf '  sudo apt-get install build-essential make pkg-config lua5.4 liblua5.4-dev\n'
fi

exit "$status"
