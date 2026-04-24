local fd = check(disk.open("shared"))
assert(check(disk.write(fd, string.rep("x", 256))) == 256)
disk.close(fd)

local laneproc = lanes.gen("string,disk", function()
    local local_fd = check(disk.open("shared"))
    local data = check(disk.read(local_fd, 256))
    disk.close(local_fd)
    assert(#data == 256)
    return true
end)

local workers = {}
for i = 1, 8 do
    workers[i] = laneproc()
end

for i = 1, 8 do
    check(workers[i]:join())
end
