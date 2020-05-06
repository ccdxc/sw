/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>

#include "platform/misc/include/misc.h"
#include "platform/evutils/include/evutils.h"
#include "platform/pciehdevices/include/pciehdevices.h"
#include "platform/pciemgr/include/pciehdev_event.h"
#include "platform/pciemgr_if/include/pciemgr_if.hpp"

static int verbose_flag;
static pciemgr *pciemgr;

static void
usage(void)
{
    fprintf(stderr,
"Usage: pciemgrev [-v]\n"
"    -v                 verbose\n");
}

static void verbose(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
static void
verbose(const char *fmt, ...)
{
    va_list arg;

    if (verbose_flag) {
        va_start(arg, fmt);
        vprintf(fmt, arg);
        va_end(arg);
    }
}

class myevhandler : public pciemgr::evhandler {
    virtual void memrd(const int port,
                       const uint32_t lif,
                       const pciehdev_memrw_notify_t *n) {
        printf("memrd: port %d lif %u\n"
               "    bar %d baraddr 0x%" PRIx64 " baroffset 0x%" PRIx64 " "
               "size %u localpa 0x%" PRIx64 " data 0x%" PRIx64 "\n",
               port, lif,
               n->cfgidx, n->baraddr, n->baroffset,
               n->size, n->localpa, n->data);
    }
    virtual void memwr(const int port,
                       const uint32_t lif,
                       const pciehdev_memrw_notify_t *n) {
        printf("memwr: port %d lif %u\n"
               "    bar %d baraddr 0x%" PRIx64 " baroffset 0x%" PRIx64 " "
               "size %u localpa 0x%" PRIx64 " data 0x%" PRIx64 "\n",
               port, lif,
               n->cfgidx, n->baraddr, n->baroffset,
               n->size, n->localpa, n->data);
    }
    virtual void hostup(const int port) {
        printf("hostup: port %d\n", port);
    }
    virtual void hostdn(const int port) {
        printf("hostdn: port %d\n", port);
    }
    virtual void sriov_numvfs(const int port,
                              const uint32_t lif,
                              const uint16_t numvfs) {
        printf("sriov_numvfs: port %d lif %u numvfs %u\n",
               port, lif, numvfs);
    }
    virtual void reset(const int port,
                       uint32_t rsttype,
                       const uint32_t lifb,
                       const uint32_t lifc) {
        printf("reset: port %d rsttype %u lifb %u lifc %u\n",
               port, rsttype, lifb, lifc);
    }
};

int
main(int argc, char *argv[])
{
    const char *myname = "pciemgrev";
    myevhandler myevh;
    pciemgr = new class pciemgr(myname, myevh, EV_DEFAULT);
    int opt;

    while ((opt = getopt(argc, argv, "v:")) != -1) {
        switch (opt) {
        case 'v':
            verbose_flag = 1;
            break;
        default:
            usage();
            exit(1);
        }
    }

    printf("waiting for events (^C to interrupt)\n");
    evutil_run(EV_DEFAULT);

    delete pciemgr;
    exit(0);

    if (0) verbose("reference");
    if (0) usage();
}
