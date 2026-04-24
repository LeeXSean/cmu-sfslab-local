assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local fd = check(disk.open("seek"))

assert(check(disk.write(fd, "0123456789")) == 10)
assert(check(disk.seek(fd, -5)) == 5)

local out = check(disk.read(fd, 3))
assert(out == "567")

assert(check(disk.seek(fd, -100)) == 0)
assert(check(disk.seek(fd, 9999)) == 10)
assert(disk.seek(-1, 0) == errno.EBADF)
disk.close(fd)
assert(check(disk.unmount()) == 0)
