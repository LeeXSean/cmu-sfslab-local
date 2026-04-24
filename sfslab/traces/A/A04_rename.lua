local fd = check(disk.open("old"))
assert(check(disk.write(fd, "payload")) == 7)
disk.close(fd)

assert(check(disk.rename("old", "new")) == 0)

fd = check(disk.open("new"))
local out = check(disk.read(fd, 7))
assert(out == "payload")
disk.close(fd)
