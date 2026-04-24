#!/bin/sh
set -eu

count=$(grep -c 'return -ENOSYS;' sfs-disk.c || true)
if [ "$count" -ne 3 ]; then
    echo "starter-check: expected three ENOSYS stubs in sfs-disk.c, found $count"
    exit 1
fi

for name in sfs_getpos sfs_seek sfs_rename; do
    if ! grep -n "$name" sfs-disk.c >/dev/null 2>&1; then
        echo "starter-check: missing $name"
        exit 1
    fi
done

echo "starter-check: ok"
