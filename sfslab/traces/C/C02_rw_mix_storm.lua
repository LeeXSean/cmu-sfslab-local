assert(check(disk.format(TRACE_DISK, TRACE_DISK_SIZE)) == 0)

local laneproc = lanes.gen("string,disk", function(tid)
    local name = string.format("mix-%d", tid)

    for i = 1, 100 do
        local fd = check(disk.open(name))
        local data = string.format("%d:%d", tid, i)
        assert(check(disk.write(fd, data)) == #data)
        assert(check(disk.seek(fd, -#data)) >= 0)
        assert(check(disk.read(fd, #data)) == data)
        disk.close(fd)
    end
    return true
end)

local workers = {}
for i = 1, 8 do
    workers[i] = laneproc(i)
end

for i = 1, 8 do
    check(workers[i]:join())
end

assert(check(disk.unmount()) == 0)
