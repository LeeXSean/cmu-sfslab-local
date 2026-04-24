#!/bin/sh
set -eu

missing=0
for path in \
    Makefile \
    README \
    SFS_Lab_Writeup.md \
    sfs-api.h \
    sfs-disk.c \
    sfs-disk.h \
    sfs-fsck.c \
    sfs-support.c \
    local/test-sfs.c \
    local/sfs-baseline-ref.c \
    local/OFFLINE_SELF_STUDY.md \
    local/SCORING.md \
    traces/README.md
do
    if [ ! -e "$path" ]; then
        echo "handout-check: missing $path"
        missing=1
    fi
done

if [ "$missing" -ne 0 ]; then
    exit 1
fi

if find . \( -name '*.o' -o -name '*.img' -o -name 'test-sfs' \
        -o -name 'test-sfs-baseline' -o -name 'sfs-fsck' \
        -o -name 'test-sfs-tsan' -o -name '.perf_baseline' \
        -o -name '.perf_baseline.tmp' -o -name 'tsan_output.log' \) \
        -print | grep . >/dev/null 2>&1; then
    echo "handout-check: generated files are present"
    find . \( -name '*.o' -o -name '*.img' -o -name 'test-sfs' \
        -o -name 'test-sfs-baseline' -o -name 'sfs-fsck' \
        -o -name 'test-sfs-tsan' -o -name '.perf_baseline' \
        -o -name '.perf_baseline.tmp' -o -name 'tsan_output.log' \) \
        -print
    exit 1
fi

if grep -RInE 'WeChat|微信|QQ|cstutor|tutorcs|代写' \
        . --exclude='handout-check.sh' --exclude-dir=.git >/dev/null 2>&1; then
    echo "handout-check: suspicious tutoring/contact text found"
    grep -RInE 'WeChat|微信|QQ|cstutor|tutorcs|代写' \
        . --exclude='handout-check.sh' --exclude-dir=.git
    exit 1
fi

sh local/check-traces.sh
echo "handout-check: ok"
