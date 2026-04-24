local laneproc = lanes.gen("string,disk", function(tid)
    local name = string.format("file-%d", tid)
    local fd = check(disk.open(name))
    local data = string.format("payload-%d", tid)

    assert(check(disk.write(fd, data)) == #data)
    disk.close(fd)
    return true
end)

local workers = {}
for i = 1, 8 do
    workers[i] = laneproc(i)
end

for i = 1, 8 do
    check(workers[i]:join())
end
