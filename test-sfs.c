/*
 * test-sfs.c - SFS Lab Autograder (local, no Lua required)
 *
 * Mirrors the original CMU grading structure:
 *   Category A (5 pts) - Feature tests
 *   Category B (3 pts) - Sequential correctness
 *   Category C (3 pts) - Concurrent correctness
 *   Performance  (10 pts) - Concurrent throughput benchmark
 *   Style        (4 pts) - Manual review (not auto-graded)
 *
 * Build:  make test-sfs
 * Run:    ./test-sfs
 */

#include "sfs-api.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define DISK_NAME "test.img"
#define CONC_DISK "test_conc.img"

/* Forward declarations for perf helpers; score_perf_against_baseline is
   defined in a later part of this file (added in a subsequent task). */
static double run_perf_benchmark_raw(void);
static int score_perf_against_baseline(double student_ops);

/* ------------------------------------------------------------------ */
/*  Per-trace check infrastructure                                     */
/* ------------------------------------------------------------------ */

static int trace_ok;

#define CHECK(cond, fmt, ...)                                                  \
    do                                                                         \
    {                                                                          \
        if (!(cond))                                                           \
        {                                                                      \
            fprintf(stderr, "       -> FAIL [%s:%d]: " fmt "\n",               \
                    __FILE__, __LINE__, ##__VA_ARGS__);                         \
            trace_ok = 0;                                                      \
        }                                                                      \
    } while (0)

static size_t disk_size(void)
{
    long ps = sysconf(_SC_PAGESIZE);
    return (size_t)ps * 64;
}

/* ================================================================== */
/*  Category A — Feature Tests                                         */
/* ================================================================== */

/* A00: format, mount, unmount */
static int trace_A00(void)
{
    trace_ok = 1;
    int r = sfs_format(DISK_NAME, disk_size());
    CHECK(r == 0, "sfs_format returned %d", r);

    r = sfs_unmount();
    CHECK(r == 0, "sfs_unmount returned %d", r);

    r = sfs_mount(DISK_NAME);
    CHECK(r == 0, "sfs_mount returned %d", r);

    r = sfs_unmount();
    CHECK(r == 0, "sfs_unmount returned %d", r);
    return trace_ok;
}

/* A01: open, close, read, write */
static int trace_A01(void)
{
    trace_ok = 1;
    sfs_format(DISK_NAME, disk_size());

    int fd = sfs_open("hello.txt");
    CHECK(fd >= 0, "sfs_open returned %d", fd);

    const char *msg = "Hello, SFS!";
    ssize_t nw = sfs_write(fd, msg, strlen(msg));
    CHECK(nw == (ssize_t)strlen(msg), "sfs_write returned %zd", nw);
    sfs_close(fd);

    fd = sfs_open("hello.txt");
    CHECK(fd >= 0, "sfs_open (reopen) returned %d", fd);

    char buf[64] = {0};
    ssize_t nr = sfs_read(fd, buf, sizeof buf);
    CHECK(nr == (ssize_t)strlen(msg), "sfs_read returned %zd", nr);
    CHECK(memcmp(buf, msg, (size_t)nr) == 0, "data mismatch");
    sfs_close(fd);

    sfs_unmount();
    return trace_ok;
}

/* A02: sfs_getpos */
static int trace_A02(void)
{
    trace_ok = 1;
    sfs_format(DISK_NAME, disk_size());

    int fd = sfs_open("pos.txt");
    CHECK(fd >= 0, "sfs_open returned %d", fd);

    ssize_t pos = sfs_getpos(fd);
    CHECK(pos == 0, "initial getpos should be 0, got %zd", pos);

    sfs_write(fd, "abcdefghij", 10);
    pos = sfs_getpos(fd);
    CHECK(pos == 10, "getpos after write(10) should be 10, got %zd", pos);

    CHECK(sfs_getpos(-1) == -EBADF, "getpos(-1) should return -EBADF");
    CHECK(sfs_getpos(99) == -EBADF, "getpos(99) should return -EBADF");

    sfs_close(fd);
    sfs_unmount();
    return trace_ok;
}

/* A03: sfs_seek */
static int trace_A03(void)
{
    trace_ok = 1;
    sfs_format(DISK_NAME, disk_size());

    int fd = sfs_open("seek.txt");
    sfs_write(fd, "0123456789", 10);

    ssize_t p = sfs_seek(fd, -5);
    CHECK(p == 5, "seek(-5) from 10 -> expected 5, got %zd", p);

    char buf[4] = {0};
    ssize_t nr = sfs_read(fd, buf, 3);
    CHECK(nr == 3 && memcmp(buf, "567", 3) == 0,
          "read after seek: got %zd bytes '%.*s'", nr, (int)nr, buf);

    p = sfs_seek(fd, -100);
    CHECK(p == 0, "seek past beginning should clamp to 0, got %zd", p);

    p = sfs_seek(fd, 9999);
    CHECK(p == 10, "seek past end should clamp to file size, got %zd", p);

    CHECK(sfs_seek(-1, 0) == -EBADF, "seek(-1,0) should return -EBADF");

    sfs_close(fd);
    sfs_unmount();
    return trace_ok;
}

/* A04: sfs_rename (basic + overwrite) */
static int trace_A04(void)
{
    trace_ok = 1;
    sfs_format(DISK_NAME, disk_size());

    int fd = sfs_open("old.txt");
    sfs_write(fd, "rename me", 9);
    sfs_close(fd);

    int r = sfs_rename("old.txt", "new.txt");
    CHECK(r == 0, "sfs_rename returned %d", r);

    sfs_unmount();
    sfs_mount(DISK_NAME);

    fd = sfs_open("new.txt");
    CHECK(fd >= 0, "open new.txt after rename returned %d", fd);
    char buf[32] = {0};
    ssize_t nr = sfs_read(fd, buf, sizeof buf);
    CHECK(nr == 9 && memcmp(buf, "rename me", 9) == 0,
          "data after rename mismatch");
    sfs_close(fd);

    r = sfs_rename("nonexistent", "whatever");
    CHECK(r == -ENOENT, "rename nonexistent should be -ENOENT, got %d", r);

    /* overwrite rename */
    fd = sfs_open("a.txt");
    sfs_write(fd, "AAA", 3);
    sfs_close(fd);

    fd = sfs_open("b.txt");
    sfs_write(fd, "BBB", 3);
    sfs_close(fd);

    r = sfs_rename("a.txt", "b.txt");
    CHECK(r == 0, "rename a->b returned %d", r);

    fd = sfs_open("b.txt");
    CHECK(fd >= 0, "open b.txt after overwrite rename returned %d", fd);
    memset(buf, 0, sizeof buf);
    nr = sfs_read(fd, buf, sizeof buf);
    CHECK(nr == 3 && memcmp(buf, "AAA", 3) == 0,
          "b.txt should contain 'AAA', got '%.*s'", (int)nr, buf);
    sfs_close(fd);

    sfs_unmount();
    return trace_ok;
}

/* ================================================================== */
/*  Category B — Sequential Correctness                                */
/* ================================================================== */

/* B00: remove + list */
static int trace_B00(void)
{
    trace_ok = 1;
    sfs_format(DISK_NAME, disk_size());

    int fd = sfs_open("del.txt");
    sfs_write(fd, "bye", 3);
    sfs_close(fd);

    int r = sfs_remove("del.txt");
    CHECK(r == 0, "sfs_remove returned %d", r);
    r = sfs_remove("del.txt");
    CHECK(r == -ENOENT, "double remove should be -ENOENT, got %d", r);

    int fd1 = sfs_open("file1");
    int fd2 = sfs_open("file2");
    int fd3 = sfs_open("file3");
    sfs_close(fd1);
    sfs_close(fd2);
    sfs_close(fd3);

    sfs_list_cookie cookie = NULL;
    char name[SFS_FILE_NAME_SIZE_LIMIT];
    int count = 0;
    while (sfs_list(&cookie, name, sizeof name) == 0)
        count++;
    CHECK(count == 3, "expected 3 files, got %d", count);

    sfs_unmount();
    return trace_ok;
}

/* B01: multi-block file + seek across block boundaries
   BLOCK_DATA_SIZE = 500, so a 1200-byte file spans 3 blocks.
   This is the critical test for sfs_seek: the student must walk
   the block linked list when seeking across block boundaries. */
static int trace_B01(void)
{
    trace_ok = 1;
    sfs_format(DISK_NAME, disk_size());

    /* Build a 1200-byte payload: 'A' x 500 + 'B' x 500 + 'C' x 200 */
    char big[1200];
    memset(big, 'A', 500);
    memset(big + 500, 'B', 500);
    memset(big + 1000, 'C', 200);

    int fd = sfs_open("multi.txt");
    CHECK(fd >= 0, "open multi.txt returned %d", fd);

    ssize_t nw = sfs_write(fd, big, 1200);
    CHECK(nw == 1200, "write 1200 bytes returned %zd", nw);

    ssize_t pos = sfs_getpos(fd);
    CHECK(pos == 1200, "getpos after write(1200) should be 1200, got %zd", pos);

    /* Seek back to byte 490 (still in block 0, near boundary) */
    ssize_t p = sfs_seek(fd, -710);
    CHECK(p == 490, "seek(-710) from 1200 -> expected 490, got %zd", p);

    /* Read 20 bytes: should cross from block 0 into block 1 */
    char buf[64] = {0};
    ssize_t nr = sfs_read(fd, buf, 20);
    CHECK(nr == 20, "read(20) at pos 490 returned %zd", nr);
    /* bytes 490-499 are 'A', bytes 500-509 are 'B' */
    char expected[20];
    memset(expected, 'A', 10);
    memset(expected + 10, 'B', 10);
    CHECK(memcmp(buf, expected, 20) == 0,
          "cross-boundary read at 490: data mismatch");

    pos = sfs_getpos(fd);
    CHECK(pos == 510, "getpos after cross-boundary read should be 510, got %zd",
          pos);

    /* Seek to byte 999 (last byte of block 1), read into block 2 */
    p = sfs_seek(fd, 489);
    CHECK(p == 999, "seek(489) from 510 -> expected 999, got %zd", p);

    nr = sfs_read(fd, buf, 10);
    CHECK(nr == 10, "read(10) at pos 999 returned %zd", nr);
    /* byte 999 is 'B', bytes 1000-1008 are 'C' */
    CHECK(buf[0] == 'B' && buf[1] == 'C' && buf[9] == 'C',
          "cross-boundary read at 999: data mismatch");

    /* Seek round-trip: spec says sfs_seek(fd, loc - sfs_getpos(fd))
       should restore position */
    ssize_t saved = sfs_getpos(fd);
    sfs_seek(fd, 200);
    p = sfs_seek(fd, saved - sfs_getpos(fd));
    CHECK(p == saved,
          "seek round-trip: expected %zd, got %zd", saved, p);

    /* Seek all the way back to 0 from deep in the file */
    p = sfs_seek(fd, -9999);
    CHECK(p == 0, "seek to beginning should clamp to 0, got %zd", p);

    /* Read from beginning to verify block chain is intact */
    nr = sfs_read(fd, buf, 5);
    CHECK(nr == 5 && memcmp(buf, "AAAAA", 5) == 0,
          "read from beginning after seek-back: data mismatch");

    sfs_close(fd);

    /* Rename + remove workflow */
    int r = sfs_rename("multi.txt", "renamed.txt");
    CHECK(r == 0, "rename multi.txt returned %d", r);

    fd = sfs_open("renamed.txt");
    CHECK(fd >= 0, "open renamed.txt returned %d", fd);
    nr = sfs_read(fd, buf, 3);
    CHECK(nr == 3 && memcmp(buf, "AAA", 3) == 0,
          "renamed file data mismatch");
    sfs_close(fd);

    r = sfs_remove("renamed.txt");
    CHECK(r == 0, "remove renamed.txt returned %d", r);

    sfs_unmount();
    return trace_ok;
}

/* B02: edge cases + getpos tracking + multi-fd independence */
static int trace_B02(void)
{
    trace_ok = 1;
    sfs_format(DISK_NAME, disk_size());

    /* read from empty file */
    int fd = sfs_open("empty.txt");
    char buf[64];
    ssize_t nr = sfs_read(fd, buf, sizeof buf);
    CHECK(nr == 0, "read from empty file should return 0, got %zd", nr);

    /* getpos on empty file */
    ssize_t pos = sfs_getpos(fd);
    CHECK(pos == 0, "getpos on empty file should be 0, got %zd", pos);

    /* seek on empty file */
    ssize_t p = sfs_seek(fd, 100);
    CHECK(p == 0, "seek(100) on empty file should clamp to 0, got %zd", p);
    p = sfs_seek(fd, -100);
    CHECK(p == 0, "seek(-100) on empty file should clamp to 0, got %zd", p);
    sfs_close(fd);

    /* name too long */
    char longname[SFS_FILE_NAME_SIZE_LIMIT + 8];
    memset(longname, 'x', sizeof longname - 1);
    longname[sizeof longname - 1] = '\0';
    int r = sfs_open(longname);
    CHECK(r == -ENAMETOOLONG, "open(longname) should be -ENAMETOOLONG, got %d",
          r);

    /* getpos tracks reads correctly */
    fd = sfs_open("track.txt");
    sfs_write(fd, "0123456789abcdef", 16);
    sfs_close(fd);
    fd = sfs_open("track.txt");
    nr = sfs_read(fd, buf, 7);
    CHECK(nr == 7, "read(7) returned %zd", nr);
    pos = sfs_getpos(fd);
    CHECK(pos == 7, "getpos after read(7) should be 7, got %zd", pos);
    nr = sfs_read(fd, buf, 3);
    CHECK(nr == 3, "read(3) returned %zd", nr);
    pos = sfs_getpos(fd);
    CHECK(pos == 10, "getpos after read(7)+read(3) should be 10, got %zd", pos);
    CHECK(memcmp(buf, "789", 3) == 0, "read(3) data mismatch");
    sfs_close(fd);

    /* double open: independent positions */
    fd = sfs_open("dup.txt");
    sfs_write(fd, "hello world!", 12);
    int fd2 = sfs_open("dup.txt");
    CHECK(fd2 >= 0 && fd2 != fd, "second open should give different fd");

    nr = sfs_read(fd2, buf, 5);
    CHECK(nr == 5 && memcmp(buf, "hello", 5) == 0,
          "second fd should read 'hello'");
    pos = sfs_getpos(fd2);
    CHECK(pos == 5, "fd2 getpos should be 5, got %zd", pos);

    /* fd1 is at pos 12 (after write), fd2 is at pos 5 — independent */
    pos = sfs_getpos(fd);
    CHECK(pos == 12, "fd1 getpos should still be 12, got %zd", pos);

    sfs_close(fd);
    sfs_close(fd2);

    /* write exactly BLOCK_DATA_SIZE bytes (boundary condition) */
    fd = sfs_open("boundary.txt");
    char fill[500];
    memset(fill, 'Z', 500);
    ssize_t nw = sfs_write(fd, fill, 500);
    CHECK(nw == 500, "write(500) returned %zd", nw);
    pos = sfs_getpos(fd);
    CHECK(pos == 500, "getpos after write(500) should be 500, got %zd", pos);
    p = sfs_seek(fd, -500);
    CHECK(p == 0, "seek(-500) from 500 should be 0, got %zd", p);
    nr = sfs_read(fd, buf, 3);
    CHECK(nr == 3 && memcmp(buf, "ZZZ", 3) == 0,
          "read after seek to 0: data mismatch");
    sfs_close(fd);

    sfs_unmount();
    return trace_ok;
}

/* ================================================================== */
/*  Category C — Concurrent Correctness                                */
/* ================================================================== */

#define NUM_THREADS 4

static void *thread_write_own_file(void *arg)
{
    int id = *(int *)arg;
    char fname[SFS_FILE_NAME_SIZE_LIMIT];
    snprintf(fname, sizeof fname, "thr%d.txt", id);

    int fd = sfs_open(fname);
    if (fd < 0)
        return (void *)(intptr_t)fd;

    char data[64];
    int len = snprintf(data, sizeof data, "thread-%d-payload", id);
    sfs_write(fd, data, (size_t)len);
    sfs_close(fd);

    fd = sfs_open(fname);
    if (fd < 0)
        return (void *)(intptr_t)fd;

    char buf[64] = {0};
    ssize_t nr = sfs_read(fd, buf, sizeof buf);
    sfs_close(fd);

    if (nr != len || memcmp(buf, data, (size_t)len) != 0)
        return (void *)(intptr_t)-1;
    return NULL;
}

/* C00: concurrent writes to separate files */
static int trace_C00(void)
{
    trace_ok = 1;
    sfs_format(CONC_DISK, disk_size());

    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_write_own_file, &ids[i]);
    }

    int ok = 1;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        void *ret;
        pthread_join(threads[i], &ret);
        if (ret != NULL)
            ok = 0;
    }
    CHECK(ok, "concurrent writes to separate files: data corruption");

    sfs_list_cookie cookie = NULL;
    char name[SFS_FILE_NAME_SIZE_LIMIT];
    int count = 0;
    while (sfs_list(&cookie, name, sizeof name) == 0)
        count++;
    CHECK(count == NUM_THREADS, "expected %d files, got %d", NUM_THREADS,
          count);

    sfs_unmount();
    unlink(CONC_DISK);
    return trace_ok;
}

static void *thread_read_shared(void *arg)
{
    int fd = sfs_open("shared.txt");
    if (fd < 0)
        return (void *)(intptr_t)fd;

    char buf[64] = {0};
    ssize_t nr = sfs_read(fd, buf, sizeof buf);
    sfs_close(fd);

    if (nr != 6 || memcmp(buf, "SHARED", 6) != 0)
        return (void *)(intptr_t)-1;
    return NULL;
}

/* C01: concurrent reads of same file */
static int trace_C01(void)
{
    trace_ok = 1;
    sfs_format(CONC_DISK, disk_size());

    int fd = sfs_open("shared.txt");
    sfs_write(fd, "SHARED", 6);
    sfs_close(fd);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_read_shared, NULL);

    int ok = 1;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        void *ret;
        pthread_join(threads[i], &ret);
        if (ret != NULL)
            ok = 0;
    }
    CHECK(ok, "concurrent reads of same file failed");

    sfs_unmount();
    unlink(CONC_DISK);
    return trace_ok;
}

struct rw_mix_arg
{
    int id;
    int do_write;
};

static void *thread_rw_mix(void *arg)
{
    struct rw_mix_arg *a = arg;
    char fname[SFS_FILE_NAME_SIZE_LIMIT];
    snprintf(fname, sizeof fname, "mix%d.txt", a->id);

    if (a->do_write)
    {
        int fd = sfs_open(fname);
        if (fd < 0)
            return (void *)(intptr_t)-1;
        char data[32];
        int len = snprintf(data, sizeof data, "data-%d", a->id);
        sfs_write(fd, data, (size_t)len);
        sfs_close(fd);
    }
    else
    {
        int fd = sfs_open(fname);
        if (fd < 0)
            return (void *)(intptr_t)-1;
        char buf[32];
        sfs_read(fd, buf, sizeof buf);
        sfs_close(fd);
    }
    return NULL;
}

static void *thread_open_close_storm(void *arg)
{
    for (int i = 0; i < 20; i++)
    {
        int fd = sfs_open("storm.txt");
        if (fd >= 0)
            sfs_close(fd);
    }
    return NULL;
}

/* C02: mixed r/w + open/close storm */
static int trace_C02(void)
{
    trace_ok = 1;

    /* Part 1: r/w mix on separate files */
    sfs_format(CONC_DISK, disk_size());

    struct rw_mix_arg args[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        args[i].id = i;
        args[i].do_write = (i % 2 == 0);
        pthread_create(&threads[i], NULL, thread_rw_mix, &args[i]);
    }

    int ok = 1;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        void *ret;
        pthread_join(threads[i], &ret);
        if (ret != NULL)
            ok = 0;
    }
    CHECK(ok, "concurrent r/w mix failed");
    sfs_unmount();
    unlink(CONC_DISK);

    /* Part 2: open/close storm on same file */
    sfs_format(CONC_DISK, disk_size());

    int fd = sfs_open("storm.txt");
    sfs_write(fd, "x", 1);
    sfs_close(fd);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_open_close_storm, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    sfs_list_cookie cookie = NULL;
    char name[SFS_FILE_NAME_SIZE_LIMIT];
    int count = 0;
    while (sfs_list(&cookie, name, sizeof name) == 0)
        count++;
    CHECK(count == 1, "storm: expected 1 file, got %d", count);

    sfs_unmount();
    unlink(CONC_DISK);
    return trace_ok;
}

/* ================================================================== */
/*  Performance Benchmark                                              */
/* ================================================================== */

#define PERF_THREADS 8
#define PERF_OPS_PER_THREAD 100
#define PERF_DISK "test_perf.img"

static void *perf_worker(void *arg)
{
    int id = *(int *)arg;
    char fname[SFS_FILE_NAME_SIZE_LIMIT];
    snprintf(fname, sizeof fname, "perf%d.txt", id);

    for (int i = 0; i < PERF_OPS_PER_THREAD; i++)
    {
        int fd = sfs_open(fname);
        if (fd < 0)
            continue;
        char data[64];
        int len = snprintf(data, sizeof data, "iter-%d-%d", id, i);
        sfs_write(fd, data, (size_t)len);
        sfs_seek(fd, -sfs_getpos(fd));
        char buf[64];
        sfs_read(fd, buf, sizeof buf);
        sfs_close(fd);
    }
    return NULL;
}

static double elapsed_sec(struct timespec *start, struct timespec *end)
{
    return (double)(end->tv_sec - start->tv_sec) +
           (double)(end->tv_nsec - start->tv_nsec) / 1e9;
}

/* Read baseline ops/sec from .perf_baseline.
   Returns the number, or -1.0 if the file is missing/unreadable/empty. */
static double read_baseline_ops(void)
{
    FILE *f = fopen(".perf_baseline", "r");
    if (!f) return -1.0;
    double ops = -1.0;
    if (fscanf(f, "%lf", &ops) != 1) ops = -1.0;
    fclose(f);
    if (ops <= 0.0) return -1.0;
    return ops;
}

/* Score student's ops/sec against the machine-calibrated baseline.
   ratio = student_ops / baseline_ops; thresholds mirror CMU's.
   If .perf_baseline is missing, fall back to legacy absolute thresholds
   with a warning so the grader still produces a number. */
static int score_perf_against_baseline(double student_ops)
{
    double baseline = read_baseline_ops();
    if (baseline < 0.0)
    {
        printf("  (.perf_baseline missing -- run `make baseline` to calibrate; "
               "using legacy absolute thresholds)\n");
        if (student_ops < 1000)  return 0;
        if (student_ops < 3000)  return 3;
        if (student_ops < 6000)  return 5;
        if (student_ops < 10000) return 7;
        if (student_ops < 15000) return 9;
        return 10;
    }

    double ratio = student_ops / baseline;
    printf("  Baseline throughput: %.0f ops/sec (from .perf_baseline)\n",
           baseline);
    printf("  Ratio (student / baseline): %.2fx\n", ratio);

    if (ratio >= 0.90) return 10;
    if (ratio >= 0.70) return 9;
    if (ratio >= 0.50) return 7;
    if (ratio >= 0.30) return 5;
    if (ratio >= 0.15) return 3;
    return 0;
}

static double run_perf_benchmark_raw(void)
{
    sfs_format(PERF_DISK, disk_size());

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    pthread_t threads[PERF_THREADS];
    int ids[PERF_THREADS];
    for (int i = 0; i < PERF_THREADS; i++)
    {
        ids[i] = i;
        pthread_create(&threads[i], NULL, perf_worker, &ids[i]);
    }
    for (int i = 0; i < PERF_THREADS; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    sfs_unmount();
    unlink(PERF_DISK);

    double secs = elapsed_sec(&t0, &t1);
    int total_ops = PERF_THREADS * PERF_OPS_PER_THREAD;
    return (double)total_ops / secs;
}

static int run_perf_benchmark(void)
{
    double ops_per_sec = run_perf_benchmark_raw();
    int total_ops = PERF_THREADS * PERF_OPS_PER_THREAD;
    printf("  Student throughput: %.0f ops/sec (%d total ops)\n",
           ops_per_sec, total_ops);
    return score_perf_against_baseline(ops_per_sec);
}

/* ================================================================== */
/*  ThreadSanitizer race detection (auto-compiled and run)             */
/* ================================================================== */

static int run_tsan_check(void)
{
    printf("\nRace Detection (ThreadSanitizer):\n");

    const char *tsan_bin = "./test-sfs-tsan";
    const char *tsan_log = "tsan_output.log";

    char compile_cmd[512];
    snprintf(compile_cmd, sizeof compile_cmd,
             "gcc -std=c11 -g -fsanitize=thread -pthread -D_GNU_SOURCE=1 "
             "-o %s test-sfs.c sfs-disk.c sfs-support.c 2>&1",
             tsan_bin);

    int rc = system(compile_cmd);
    if (rc != 0)
    {
        printf("  (skipped — TSan compilation failed, gcc may not support "
               "-fsanitize=thread)\n");
        return 1; // don't penalize if TSan unavailable
    }

    char run_cmd[512];
    snprintf(run_cmd, sizeof run_cmd,
             "%s --tsan-only > %s 2>&1",
             tsan_bin, tsan_log);

    rc = system(run_cmd);
    unlink(tsan_bin);

    int race_found = 0;
    FILE *f = fopen(tsan_log, "r");
    if (f)
    {
        char line[256];
        while (fgets(line, sizeof line, f))
        {
            if (strstr(line, "WARNING: ThreadSanitizer: data race"))
            {
                race_found = 1;
                break;
            }
        }
        fclose(f);
    }

    if (!race_found && WIFEXITED(rc) && WEXITSTATUS(rc) != 0)
        race_found = 1;

    unlink(tsan_log);
    unlink("test_conc.img");

    if (race_found)
    {
        printf("  DATA RACE DETECTED — Category C score set to 0\n");
        printf("  Run with TSan manually for details:\n");
        printf("    gcc -std=c11 -g -fsanitize=thread -pthread -D_GNU_SOURCE=1 "
               "\\\n");
        printf("        -o test-sfs-tsan test-sfs.c sfs-disk.c sfs-support.c\n");
        printf("    ./test-sfs-tsan --tsan-only\n");
        return 0;
    }

    printf("  No races detected                          PASS\n");
    return 1;
}

/* ================================================================== */
/*  Main — run all traces and print scoreboard                         */
/* ================================================================== */

typedef int (*trace_fn)(void);

struct trace_entry
{
    const char *id;
    const char *name;
    trace_fn fn;
};

#define TRACE_TIMEOUT_SEC 30

/* Run `fn` in a forked child with a TRACE_TIMEOUT_SEC wall-clock limit.
   Child exit code:
     0 = trace passed
     1 = trace failed (a CHECK inside it failed and set trace_ok = 0)
   Parent handles SIGKILL-on-timeout and reports TIMEOUT in stderr.
   Returns 1 if the trace passed, 0 otherwise. */
static int run_trace_with_timeout(const char *id, trace_fn fn)
{
    fflush(stdout);
    fflush(stderr);

    pid_t pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "fork() failed: %s\n", strerror(errno));
        return 0;
    }
    if (pid == 0)
    {
        int ok = fn();
        _exit(ok ? 0 : 1);
    }

    /* Parent: poll waitpid up to TRACE_TIMEOUT_SEC seconds. */
    int elapsed_ms = 0;
    const int step_ms = 100;
    int status = 0;
    while (elapsed_ms < TRACE_TIMEOUT_SEC * 1000)
    {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid) break;
        if (r < 0)
        {
            fprintf(stderr, "waitpid: %s\n", strerror(errno));
            return 0;
        }
        struct timespec ts = { .tv_sec = 0, .tv_nsec = step_ms * 1000000L };
        nanosleep(&ts, NULL);
        elapsed_ms += step_ms;
    }

    if (elapsed_ms >= TRACE_TIMEOUT_SEC * 1000)
    {
        fprintf(stderr, "       -> TIMEOUT: trace %s exceeded %d seconds "
                        "(deadlock suspected); killing child\n",
                id, TRACE_TIMEOUT_SEC);
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        unlink(CONC_DISK);
        return 0;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status) == 0 ? 1 : 0;
    if (WIFSIGNALED(status))
    {
        fprintf(stderr, "       -> CRASH: trace %s killed by signal %d\n",
                id, WTERMSIG(status));
        return 0;
    }
    return 0;
}

static int run_category(const char *label, struct trace_entry *traces, int n)
{
    printf("\nCategory %s:\n", label);
    /* Only concurrent-correctness traces need the fork+timeout wrapper. */
    int use_timeout = (label[0] == 'C');
    int passed = 0;
    for (int i = 0; i < n; i++)
    {
        int ok = use_timeout
                 ? run_trace_with_timeout(traces[i].id, traces[i].fn)
                 : traces[i].fn();
        passed += ok;
        printf("  %s %-24s %s  [%d/1]\n", traces[i].id, traces[i].name,
               ok ? "PASS" : "FAIL", ok);
    }
    printf("  Subtotal: %d/%d\n", passed, n);
    return passed;
}

int main(int argc, char *argv[])
{
    /* When invoked with --tsan-only, run only the C traces and exit.
       TSan will report races via its own runtime; exit code 66 signals
       that the C traces themselves failed. */
    if (argc > 1 && strcmp(argv[1], "--tsan-only") == 0)
    {
        struct trace_entry cat_c[] = {
            {"C00", "separate_files", trace_C00},
            {"C01", "read_same_file", trace_C01},
            {"C02", "rw_mix_storm", trace_C02},
        };
        int c = 0;
        for (int i = 0; i < 3; i++)
            c += cat_c[i].fn();
        unlink(DISK_NAME);
        return (c == 3) ? 0 : 66;
    }

    /* --perf-only: used by test-sfs-baseline to print baseline ops/sec.
       Writes a single decimal number (no scoring, no banners) to stdout. */
    if (argc > 1 && strcmp(argv[1], "--perf-only") == 0)
    {
        double baseline_ops = run_perf_benchmark_raw();
        printf("%.2f\n", baseline_ops);
        unlink(DISK_NAME);
        return 0;
    }

    printf("========================================\n");
    printf("        SFS Lab Autograder\n");
    printf("========================================\n");

    struct trace_entry cat_a[] = {
        {"A00", "format_mount", trace_A00},
        {"A01", "open_close_rw", trace_A01},
        {"A02", "getpos", trace_A02},
        {"A03", "seek", trace_A03},
        {"A04", "rename", trace_A04},
    };

    struct trace_entry cat_b[] = {
        {"B00", "remove_list", trace_B00},
        {"B01", "multi_block_seek", trace_B01},
        {"B02", "edge_cases", trace_B02},
    };

    struct trace_entry cat_c[] = {
        {"C00", "separate_files", trace_C00},
        {"C01", "read_same_file", trace_C01},
        {"C02", "rw_mix_storm", trace_C02},
    };

    int a = run_category("A (Feature Tests)", cat_a, 5);
    int b = run_category("B (Sequential Correctness)", cat_b, 3);
    int c = run_category("C (Concurrent Correctness)", cat_c, 3);

    /* TSan race detection: if races found, C score becomes 0 */
    int tsan_ok = 1;
    if (c > 0)
        tsan_ok = run_tsan_check();
    if (!tsan_ok)
        c = 0;

    int correctness = a + b + c;
    printf("\nCorrectness: %d/11\n", correctness);

    printf("\nPerformance:\n");
    int perf = 0;
    if (correctness < 11)
    {
        printf("  (skipped — correctness tests must all pass first)\n");
        printf("  Score: 0/10\n");
    }
    else
    {
        /* Perf benchmark also needs deadlock protection — 60s budget
           (longer than a Category C trace because slower machines may
           legitimately take more time on the full workload). */
        fflush(stdout); fflush(stderr);
        pid_t pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "fork() for perf failed: %s\n", strerror(errno));
            perf = 0;
        }
        else if (pid == 0)
        {
            int score = run_perf_benchmark();
            _exit(score);
        }
        else
        {
            int status = 0;
            int elapsed_ms = 0;
            const int step_ms = 100;
            const int perf_budget_ms = 60 * 1000;
            while (elapsed_ms < perf_budget_ms)
            {
                pid_t r = waitpid(pid, &status, WNOHANG);
                if (r == pid) break;
                if (r < 0)
                {
                    fprintf(stderr, "waitpid(perf): %s\n", strerror(errno));
                    break;
                }
                struct timespec ts = {
                    .tv_sec = 0, .tv_nsec = step_ms * 1000000L
                };
                nanosleep(&ts, NULL);
                elapsed_ms += step_ms;
            }
            if (elapsed_ms >= perf_budget_ms)
            {
                fprintf(stderr,
                        "  TIMEOUT: perf benchmark exceeded %ds; killing\n",
                        perf_budget_ms / 1000);
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                unlink(PERF_DISK);
                perf = 0;
            }
            else if (WIFEXITED(status))
            {
                perf = WEXITSTATUS(status);
            }
            else
            {
                perf = 0;
            }
        }
        printf("  Score: %d/10\n", perf);
    }

    int total = correctness + perf;
    printf("\n----------------------------------------\n");
    printf("  Total: %d/21  (+ up to 4 style pts)\n", total);
    printf("========================================\n");

    unlink(DISK_NAME);
    return (correctness == 11) ? 0 : 1;
}
