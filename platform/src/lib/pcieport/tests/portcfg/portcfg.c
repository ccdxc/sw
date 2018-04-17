/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/param.h>

#include "src/lib/pcieport/include/portcfg.h"

enum { DIRRD, DIRWR };

typedef struct req {
    u_int16_t addr;
    size_t size;
    int dir;
    char **wvals;
    int nwvals;
} req_t;

static void
usage(void)
{
    fprintf(stderr,
            "Usage: portcfg [-1248bhldi] <addr> [<write-val> ...]\n"
            "    -b|1   byte (1-byte) access\n"
            "    -h|2   half-word (2-byte) access\n"
            "    -l|4   word (4-byte) access (default)\n");
}

static void
do_reqrd(req_t *req)
{
    u_int32_t val;

    switch (req->size) {
    case 1: val = portcfg_readb(0, req->addr); break;
    case 2: val = portcfg_readw(0, req->addr); break;
    case 4: val = portcfg_readd(0, req->addr); break;
    default:
        fprintf(stderr, "invalid size %zu\n", req->size);
        return;
    }
    printf("0x%0*lx\n", (int)req->size * 2, (unsigned long)val);
}

static void
do_reqwr(req_t *req)
{
    u_int16_t addr, val;
    int i;

    for (addr = req->addr, i = 0; i < req->nwvals; i++, addr += req->size) {
        val = strtoul(req->wvals[i], NULL, 0);
        switch (req->size) {
        case 1: portcfg_writeb(0, addr, val); break;
        case 2: portcfg_writew(0, addr, val); break;
        case 4: portcfg_writed(0, addr, val); break;
        default:
            fprintf(stderr, "invalid size %zu\n", req->size);
            return;
        }
    }
}

static void
do_req(req_t *req)
{
    if (req->dir == DIRRD) {
        do_reqrd(req);
    } else {
        do_reqwr(req);
    }
}

int
main(int argc, char *argv[])
{
    int opt;
    req_t req;

    memset(&req, 0, sizeof(req));
    req.dir = DIRRD;
    req.size = 4;

    while ((opt = getopt(argc, argv, "124bhl")) != -1) {
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
        default:
            usage();
            exit(1);
        }
    }

    if (optind >= argc) {
        usage();
        exit(2);
    }

    req.addr = strtoul(argv[optind], NULL, 0);

    optind++;
    if (optind < argc) {
        req.dir = DIRWR;
        req.wvals = &argv[optind];
        req.nwvals = argc - optind;
    }

    do_req(&req);

    exit(0);
}
