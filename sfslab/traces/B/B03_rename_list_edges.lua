local fd = check(disk.open("src"))
assert(check(disk.write(fd, "SRC")) == 3)
disk.close(fd)

fd = check(disk.open("dst"))
assert(check(disk.write(fd, "DST")) == 3)
disk.close(fd)

fd = check(disk.open("keep"))
disk.close(fd)

assert(check(disk.rename("src", "dst")) == 0)

local seen = {}
for name in disk.list() do
    seen[name] = true
end

assert(not seen["src"])
assert(seen["dst"])
assert(seen["keep"])

fd = check(disk.open("dst"))
assert(check(disk.read(fd, 3)) == "SRC")
disk.close(fd)
