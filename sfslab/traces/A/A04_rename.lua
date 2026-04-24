assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local fd = check(disk.open("old"))
assert(check(disk.write(fd, "rename me")) == 9)
disk.close(fd)

assert(check(disk.rename("old", "new")) == 0)

assert(check(disk.unmount()) == 0)
assert(check(disk.mount(TRACE_DISK)) == 0)

fd = check(disk.open("new"))
local out = check(disk.read(fd, 9))
assert(out == "rename me")
disk.close(fd)

assert(disk.rename("nonexistent", "whatever") == errno.ENOENT)

fd = check(disk.open("a"))
assert(check(disk.write(fd, "AAA")) == 3)
disk.close(fd)

fd = check(disk.open("b"))
assert(check(disk.write(fd, "BBB")) == 3)
disk.close(fd)

assert(check(disk.rename("a", "b")) == 0)
fd = check(disk.open("b"))
assert(check(disk.read(fd, 3)) == "AAA")
disk.close(fd)
assert(check(disk.unmount()) == 0)
