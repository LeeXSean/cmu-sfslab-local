assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local fd = check(disk.open("pos"))

assert(check(disk.getPos(fd)) == 0)
assert(check(disk.write(fd, "abcdefghij")) == 10)
assert(check(disk.getPos(fd)) == 10)

assert(disk.getPos(-1) == errno.EBADF)
assert(disk.getPos(99) == errno.EBADF)

disk.close(fd)
assert(check(disk.unmount()) == 0)
