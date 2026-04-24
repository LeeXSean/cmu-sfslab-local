assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local big = string.rep("A", 500) .. string.rep("B", 500) ..
    string.rep("C", 200)
local fd = check(disk.open("multi.txt"))

assert(check(disk.write(fd, big)) == 1200)
assert(check(disk.getPos(fd)) == 1200)

assert(check(disk.seek(fd, -710)) == 490)
assert(check(disk.read(fd, 20)) ==
    string.rep("A", 10) .. string.rep("B", 10))
assert(check(disk.getPos(fd)) == 510)

assert(check(disk.seek(fd, 489)) == 999)
local out = check(disk.read(fd, 10))
assert(out:sub(1, 1) == "B")
assert(out:sub(2) == string.rep("C", 9))

local saved = check(disk.getPos(fd))
assert(check(disk.seek(fd, 200)) == 1200)
assert(check(disk.seek(fd, saved - check(disk.getPos(fd)))) == saved)

assert(check(disk.seek(fd, -9999)) == 0)
assert(check(disk.read(fd, 5)) == "AAAAA")
disk.close(fd)

assert(check(disk.rename("multi.txt", "renamed.txt")) == 0)
fd = check(disk.open("renamed.txt"))
assert(check(disk.read(fd, 3)) == "AAA")
disk.close(fd)
assert(check(disk.remove("renamed.txt")) == 0)

assert(check(disk.unmount()) == 0)
