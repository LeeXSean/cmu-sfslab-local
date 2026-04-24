local block = string.rep("x", 600)
local fd = check(disk.open("large"))

assert(check(disk.write(fd, block .. "tail")) == 604)
assert(check(disk.seek(fd, -4)) == 600)

local out = check(disk.read(fd, 4))
assert(out == "tail")
disk.close(fd)
