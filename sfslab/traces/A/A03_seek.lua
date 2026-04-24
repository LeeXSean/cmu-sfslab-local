assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local fd = check(disk.open("seek"))

assert(check(disk.write(fd, "abcdef")) == 6)
assert(check(disk.seek(fd, -2)) == 4)

local out = check(disk.read(fd, 2))
assert(out == "ef")

assert(check(disk.seek(fd, -100)) == 0)
disk.close(fd)
assert(check(disk.unmount()) == 0)
