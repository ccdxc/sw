/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/user.h>
#ifdef BUILD_OS_Darwin
#define inb(port) 0
#define inw(port) 0
#define inl(port) 0
#define outb(port, val) while (0);
#define outw(port, val) while (0);
#define outl(port, val) while (0);
#else
#include <sys/io.h>
#endif

enum { SPACEMEM, SPACEIO };
enum { DIRRD, DIRWR };

typedef struct req {
    u_int64_t addr;
    size_t size;
    int space;
    int dir;
    char **wvals;
    int nwvals;
} req_t;

static long page_size;

static void
usage(void)
{
    fprintf(stderr,
            "Usage: physmem [-1248bhldi] <addr> [<write-val> ...]\n"
            "    -b|1   byte (1-byte) access\n"
            "    -h|2   half-word (2-byte) access\n"
            "    -l|4   word (4-byte) access (default)\n"
            "    -d|8   double-word (8-byte) access\n"
            "    -i     iospace access\n"
            "    -m     memory space access (default)\n");
}

static void
page_size_init(void)
{
    if (page_size == 0) {
        page_size = getpagesize();
    }
}

static size_t
page_offset(const off_t addr)
{
    return addr & (page_size - 1);
}

static size_t
page_align(const off_t addr)
{
    return addr & ~(page_size - 1);
}

static size_t
page_span(const off_t addr, const size_t size)
{
    return roundup(addr + size, page_size) - page_align(addr);
}

static void *
map_address(off_t addr, size_t size, int oflags)
{
    const int prot = (oflags == O_RDONLY) ? PROT_READ : PROT_READ | PROT_WRITE;
    const int mapf = MAP_SHARED;
    int fd;
    void *va;

#ifdef BUILD_OS_Darwin
    static char buf[8] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
    return buf;
#endif

    fd = open("/dev/mem", oflags | O_SYNC);
    if (fd < 0) {
        if (errno == EPERM) {
            fprintf(stderr, "requires root privileges for /dev/mem access\n");
        } else {
            perror("/dev/mem");
        }
        return NULL;
    }
    va = mmap(NULL, page_span(addr, size), prot, mapf, fd, page_align(addr));
    (void)close(fd);
    if (va == (void *)-1) {
        perror("mmap");
        return NULL;
    }
    return va + page_offset(addr);
}

static void
do_memrd(req_t *req)
{
    u_int64_t val;
    void *va = map_address(req->addr, req->size, O_RDONLY);
    if (va != NULL) {
        switch (req->size) {
        case 1: val = *(u_int8_t  *)va; break;
        case 2: val = *(u_int16_t *)va; break;
        case 4: val = *(u_int32_t *)va; break;
        case 8: val = *(u_int64_t *)va; break;
        default:
            fprintf(stderr, "invalid size %zu\n", req->size);
            return;
        }
        printf("0x%0*llx\n", (int)req->size * 2, (long long)val);
    }
}

static void
do_memwr(req_t *req)
{
    u_int64_t val;
    void *va;
    int i;

    va = map_address(req->addr, req->size * req->nwvals, O_RDWR);
    if (va != NULL) {
        for (i = 0; i < req->nwvals; i++, va += req->size) {
            val = strtoull(req->wvals[i], NULL, 0);
            switch (req->size) {
            case 1: *(u_int8_t  *)va = val; break;
            case 2: *(u_int16_t *)va = val; break;
            case 4: *(u_int32_t *)va = val; break;
            case 8: *(u_int64_t *)va = val; break;
            default:
                fprintf(stderr, "invalid size %zu\n", req->size);
                return;
            }
        }
    }
}

static void
do_memreq(req_t *req)
{
    if (req->dir == DIRRD) {
        do_memrd(req);
    } else {
        do_memwr(req);
    }
}

static void
do_iord(req_t *req)
{
    unsigned val;

    switch (req->size) {
    case 1: val = inb(req->addr); break;
    case 2: val = inw(req->addr); break;
    case 4: val = inl(req->addr); break;
    default:
        fprintf(stderr, "invalid size %zu\n", req->size);
        return;
    }
    printf("0x%0*x\n", (int)req->size * 2, val);
}

static void
do_iowr(req_t *req)
{
    unsigned val;
    u_int16_t port;
    int i;

    port = req->addr;
    for (i = 0; i < req->nwvals; i++, port += req->size) {
        val = strtoul(req->wvals[i], NULL, 0);
        switch (req->size) {
        case 1: outb(val, port); break;
        case 2: outw(val, port); break;
        case 4: outl(val, port); break;
        default:
            fprintf(stderr, "invalid size %zu\n", req->size);
            return;
        }
    }
}

static void
do_ioreq(req_t *req)
{
#ifndef BUILD_OS_Darwin
    if (iopl(3) < 0) {
        if (errno == EPERM) {
            fprintf(stderr, "requires root privileges for iospace access\n");
        } else {
            perror("iopl");
        }
        return;
    }
#endif

    if (req->dir == DIRRD) {
        do_iord(req);
    } else {
        do_iowr(req);
    }
}

int
main(int argc, char *argv[])
{
    req_t req;
    int opt;

    memset(&req, 0, sizeof(req));
    req.space = SPACEMEM;
    req.dir = DIRRD;
    req.size = 4;

    while ((opt = getopt(argc, argv, "1248bhldim")) != -1) {
        switch (opt) {
        case '1':
        case 'b':
            req.size = 1;
            break;
        case '2':
        case 'h':
            req.size = 2;
            break;
        case '4':
        case 'l':
            req.size = 4;
            break;
        case '8':
        case 'd':
            req.size = 8;
            break;
        case 'i':
            req.space = SPACEIO;
            break;
        case 'm':
            req.space = SPACEMEM;
            break;
        default:
            usage();
            exit(1);
        }
    }

    if (optind >= argc) {
        usage();
        exit(2);
    }

    req.addr = strtoull(argv[optind], NULL, 0);

    optind++;
    if (optind < argc) {
        req.dir = DIRWR;
        req.wvals = &argv[optind];
        req.nwvals = argc - optind;
    }

    page_size_init();

    if (req.space == SPACEMEM) {
        do_memreq(&req);
    } else {
        do_ioreq(&req);
    }

    exit(0);
}
