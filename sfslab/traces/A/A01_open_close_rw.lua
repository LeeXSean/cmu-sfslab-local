local fd = check(disk.open("hello.txt"))
assert(fd >= 0)

assert(check(disk.write(fd, "Hello, SFS!")) == 11)
disk.close(fd)

fd = check(disk.open("hello.txt"))
assert(check(disk.read(fd, 11)) == "Hello, SFS!")
disk.close(fd)
