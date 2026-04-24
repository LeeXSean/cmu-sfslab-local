assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local fd = check(disk.open("src"))
assert(check(disk.write(fd, "SRC-DATA")) == 8)
disk.close(fd)

fd = check(disk.open("dst"))
assert(check(disk.write(fd, "D")) == 1)
disk.close(fd)

fd = check(disk.open("keep"))
disk.close(fd)

assert(check(disk.rename("src", "dst")) == 0)

local seen = {}
local count = 0
for name in disk.list() do
    count = count + 1
    seen[name] = true
end

assert(count == 2)
assert(not seen["src"])
assert(seen["dst"])
assert(seen["keep"])

fd = check(disk.open("dst"))
assert(check(disk.read(fd, 16)) == "SRC-DATA")
disk.close(fd)

fd = check(disk.open("reuse"))
assert(check(disk.write(fd, "R")) == 1)
disk.close(fd)

seen = {}
count = 0
for name in disk.list() do
    count = count + 1
    seen[name] = true
end

assert(count == 3)
assert(not seen["src"])
assert(seen["dst"])
assert(seen["keep"])
assert(seen["reuse"])

local longname = string.rep("x", 31)
assert(disk.remove(longname) == errno.ENAMETOOLONG)
assert(disk.rename("dst", longname) == errno.ENAMETOOLONG)
assert(disk.list(0) == errno.EINVAL)

assert(check(disk.unmount()) == 0)
