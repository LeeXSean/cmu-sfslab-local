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

for i = 1, 20 do
    local name = string.format("dir-%02d", i)
    local fd = check(disk.open(name))
    assert(check(disk.write(fd, "dir")) == 3)
    disk.close(fd)

    local saw = false
    for listed in disk.list() do
        if listed == name then
            saw = true
        end
    end
    assert(saw)
    assert(check(disk.remove(name)) == 0)
end

for listed in disk.list() do
    assert(not string.match(listed, "^dir%-"))
end

assert(check(disk.unmount()) == 0)
