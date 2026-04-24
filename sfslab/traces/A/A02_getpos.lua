assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local fd = check(disk.open("pos"))

assert(check(disk.getPos(fd)) == 0)
assert(check(disk.write(fd, "hello")) == 5)
assert(check(disk.getPos(fd)) == 5)

disk.close(fd)
assert(check(disk.unmount()) == 0)
