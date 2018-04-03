/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "pal.h"

#define pal_readreg32(pr, pa, v) \
    do { *v = pal_reg_rd32(pa); } while (0)
#define pal_readreg64(pr, pa, v) \
    do { *v = pal_reg_rd64(pa); } while (0)
#define pal_writereg32(pr, pa, v) \
    do { pal_reg_wr32(pa, v); } while (0)
#define pal_writereg64(pr, pa, v) \
    do { pal_reg_wr64(pa, v); } while (0)

enum { DIRRD, DIRWR };

typedef struct req {
    u_int64_t addr;
    size_t size;
    int dir;
    char **wvals;
    int nwvals;
} req_t;

static void
usage(void)
{
    fprintf(stderr,
            "Usage: palreg [-ld] <addr> [<write-val> ...]\n");
}

static void
do_readreg32(req_t *req)
{
    u_int32_t v;

    assert(req->size == 4);
    pal_readreg32(req->pr, req->addr, &v);
    printf("0x%"PRIx64": 0x%08x\n", req->addr, v);
}

static void
do_readreg64(req_t *req)
{
    u_int64_t v;

    assert(req->size == 8);
    pal_readreg64(req->pr, req->addr, &v);
    printf("0x%"PRIx64": 0x%08"PRIx64"\n", req->addr, v);
}

static void
do_writereg32(req_t *req)
{
    int i;

    assert(req->size == 4);
    for (i = 0; i < req->nwvals; i++) {
        u_int32_t val = strtoul(req->wvals[i], NULL, 0);

        pal_writereg32(req->pr, req->addr, val);
    }
}

static void
do_writereg64(req_t *req)
{
    int i;

    assert(req->size == 8);
    for (i = 0; i < req->nwvals; i++) {
        u_int64_t val = strtoull(req->wvals[i], NULL, 0);

        pal_writereg64(req->pr, req->addr, val);
    }
}

static void
do_readreg(req_t *req)
{
    switch (req->size) {
    case 4: do_readreg32(req); break;
    case 8: do_readreg64(req); break;
    default:
        fprintf(stderr, "readreg invalid size: %lu\n", req->size);
        break;
    }
}

static void
do_writereg(req_t *req)
{
    switch (req->size) {
    case 4: do_writereg32(req); break;
    case 8: do_writereg64(req); break;
    default:
        fprintf(stderr, "writereg invalid size: %lu\n", req->size);
        break;
    }
}

static void
do_req(req_t *req)
{
    if (req->dir == DIRRD) {
        do_readreg(req);
    } else {
        do_writereg(req);
    }
}

int
main(int argc, char *argv[])
{
    req_t req;
    int opt;

    memset(&req, 0, sizeof(req));
    req.dir = DIRRD;
    req.size = 4;

    while ((opt = getopt(argc, argv, "48lde")) != -1) {
        switch (opt) {
        case '4':
        case 'l':
            req.size = 4;
            break;
        case '8':
        case 'd':
            req.size = 8;
            break;
        case 'e':
            printf("env %d is_asic %d\n", pal_get_env(), pal_is_asic());
            exit(0);
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

    do_req(&req);

    exit(0);
}
