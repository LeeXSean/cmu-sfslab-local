assert(check(disk.format("trace.img", 1048576)) == 0)
assert(check(disk.unmount()) == 0)
assert(check(disk.mount("trace.img")) == 0)
assert(check(disk.unmount()) == 0)
