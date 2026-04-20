# -*- Makefile -*- for sfslab (local build)

CC      = gcc

LDFLAGS  = -O2 -g -pthread
CFLAGS   = -std=c11 $(LDFLAGS) $(WARNINGS)
CPPFLAGS = -D_GNU_SOURCE=1 -D_FORTIFY_SOURCE=2
WARNINGS = -Werror -Wall -Wextra -Wpedantic -Wconversion
WARNINGS += -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings
WARNINGS += -Wno-unused-parameter

PROGRAMS = sfs-fsck test-sfs

all: $(PROGRAMS)
.PHONY: all

sfs-fsck: sfs-fsck.o
	$(CC) $(LDFLAGS) -o $@ $^

test-sfs: test-sfs.o sfs-disk.o sfs-support.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	-rm -f $(PROGRAMS) test-sfs-tsan *.o *.img tsan_output.log
.PHONY: clean

## cut here ##
sfs-support.o: sfs-support.c sfs-disk.h
sfs-disk.o: sfs-disk.c sfs-api.h sfs-disk.h
sfs-fsck.o: sfs-fsck.c sfs-disk.h
test-sfs.o: test-sfs.c sfs-api.h
