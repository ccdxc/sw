/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/io.h>

#include "pci.h"

typedef struct tdev {
    struct pci_dev *pcidev;
    void *memvad;
    void *memvan;
    void *memvai;
    void *io_vad;
    void *io_van;
    void *io_vai;
    unsigned int cfgd;
    unsigned int cfgn;
    unsigned int cfgi;
} tdev_t;

static tdev_t tdev;
static u_int8_t testdata[0x1000];
static u_int8_t readdata[0x1000];
static int verbose;
static int testmode;
static int testiters = 1;
static int simio;
static int showdots;

static void
usage(void)
{
    fprintf(stderr,
"Usage: pciestress [-d <ven:dev>][-s <slot>][-vI] [test [args...]]\n"
"    -d <ven:dev>   select target dev like lspci -d <ven:dev>\n"
"    -s <slot>      select target dev like lspci -s <slot>\n"
"    -S             simulate I/O,cfg space access\n"
"    -v             verbose output\n"
            );
}

static void
logv(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
static void
logv(const char *fmt, ...)
{
    va_list ap;

    if (verbose) {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
}

static void *
mapfile_region(const char *file, 
               const off_t off, const size_t sz, const int oflags)
{
    int fd, prot, mapflags;
    const int pagesize = getpagesize();
    const off_t aligned_off = off & ~(pagesize - 1);
    size_t aligned_sz = roundup(off + sz, pagesize) - aligned_off;
    void *va;

    fd = open(file, oflags, 0666);
    if (fd < 0) {
        return NULL;
    }
    if (oflags & O_CREAT) {
        char z = 0;

        /* grow file to proper size */
        lseek(fd, aligned_sz - 1, SEEK_SET);
        if (write(fd, &z, 1) != 1) {
            (void)close(fd);
            return NULL;
        }
    }
    prot = PROT_READ | PROT_WRITE;
    mapflags = MAP_SHARED;
    va = mmap(NULL, aligned_sz, prot, mapflags, fd, aligned_off);
    (void)close(fd);
    if (va == (void *)-1) {
        return NULL;
    }
    return va + (off & (pagesize - 1));
}

static void *
mapfd(int fd, const off_t off, const size_t sz)
{
    const int prot = PROT_READ | PROT_WRITE;
    const int mapflags = MAP_SHARED;
    void *va = mmap(NULL, sz, prot, mapflags, fd, off);
    assert(va != (void *)-1);
    return va;
}

static void *
map_address(u_int64_t pa, size_t sz)
{
    return mapfile_region("/dev/mem", pa, sz, O_RDWR);
}

static int
opentdev(tdev_t *td)
{
    struct pci_dev *pd = td->pcidev;
    struct pci_cap *cap;
    u_int64_t pa;
    size_t sz;

    pci_fill_info(pd, PCI_FILL_BASES | PCI_FILL_SIZES);

    /* map mem bar */
    pa = pd->base_addr[0] & PCI_ADDR_MEM_MASK;
    sz = pd->size[0];
    td->memvad = map_address(pa, sz);

    /* save i/o bar address */
    pa = pd->base_addr[2] & PCI_ADDR_IO_MASK;
    td->io_vad = (void *)(uintptr_t)pa;

    if (td->memvad == NULL || td->io_vad == NULL) {
        perror("map_address");
        return -1;
    }

    td->memvan = td->memvad + 0x1000;
    td->memvai = td->memvad + 0x2000;

    td->io_van = td->io_vad + 0x20;
    td->io_vai = td->io_vad + 0x40;

    if (0) {
        cap = pci_find_cap(pd, PCI_CAP_ID_VNDR, PCI_CAP_NORMAL);
        if (cap == NULL) {
            fprintf(stderr, "no vendor capability\n");
            return -1;
        }

        td->cfgd = cap->addr + 4;
        td->cfgn = td->cfgd + 0x8;
        td->cfgi = td->cfgd + 0x10;
    }

    return 0;
}

static int
opentestdev(tdev_t *td)
{
    const char *home = getenv("HOME");
    const int oflags = O_RDWR | O_CREAT;
    char path[128];
    char z = 0;
    int fd;
    struct stat sb;
    void *va;
    const size_t sz = 0x1000;

    snprintf(path, sizeof(path), "%s/.pciestress", home);
    fd = open(path, oflags, 0666);
    assert(fd >= 0);

    /* if new/short file, grow to required size for mmap */
    if (fstat(fd, &sb) < 0 || sb.st_size < sz) {
        lseek(fd, sz - 1, SEEK_SET);
        if (write(fd, &z, 1) != 1) assert(0);
    }

    /* map the entire file */
    va = mapfd(fd, 0, sz);
    (void)close(fd);

    td->memvad = va;
    td->io_vad = va;

    if (td->memvad == NULL || td->io_vad == NULL) {
        perror("map_address");
        return -1;
    }

    td->memvan = td->memvad;
    td->memvai = td->memvad;

    td->io_van = td->io_vad;
    td->io_vai = td->io_vad;

    return 0;
}

/*****************************************************************
 * mem rd/wr
 */

#define memrwX(N, TYPE) \
static size_t \
memrw##N(void *dst, void *src, const size_t sz) \
{ \
    volatile TYPE *s = src; \
    volatile TYPE *d = dst; \
    const size_t n = sz / sizeof(TYPE); \
    int i; \
\
    for (i = 0; i < n; i++) { \
        *d++ = *s++; \
    } \
    return n; \
}

memrwX(1, u_int8_t);    /* memrw1() */
memrwX(2, u_int16_t);   /* memrw2() */
memrwX(4, u_int32_t);   /* memrw4() */
memrwX(8, u_int64_t);   /* memrw8() */

#define memrdX(N, TYPE) \
static size_t \
memrd##N(void *va, const size_t sz) \
{ \
    return memrw##N(readdata, va, sz); \
}

#define memwrX(N, TYPE) \
static size_t \
memwr##N(void *va, const size_t sz) \
{ \
    return memrw##N(va, testdata, sz); \
}

memrdX(1, u_int8_t);    /* memrd1() */
memrdX(2, u_int16_t);   /* memrd2() */
memrdX(4, u_int32_t);   /* memrd4() */
memrdX(8, u_int64_t);   /* memrd8() */

memwrX(1, u_int8_t);    /* memwr1() */
memwrX(2, u_int16_t);   /* memwr2() */
memwrX(4, u_int32_t);   /* memwr4() */
memwrX(8, u_int64_t);   /* memwr8() */


/*****************************************************************
 * io rd/wr
 */

#define iordX(N, TYPE, IN) \
static size_t \
iord##N(void *va, const size_t sz) \
{ \
    volatile TYPE *s = va; \
    TYPE *d = (void *)readdata; \
    const size_t n = sz / sizeof(*d); \
    int i; \
\
    if (simio) { \
        return memrd##N(va, sz); \
    } else { \
        for (i = 0; i < n; i++) {   \
            *d++ = IN((uintptr_t)s++); \
        } \
    } \
    return n; \
}

#define iowrX(N, TYPE, OUT) \
static size_t \
iowr##N(void *va, const size_t sz) \
{ \
    TYPE *s = (void *)testdata; \
    volatile TYPE *d = va; \
    const size_t n = sz / sizeof(*d); \
    int i; \
\
    if (simio) { \
        return memwr##N(va, sz); \
    } else { \
        for (i = 0; i < n; i++) {  \
            OUT(*s++, (uintptr_t)d++); \
        } \
    } \
    return n; \
}

iordX(1, u_int8_t,  inb);       /* iord1() */
iordX(2, u_int16_t, inw);       /* iord2() */
iordX(4, u_int32_t, inl);       /* iord4() */

iowrX(1, u_int8_t,  outb);      /* iowr1() */
iowrX(2, u_int16_t, outw);      /* iowr2() */
iowrX(4, u_int32_t, outl);      /* iowr4() */

/*****************************************************************
 * cfg rd/wr
 */

#define cfgrdX(N, TYPE, BWL) \
static size_t \
cfgrd##N(const unsigned int pos, const size_t sz) \
{ \
    TYPE *d = (void *)readdata; \
    const size_t n = sz / sizeof(TYPE); \
    int i; \
\
    if (simio) { \
        return memrd##N(tdev.memvad + pos, sz); \
    } else { \
        for (i = 0; i < n; i++) {   \
            *d++ = pci_read_##BWL(tdev.pcidev, pos + i * sizeof(TYPE)); \
        } \
    } \
    return n; \
}

#define cfgwrX(N, TYPE, BWL) \
static size_t \
cfgwr##N(const unsigned int pos, const size_t sz) \
{ \
    TYPE *s = (void *)testdata; \
    const size_t n = sz / sizeof(TYPE); \
    int i; \
\
    if (simio) { \
        return memwr##N(tdev.memvad + pos, sz); \
    } else { \
        for (i = 0; i < n; i++) {  \
            pci_write_##BWL(tdev.pcidev, pos + i * sizeof(TYPE), *s++); \
        } \
    } \
    return n; \
}

cfgrdX(1, u_int8_t,  byte);     /* cfgrd1() */
cfgrdX(2, u_int16_t, word);     /* cfgrd2() */
cfgrdX(4, u_int32_t, long);     /* cfgrd4() */

cfgwrX(1, u_int8_t,  byte);     /* cfgwr1() */
cfgwrX(2, u_int16_t, word);     /* cfgwr2() */
cfgwrX(4, u_int32_t, long);     /* cfgwr4() */

/*****************************************************************
 * readdata, testdata
 */

/*
 * verify rwdata contents match expected testdata contents
 */
static int
readdata_verify(const size_t sz)
{
    u_int8_t *t = testdata;
    u_int8_t *r = readdata;
    int i, errs = 0;

    /*
     * Expect these to match so use memcmp for efficient compare.
     * If compare fails walk through bytes to report exactly what
     * was wrong.
     */
    if (memcmp(t, r, sz)) {
        for (i = 0; i < sz; i++) {
            if (t[i] != r[i]) {
                logv("ERROR: offset %d expected 0x%02x got 0x%02x\n",
                     i, t[i], r[i]);
                errs++;
            }
        }
    }
    return errs;
}

static void
testdata_init(void)
{
    u_int16_t *w = (u_int16_t *)testdata;
    const size_t nw = sizeof(testdata) / sizeof(*w);
    int i;

    for (i = 0; i < nw; i++) {
        *w++ = i;
    }
}

static void
data_randomize(void *va, const size_t sz)
{
    long int *lp = va;
    const size_t n = sz / sizeof(*lp);
    int i;

    for (i = 0; i < n; i++) {
        *lp++ = random();
    }
}

static void
readdata_randomize(void)
{
    data_randomize(readdata, sizeof(readdata));
}

static void
bardata_randomize(const size_t sz)
{
    readdata_randomize();
    memrw8(tdev.memvad, readdata, sz);
}

/*
 * Prepare buffers for a read test.
 * Send expected data into the device, using the most efficient
 * method, direct memory 8-byte writes.
 */
static void
read_test_prepare(void)
{
    memrw8(tdev.memvad, testdata, 0x1000);
}

/*
 * Load random data into the read buffer.
 */
static void
read_test_reset(void)
{
    readdata_randomize();
}

static int
read_test_verify(const size_t sz)
{
    return readdata_verify(sz);
}

/*
 * Prepare buffers for a write test.
 */
static void
write_test_prepare(void)
{
}

/*
 * Send random data into device.
 */
static void
write_test_reset(void)
{
    bardata_randomize(0x1000);
}

/*
 * We have completed a "write" test which transferred "testdata"
 * to device for "sz" bytes.  Verify the writes all landed as expected
 * by reading the device into readdata, then verifying readdata matches
 * testdata.
 */
static int
write_test_verify_va(void *va, const size_t sz)
{
    memrd8(va, sz);
    return readdata_verify(sz);
}

/*
 * Write test verify using efficient devic reads (direct mem).
 */
static int
write_test_verify(const size_t sz)
{
    return write_test_verify_va(tdev.memvad, sz);
}

/*****************************************************************
 * tests
 */

static int
memrdseq(const char *testname, void *va, const size_t sz)
{
    int r = 0;

    logv("%s\n", testname);

    read_test_prepare();

    logv("  mem rd 1-byte\n");
    read_test_reset();
    memrd1(va, sz);
    r += read_test_verify(sz);

    logv("  mem rd 2-byte\n");
    read_test_reset();
    memrd2(va, sz);
    r += read_test_verify(sz);

    logv("  mem rd 4-byte\n");
    read_test_reset();
    memrd4(va, sz);
    r += read_test_verify(sz);

    logv("  mem rd 8-byte\n");
    read_test_reset();
    memrd8(va, sz);
    r += read_test_verify(sz);

    return r;
}

static int
memwrseq(const char *testname, void *va, const size_t sz)
{
    int r = 0;

    logv("%s\n", testname);

    write_test_prepare();

    logv("  mem wr 1-byte\n");
    write_test_reset();
    memwr1(va, sz);
    r += write_test_verify(sz);

    logv("  mem wr 2-byte\n");
    write_test_reset();
    memwr2(va, sz);
    r += write_test_verify(sz);

    logv("  mem wr 4-byte\n");
    write_test_reset();
    memwr4(va, sz);
    r += write_test_verify(sz);

    logv("  mem wr 8-byte\n");
    write_test_reset();
    memwr8(va, sz);
    r += write_test_verify(sz);

    return r;
}

static int
iordseq(const char *testname, void *va, const size_t sz)
{
    int r = 0;

    logv("%s\n", testname);

    read_test_prepare();

    logv("  io  rd 1-byte\n");
    read_test_reset();
    iord1(va, sz);
    r += read_test_verify(sz);

    logv("  io  rd 2-byte\n");
    read_test_reset();
    iord2(va, sz);
    r += read_test_verify(sz);

    logv("  io  rd 4-byte\n");
    read_test_reset();
    iord4(va, sz);
    r += read_test_verify(sz);

    return r;
}

static int
iowrseq(const char *testname, void *va, const size_t sz)
{
    int r = 0;

    logv("%s\n", testname);

    write_test_prepare();

    logv("  io  wr 1-byte\n");
    write_test_reset();
    iowr1(va, sz);
    r += write_test_verify(sz);

    logv("  io  wr 2-byte\n");
    write_test_reset();
    iowr2(va, sz);
    r += write_test_verify(sz);

    logv("  io  wr 4-byte\n");
    write_test_reset();
    iowr4(va, sz);
    r += write_test_verify(sz);

    return r;
}

static int
cfgrdseq(const char *testname, const unsigned int pos, const size_t sz)
{
    int r = 0;

    logv("%s\n", testname);

    read_test_prepare();

    logv("  cfg rd 1-byte\n");
    read_test_reset();
    cfgrd1(pos, sz);
    r += read_test_verify(sz);

    logv("  cfg rd 2-byte\n");
    read_test_reset();
    cfgrd2(pos, sz);
    r += read_test_verify(sz);

    logv("  cfg rd 4-byte\n");
    read_test_reset();
    cfgrd4(pos, sz);
    r += read_test_verify(sz);

    return r;
}

static int
cfgwrseq(const char *testname, const unsigned int pos, const size_t sz)
{
    int r = 0;

    logv("%s\n", testname);

    write_test_prepare();

    logv("  cfg wr 1-byte\n");
    write_test_reset();
    cfgwr1(pos, sz);
    r += write_test_verify(sz);

    logv("  cfg wr 2-byte\n");
    write_test_reset();
    cfgwr2(pos, sz);
    r += write_test_verify(sz);

    logv("  cfg wr 4-byte\n");
    write_test_reset();
    cfgwr4(pos, sz);
    r += write_test_verify(sz);

    return r;
}

static int
t_memrdseq_direct(int argc, char *argv[])
{
    return memrdseq("memrdseq_direct", tdev.memvad, 0x1000);
}

static int
t_memwrseq_direct(int argc, char *argv[])
{
    return memwrseq("memwrseq_direct", tdev.memvad, 0x1000);
}

static int
t_memrdseq_notify(int argc, char *argv[])
{
    return memrdseq("memrdseq_notify", tdev.memvan, 0x1000);
}

static int
t_memwrseq_notify(int argc, char *argv[])
{
    return memwrseq("memwrseq_notify", tdev.memvan, 0x1000);
}

static int
t_memrdseq_indirect(int argc, char *argv[])
{
    return memrdseq("memrdseq_indirect", tdev.memvai, 0x1000);
}

static int
t_memwrseq_indirect(int argc, char *argv[])
{
    return memwrseq("memwrseq_indirect", tdev.memvai, 0x1000);
}

static int
t_iordseq_direct(int argc, char *argv[])
{
    return iordseq("iordseq_direct", tdev.io_vad, 0x20);
}

static int
t_iowrseq_direct(int argc, char *argv[])
{
    return iowrseq("iowrseq_direct", tdev.io_vad, 0x20);
}

static int
t_iordseq_notify(int argc, char *argv[])
{
    return iordseq("iordseq_notify", tdev.io_van, 0x20);
}

static int
t_iowrseq_notify(int argc, char *argv[])
{
    return iowrseq("iowrseq_notify", tdev.io_van, 0x20);
}

static int
t_iordseq_indirect(int argc, char *argv[])
{
    return iordseq("iordseq_indirect", tdev.io_vai, 0x20);
}

static int
t_iowrseq_indirect(int argc, char *argv[])
{
    return iowrseq("iowrseq_indirect", tdev.io_vai, 0x20);
}

static int
t_cfgrdseq_direct(int argc, char *argv[])
{
    return cfgrdseq("cfgrdseq_direct", tdev.cfgd, 0x8);
}

static int
t_cfgwrseq_direct(int argc, char *argv[])
{
    return cfgwrseq("cfgwrseq_direct", tdev.cfgd, 0x8);
}

static int
t_simpletest(int argc, char *argv[])
{
    void *va = tdev.memvad;
    size_t sz = 0x1000;
    size_t us = 2000;
    int opt, wronly = 0, r = 0;
    int wrflags = 0;

    optind = 0;
    while ((opt = getopt(argc, argv, "1248s:t:w")) != -1) {
        switch (opt) {
        case '1':
            wrflags |= 1;
            break;
        case '2':
            wrflags |= 2;
            break;
        case '4':
            wrflags |= 4;
            break;
        case '8':
            wrflags |= 8;
            break;
        case 's':
            sz = strtoul(optarg, NULL, 0);
            break;
        case 't':
            us = strtoul(optarg, NULL, 0);
            break;
        case 'w':
            wronly = 1;
            break;
        default:
            fprintf(stderr, "unexpected option: %c\n", opt);
            return 1;
        }
    }

    if (wrflags == 0) wrflags = 0xf;

    logv("simpletest %s sz=%ld us=%ld wrflags 0x%x\n",
         wronly ? "wronly" : "rw", sz, us, wrflags);

    testdata_init();

    if (!wronly) memrd8(testdata, sz);

    if (wrflags & 0x1) {
        logv("  mem wr 1-byte\n");
        memwr1(va, sz);
        if (us) usleep(us);
        if (!wronly) {
            memrd8(va, sz);
            r += readdata_verify(sz);
        }
    }

    if (wrflags & 0x2) {
        logv("  mem wr 2-byte\n");
        memwr2(va, sz);
        if (us) usleep(us);
        if (!wronly) {
            memrd8(va, sz);
            r += readdata_verify(sz);
        }
    }

    if (wrflags & 0x4) {
        logv("  mem wr 4-byte\n");
        memwr4(va, sz);
        if (us) usleep(us);
        if (!wronly) {
            memrd8(va, sz);
            r += readdata_verify(sz);
        }
    }

    if (wrflags & 0x8) {
        logv("  mem wr 8-byte\n");
        memwr8(va, sz);
        if (us) usleep(us);
        if (!wronly) {
            memrd8(va, sz);
            r += readdata_verify(sz);
        }
    }

    return r;
}

static u_int64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

#define MICF_M  0x01    /* memory */
#define MICF_I  0x02    /* io */
#define MICF_C  0x04    /* cfg */

#define DNIF_D  0x01    /* direct */
#define DNIF_N  0x02    /* notify */
#define DNIF_I  0x04    /* indirect */

#define RW_F_R  0x01    /* read */
#define RW_F_W  0x02    /* write */

static int
t_perf1(int argc, char *argv[])
{
    u_int64_t tstart, tstop, tsum, tdiff, tmin, tmax;
    u_int64_t ntrans;
    int opt, micflags, dniflags, rw_flags;

    micflags = dniflags = rw_flags = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "MICdnirw")) != -1) {
        switch (opt) {
        case 'M': micflags |= MICF_M; break;
        case 'I': micflags |= MICF_I; break;
        case 'C': micflags |= MICF_C; break;
        case 'd': dniflags |= DNIF_D; break;
        case 'n': dniflags |= DNIF_N; break;
        case 'i': dniflags |= DNIF_I; break;
        case 'r': rw_flags |= RW_F_R; break;
        case 'w': rw_flags |= RW_F_W; break;
        default:
            return 1;
        }
    }
    if (micflags == 0) micflags = MICF_M | MICF_I /* | MICF_C */;
    if (dniflags == 0) dniflags = DNIF_D | DNIF_N | DNIF_I;
    if (rw_flags == 0) rw_flags = RW_F_R | RW_F_W;

    logv("perf1 %x%x%x\n", micflags, dniflags, rw_flags);

#define PERFTEST(MICF, DNIF, RW_F, TNAME, TFUNC)        \
    do {                                                \
        if (micflags & MICF_##MICF &&                   \
            dniflags & DNIF_##DNIF &&                   \
            rw_flags & RW_F_##RW_F) {                   \
            tsum = tmax = 0;                            \
            tmin = -1ULL;                               \
            ntrans = 0;                                 \
            while (ntrans < 1000) {                    \
                tstart = timestamp();                   \
                TFUNC;                                  \
                tstop = timestamp();                    \
                ntrans++;                               \
                tdiff = tstop - tstart;                 \
                tsum += tdiff;                          \
                if (tdiff < tmin) tmin = tdiff;         \
                if (tdiff > tmax) tmax = tdiff;                 \
            }                                                   \
            logv("  %s %10.3lfus (min %ldus max %ldus)\n",      \
                 TNAME, (double)tsum / ntrans, tmin, tmax);     \
            /* mem direct rd transaction to flush */            \
            memrd4(tdev.memvad, 0x4);                           \
        }                                                       \
    } while (0)

    PERFTEST(M,D,R, "mem   direct rd", memrd4(tdev.memvad, 0x4));
    PERFTEST(M,D,W, "mem   direct wr", memwr4(tdev.memvad, 0x4));
    PERFTEST(M,N,R, "mem   notify rd", memrd4(tdev.memvan, 0x4));
    PERFTEST(M,N,W, "mem   notify wr", memwr4(tdev.memvan, 0x4));
    PERFTEST(M,I,R, "mem indirect rd", memrd4(tdev.memvai, 0x4));
    PERFTEST(M,I,W, "mem indirect wr", memwr4(tdev.memvai, 0x4));

    PERFTEST(I,D,R, "io    direct rd", iord4(tdev.io_vad, 0x4));
    PERFTEST(I,D,W, "io    direct wr", iowr4(tdev.io_vad, 0x4));
    PERFTEST(I,N,R, "io    notify rd", iord4(tdev.io_van, 0x4));
    PERFTEST(I,N,W, "io    notify wr", iowr4(tdev.io_van, 0x4));
    PERFTEST(I,I,R, "io  indirect rd", iord4(tdev.io_vai, 0x4));
    PERFTEST(I,I,W, "io  indirect wr", iowr4(tdev.io_vai, 0x4));

#if 0
    PERFTEST(C,D,R, "cfg   direct rd", cfgrd4(tdev.cfgd, 0x4));
    PERFTEST(C,D,W, "cfg   direct wr", cfgwr4(tdev.cfgd, 0x4));
    PERFTEST(C,N,R, "cfg   notify rd", cfgrd4(tdev.cfgn, 0x4));
    PERFTEST(C,N,W, "cfg   notify wr", cfgwr4(tdev.cfgn, 0x4));
    PERFTEST(C,I,R, "cfg indirect rd", cfgrd4(tdev.cfgi, 0x4));
    PERFTEST(C,I,W, "cfg indirect wr", cfgwr4(tdev.cfgi, 0x4));
#endif

    return 0;
#undef PERFTEST
}

static int
t_perfmulti(int argc, char *argv[])
{
    u_int64_t tstart, tstop, tsum;
    u_int64_t ntrans;
    int opt, micflags, dniflags, rw_flags;

    micflags = dniflags = rw_flags = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "MICdnirw")) != -1) {
        switch (opt) {
        case 'M': micflags |= MICF_M; break;
        case 'I': micflags |= MICF_I; break;
        case 'C': micflags |= MICF_C; break;
        case 'd': dniflags |= DNIF_D; break;
        case 'n': dniflags |= DNIF_N; break;
        case 'i': dniflags |= DNIF_I; break;
        case 'r': rw_flags |= RW_F_R; break;
        case 'w': rw_flags |= RW_F_W; break;
        default:
            return 1;
        }
    }
    if (micflags == 0) micflags = MICF_M | MICF_I /* | MICF_C */;
    if (dniflags == 0) dniflags = DNIF_D | DNIF_N | DNIF_I;
    if (rw_flags == 0) rw_flags = RW_F_R | RW_F_W;

    logv("perfmulti %x%x%x\n", micflags, dniflags, rw_flags);

#define PERFTEST(MICF, DNIF, RW_F, TNAME, TFUNC)        \
    do {                                                \
        if (micflags & MICF_##MICF &&                   \
            dniflags & DNIF_##DNIF &&                   \
            rw_flags & RW_F_##RW_F) {                   \
            tsum = 0;                                   \
            ntrans = 0;                                 \
            while (tsum < 100000) {                     \
                tstart = timestamp();                   \
                ntrans += TFUNC;                        \
                tstop = timestamp();                    \
                tsum += tstop - tstart;                 \
            }                                                           \
            logv("  %s %10.3lfus\n", TNAME, (double)tsum / ntrans);     \
            /* mem direct rd transaction to flush */                    \
            memrd4(tdev.memvad, 0x4);                                   \
        }                                                               \
    } while (0)

    PERFTEST(M,D,R, "mem   direct rd", memrd4(tdev.memvad, 0x1000));
    PERFTEST(M,D,W, "mem   direct wr", memwr4(tdev.memvad, 0x1000));
    PERFTEST(M,N,R, "mem   notify rd", memrd4(tdev.memvan, 0x1000));
    PERFTEST(M,N,W, "mem   notify wr", memwr4(tdev.memvan, 0x1000));
    PERFTEST(M,I,R, "mem indirect rd", memrd4(tdev.memvai, 0x1000));
    PERFTEST(M,I,W, "mem indirect wr", memwr4(tdev.memvai, 0x1000));

    PERFTEST(I,D,R, "io    direct rd", iord4(tdev.io_vad, 0x20));
    PERFTEST(I,D,W, "io    direct wr", iowr4(tdev.io_vad, 0x20));
    PERFTEST(I,N,R, "io    notify rd", iord4(tdev.io_van, 0x20));
    PERFTEST(I,N,W, "io    notify wr", iowr4(tdev.io_van, 0x20));
    PERFTEST(I,I,R, "io  indirect rd", iord4(tdev.io_vai, 0x20));
    PERFTEST(I,I,W, "io  indirect wr", iowr4(tdev.io_vai, 0x20));

#if 0
    PERFTEST(C,D,R, "cfg   direct rd", cfgrd4(tdev.cfgd, 0x8));
    PERFTEST(C,D,W, "cfg   direct wr", cfgwr4(tdev.cfgd, 0x8));
    PERFTEST(C,N,R, "cfg   notify rd", cfgrd4(tdev.cfgn, 0x8));
    PERFTEST(C,N,w, "cfg   notify wr", cfgwr4(tdev.cfgn, 0x8));
    PERFTEST(C,I,R, "cfg indirect rd", cfgrd4(tdev.cfgi, 0x8));
    PERFTEST(C,I,W, "cfg indirect wr", cfgwr4(tdev.cfgi, 0x8));
#endif

    return 0;
#undef PERFTEST
}

static int
t_direct(int argc, char *argv[])
{
    int errs = 0;

    errs += t_memrdseq_direct(argc, argv);
    errs += t_memwrseq_direct(argc, argv);
    errs += t_iordseq_direct(argc, argv);
    errs += t_iowrseq_direct(argc, argv);

    return errs;
}

static int
t_notify(int argc, char *argv[])
{
    int errs = 0;

    errs += t_memrdseq_notify(argc, argv);
    errs += t_memwrseq_notify(argc, argv);
    errs += t_iordseq_notify(argc, argv);
    errs += t_iowrseq_notify(argc, argv);

    return errs;
}

static int
t_memrwseq_notify(int argc, char *argv[])
{
    int errs = 0;

    errs += t_memrdseq_notify(argc, argv);
    errs += t_memwrseq_notify(argc, argv);

    return errs;
}

static int
t_indirect(int argc, char *argv[])
{
    int errs = 0;

    errs += t_memrdseq_indirect(argc, argv);
    errs += t_memwrseq_indirect(argc, argv);
    errs += t_iordseq_indirect(argc, argv);
    errs += t_iowrseq_indirect(argc, argv);

    return errs;
}

static int
t_default(int argc, char *argv[])
{
    int errs = 0;

    errs += t_memrdseq_direct(argc, argv);
    errs += t_memwrseq_direct(argc, argv);
    errs += t_memrdseq_notify(argc, argv);
    errs += t_memwrseq_notify(argc, argv);
    errs += t_memrdseq_indirect(argc, argv);
    errs += t_memwrseq_indirect(argc, argv);

    errs += t_iordseq_direct(argc, argv);
    errs += t_iowrseq_direct(argc, argv);
    errs += t_iordseq_notify(argc, argv);
    errs += t_iowrseq_notify(argc, argv);
    errs += t_iordseq_indirect(argc, argv);
    errs += t_iowrseq_indirect(argc, argv);

    errs += t_perf1(argc, argv);
    errs += t_perfmulti(argc, argv);

    return errs;
}

static int
t_notifyhang(int argc, char *argv[])
{
    void *va = tdev.memvan;
    void *verify_va = tdev.memvad;
    char verify_by = 'd';
    size_t sz = 0x1000;
    int errs = 0;
    int wrflags = 0;
    int opt;

    optind = 0;
    while ((opt = getopt(argc, argv, "1248s:V:")) != -1) {
        switch (opt) {
        case '1':
            wrflags |= 1;
            break;
        case '2':
            wrflags |= 2;
            break;
        case '4':
            wrflags |= 4;
            break;
        case '8':
            wrflags |= 8;
            break;
        case 's':
            sz = strtoul(optarg, NULL, 0);
            break;
        case 'V':
            verify_by = *optarg;
            switch (verify_by) {
            case 'd': verify_va = tdev.memvad; break;
            case 'n': verify_va = tdev.memvan; break;
            case 'i': verify_va = tdev.memvai; break;
            }
            break;
        default:
            return 1;
        }
    }

    if (wrflags == 0) wrflags = 0xf;

    logv("notifyhang sz %ld wrflags 0x%x verify_by %c\n",
         sz, wrflags, verify_by);

    if (wrflags & 0x1) {
        logv("  mem wr 1-byte\n");
        write_test_reset();
        memwr1(va, sz);
        errs += write_test_verify_va(verify_va, sz);
    }

    if (wrflags & 0x2) {
        logv("  mem wr 2-byte\n");
        write_test_reset();
        memwr2(va, sz);
        errs += write_test_verify_va(verify_va, sz);
    }

    if (wrflags & 0x4) {
        logv("  mem wr 4-byte\n");
        write_test_reset();
        memwr4(va, sz);
        errs += write_test_verify_va(verify_va, sz);
    }

    if (wrflags & 0x8) {
        logv("  mem wr 8-byte\n");
        write_test_reset();
        memwr8(va, sz);
        errs += write_test_verify_va(verify_va, sz);
    }

    return errs;
}

typedef struct pcietest_s {
    char *name;
    int (*f)(int argc, char *argv[]);
} pcietest_t;

static pcietest_t testtab[] = {
#define TESTTAB(name) \
    { #name, t_##name }

    /* unit test cases */
    TESTTAB(memrdseq_direct),
    TESTTAB(memwrseq_direct),
    TESTTAB(memrdseq_notify),
    TESTTAB(memwrseq_notify),
    TESTTAB(memrdseq_indirect),
    TESTTAB(memwrseq_indirect),
    TESTTAB(iordseq_direct),
    TESTTAB(iowrseq_direct),
    TESTTAB(iordseq_notify),
    TESTTAB(iowrseq_notify),
    TESTTAB(iordseq_indirect),
    TESTTAB(iowrseq_indirect),
    TESTTAB(cfgrdseq_direct),
    TESTTAB(cfgwrseq_direct),

    /* composite test cases */
    TESTTAB(default),
    TESTTAB(perf1),
    TESTTAB(perfmulti),
    TESTTAB(direct),
    TESTTAB(notify),
    TESTTAB(memrwseq_notify),
    TESTTAB(indirect),
    TESTTAB(simpletest),
    TESTTAB(notifyhang),

    { NULL, NULL }
};

static pcietest_t *
test_lookup(const char *name)
{
    pcietest_t *t = testtab;

    for (t = testtab; t->name != NULL; t++) {
        if (strcmp(name, t->name) == 0) {
            return t;
        }
    }
    return NULL;
}

static int
runtest(int argc, char *argv[])
{
    pcietest_t *t = test_lookup(argv[0]);
    int niters = testiters;
    int errs, runs, linedots;
    time_t now, lastdot;

    if (t == NULL) {
        fprintf(stderr, "%s: invalid test name\n", argv[0]);
        return -1;
    }

    /* init test pattern once */
    testdata_init();

    logv("=== test: %s\n", t->name);

    runs = errs = 0;
    linedots = 100;
    lastdot = 0;

    while (testiters == 0 || niters--) {

        errs += t->f(argc, argv);
        runs++;

        if (showdots) {
            verbose = 0; /* cancel verbose if enabled, replace with dots */
            time(&now);
            if (difftime(now, lastdot) > 1) {
                if (linedots >= 30) {
                    printf("\n[%24.24s runs %d errs %d]",
                           ctime(&now), runs, errs);
                    linedots = 0;
                } else {
                    putchar('.');
                    linedots++;
                }
                fflush(stdout);
                lastdot = now;
            }
        }
    }
    if (showdots) {
        time(&now);
        printf("\n[%24.24s runs %d errs %d]",
               ctime(&now), runs, errs);
        putchar('\n');
        fflush(stdout);
    }

    return errs;
}

int
main(int argc, char *argv[])
{
    struct pci_access *pacc;
    struct pci_filter pfilter;
    struct pci_dev *pd;
    tdev_t *td = &tdev;
    int opt, got_filter, r;
    char *s;

    memset(td, 0, sizeof(tdev_t));

    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);
    pci_filter_init(pacc, &pfilter);

    got_filter = 0;
    while ((opt = getopt(argc, argv, "d:DIn:s:v")) != -1) {
        switch (opt) {
        case 'd':
            if (strcmp(optarg, "test") == 0) {
                testmode = 1;
            } else if ((s = pci_filter_parse_id(&pfilter, optarg)) != NULL) {
                fprintf(stderr, "Unable to parse -d: %s\n", s);
                usage();
                exit(1);
            }
            got_filter = 1;
            break;
        case 's':
            if ((s = pci_filter_parse_slot(&pfilter, optarg)) != NULL) {
                fprintf(stderr, "Unable to parse -s: %s\n", s);
                usage();
                exit(1);
            }
            got_filter = 1;
            break;
        case 'D':
            showdots = 1;
            break;
        case 'I':
            simio = 1;
            break;
        case 'n':
            testiters = strtoul(optarg, NULL, 0);
            break;
        case 'v':
            verbose++;
            break;
        default:
            usage();
            exit(1);
        }
    }

    argc -= optind;
    argv += optind;

    /* no device specified? find default */
    if (!got_filter) {
        char filterstr[80] = { "1dd8:8001" };
        pci_filter_parse_id(&pfilter, filterstr);
    }

    if (!testmode) {
        /* find the first instance of our test device */
        for (pd = pacc->devices; pd; pd = pd->next) {
            if (pci_filter_match(&pfilter, pd)) {
                td->pcidev = pd;
                break;
            }
        }

        if (td->pcidev == NULL) {
            fprintf(stderr, "no pciestress device found\n");
            exit(1);
        }

        if (opentdev(td) < 0) {
            exit(1);
        }
    } else {
        opentestdev(td);
    }

    /* allow in/out's */
    if (!simio && iopl(0x3) < 0) {
        fprintf(stderr, "iopl failed (run as root?)\n");
        exit(1);
    }

    if (argc) {
        r = runtest(argc, argv);
    } else {
        char *t_default = "default";
        r = runtest(1, &t_default);
    }

    pci_cleanup(pacc);
    if (r) fprintf(stderr, "ERRORS: %d\n", r);
    exit(r ? 1 : 0);
}
