assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)
assert(check(disk.unmount()) == 0)
assert(check(disk.mount(TRACE_DISK)) == 0)
assert(check(disk.unmount()) == 0)
