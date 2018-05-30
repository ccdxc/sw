
/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <libgen.h>

static const char *progname;

static void __attribute__((noreturn))
exit_usage(void)
{
    fprintf(stderr, "usage: pollregread ADDR REPS\n"
                    "       pollregwrite ADDR DATA REPS\n"
                    "       readreg ADDR\n"
                    "       writereg ADDR VAL\n"
                    "       memread [--uncached] ADDR\n"
                    "       memwrite [--uncached] ADDR VAL\n"
                    "       memcat [--uncached] ADDR LEN\n");
    exit(1);
}

typedef enum {
    MATTR_DEVICE,
    MATTR_UNCACHED,
    MATTR_CACHED
} mattr_t;

static void *
get_mempage(off_t offs, mattr_t mattr)
{
    const char path[] = "/dev/mem";
    int flags = O_RDWR;
    void *p;
    int fd;

    // Only cached/uncached when using /dev/mem
    // Even then, cached is only valid for addresses the kernel knows about
    if (mattr == MATTR_DEVICE || mattr == MATTR_UNCACHED) {
        flags |= O_SYNC;
    }
    fd = open(path, flags, 0);
    if (fd < 0) {
        perror(path);
        return NULL;
    }
    p = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, offs);
    close(fd);
    if (p == (void *)-1) {
        perror("mmap");
        return NULL;
    }
    return p;
}

static volatile uint32_t *
mem_getptr(uint64_t addr, mattr_t mattr)
{
    int pgsz = getpagesize();
    off_t base;
    int offs;
    void *p;

    base = addr & -pgsz;
    offs = addr & (pgsz - 1);
    p = get_mempage(base, mattr);
    if (p == NULL) {
        return NULL;
    }
    return (volatile uint32_t *)((char *)p + offs);
}

static int
mem_read32(uint64_t addr, mattr_t mattr, uint32_t *valp)
{
    int pgsz = getpagesize();
    off_t base;
    int offs;
    void *p;

    base = addr & -pgsz;
    offs = addr & (pgsz - 1);
    p = get_mempage(base, mattr);
    if (p == NULL) {
        return -1;
    }
    *valp = *(volatile uint32_t *)((char *)p + offs);
    return 0;
}

static int
mem_write32(uint64_t addr, mattr_t mattr, uint32_t val)
{
    int pgsz = getpagesize();
    off_t base;
    int offs;
    void *p;

    base = addr & -pgsz;
    offs = addr & (pgsz - 1);
    p = get_mempage(base, mattr);
    if (p == NULL) {
        return -1;
    }
    *(volatile uint32_t *)((char *)p + offs) = val;
    return 0;
}

static int
cm_pollregread(int argc, char *argv[])
{
    uint32_t val, last_val, reps, i;
    volatile uint32_t *ptr;
    uint64_t addr;
    char *p;

    if (argc < 3) {
        exit_usage();
    }
    addr = strtoull(argv[1], &p, 0);
    if (*p != '\0' || (addr & 0x3)) {
        exit_usage();
    }
    ptr = mem_getptr(addr, MATTR_DEVICE);
    if (ptr == NULL) {
        return -1;
    }
    reps = strtoul(argv[2], &p, 0);
    if (*p != '\0' || (reps == 0)) {
        exit_usage();
    }
    last_val = 0;
    for (i = 0; (reps == 0) || (i < reps); i++) {
        val = *ptr;
        if (i == 0 || val != last_val) {
            printf("[%u] %08x\n", i, val);
        }
        last_val = val;
    }
    return 0;
}

static int
cm_pollregwrite(int argc, char *argv[])
{
    uint32_t val, reps, i;
    volatile uint32_t *ptr;
    uint64_t addr;
    char *p;

    if (argc < 4) {
        exit_usage();
    }
    addr = strtoull(argv[1], &p, 0);
    if (*p != '\0' || (addr & 0x3)) {
        exit_usage();
    }
    val = strtoul(argv[2], &p, 0);
    if (*p != '\0') {
        exit_usage();
    }
    ptr = mem_getptr(addr, MATTR_DEVICE);
    if (ptr == NULL) {
        return -1;
    }
    reps = strtoul(argv[3], &p, 0);
    if (*p != '\0' || (reps == 0)) {
        exit_usage();
    }
    for (i = 0; (reps == 0) || (i < reps); i++) {
        *ptr = val;
    }
    return 0;
}

static int
do_read(int argc, char *argv[], mattr_t mattr)
{
    uint64_t addr;
    uint32_t val;
    char *p;

    if (argc < 2) {
        exit_usage();
    }
    addr = strtoull(argv[1], &p, 0);
    if (*p != '\0' || (addr & 0x3)) {
        exit_usage();
    }
    if (mem_read32(addr, mattr, &val) < 0) {
        return -1;
    }
    printf("0x%08x\n", val);
    return 0;
}

static int
do_write(int argc, char *argv[], mattr_t mattr)
{
    uint64_t addr;
    uint32_t val;
    char *p;

    if (argc < 3) {
        exit_usage();
    }
    addr = strtoull(argv[1], &p, 0);
    if (*p != '\0' || (addr & 0x3)) {
        exit_usage();
    }
    val = strtoul(argv[2], &p, 0);
    if (*p != '\0') {
        exit_usage();
    }
    return mem_write32(addr, mattr, val);
}

static int
cm_readreg(int argc, char *argv[])
{
    return do_read(argc, argv, MATTR_DEVICE);
}

static int
cm_writereg(int argc, char *argv[])
{
    return do_write(argc, argv, MATTR_DEVICE);
}

static int
cm_readmem(int argc, char *argv[])
{
    mattr_t mattr = MATTR_CACHED;

    if (argc > 1 && strcmp(argv[1], "--uncached") == 0) {
        mattr = MATTR_UNCACHED;
        --argc;
        ++argv;
    }
    return do_read(argc, argv, mattr);
}

static int
cm_writemem(int argc, char *argv[])
{
    mattr_t mattr = MATTR_CACHED;

    if (argc > 1 && strcmp(argv[1], "--uncached") == 0) {
        mattr = MATTR_UNCACHED;
        --argc;
        ++argv;
    }
    return do_write(argc, argv, mattr);
}

static int
cm_memcat(int argc, char *argv[])
{
    mattr_t mattr = MATTR_CACHED;
    uint32_t len, val;
    uint64_t addr;
    char *p;

    if (argc > 1 && strcmp(argv[1], "--uncached") == 0) {
        mattr = MATTR_UNCACHED;
        --argc;
        ++argv;
    }
    if (argc < 3) {
        exit_usage();
    }
    addr = strtoull(argv[1], &p, 0);
    if (*p != '\0' || (addr & 0x3)) {
        exit_usage();
    }
    len = strtoul(argv[2], &p, 0);
    if (*p != '\0') {
        exit_usage();
    }
    len = ((len + 3) & -4) >> 2;
    while (len--) {
        if (mem_read32(addr, mattr, &val) < 0) {
            return -1;
        }
        if (fwrite(&val, sizeof (val), 1, stdout) != 1) {
            perror("fwrite");
            return -1;
        }
        addr += 4;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    progname = basename(argv[0]);
    int ret = 0;

    if (strcmp(progname, "pollregread") == 0) {
        ret = cm_pollregread(argc, argv);
    } else if (strcmp(progname, "pollregwrite") == 0) {
        ret = cm_pollregwrite(argc, argv);
    } else if (strcmp(progname, "readreg") == 0) {
        ret = cm_readreg(argc, argv);
    } else if (strcmp(progname, "writereg") == 0) {
        ret = cm_writereg(argc, argv);
    } else if (strcmp(progname, "readmem") == 0) {
        ret = cm_readmem(argc, argv);
    } else if (strcmp(progname, "writemem") == 0) {
        ret = cm_writemem(argc, argv);
    } else if (strcmp(progname, "memcat") == 0) {
        ret = cm_memcat(argc, argv);
    } else {
        exit_usage();
    }
    return ret;
}
