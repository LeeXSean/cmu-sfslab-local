assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local fd = check(disk.open("del.txt"))
assert(check(disk.write(fd, "bye")) == 3)
disk.close(fd)

assert(check(disk.remove("del.txt")) == 0)
assert(disk.remove("del.txt") == errno.ENOENT)

for _, name in ipairs({ "file1", "file2", "file3" }) do
    fd = check(disk.open(name))
    disk.close(fd)
end

assert(check(disk.remove("file2")) == 0)
fd = check(disk.open("file4"))
disk.close(fd)

local count = 0
local seen = {}
for name in disk.list() do
    count = count + 1
    seen[name] = true
end
assert(count == 3)
assert(not seen["del.txt"])
assert(not seen["file2"])
assert(seen["file1"])
assert(seen["file3"])
assert(seen["file4"])

assert(check(disk.unmount()) == 0)
