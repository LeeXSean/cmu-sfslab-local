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

local count = 0
for _ in disk.list() do
    count = count + 1
end
assert(count == 3)

assert(check(disk.unmount()) == 0)
