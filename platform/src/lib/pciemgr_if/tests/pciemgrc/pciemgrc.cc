/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"

static int verbose_flag;
static pciemgr *pciemgr;

static void
usage(void)
{
    fprintf(stderr,
"Usage: pciemgr [-Fnv][-e <enabled_ports>[-b <first_bus_num>][-P gen<G>x<W>][-s subdeviceid]\n"
"    -b <first_bus_num> set first bus used to <first_bus_num>\n"
"    -e <enabled_ports> max of enabled pcie ports\n"
"    -F                 no fake bios scan\n"
"    -h                 initializing hw\n"
"    -H                 no initializing hw\n"
"    -P gen<G>x<W>      spec devices as pcie gen <G>, lane width <W>\n"
"    -s subdeviceid     default subsystem device id\n"
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
};

int
main(int argc, char *argv[])
{
    const char *myname = "pciemgr";
    myevhandler myevh;
    pciemgr = new class pciemgr(myname, myevh);

    pciemgr->initialize();

    pciehdevice_resources_t p;
    memset(&p, 0, sizeof(p));
    p.lifb = 5;
    p.lifc = 1;
    p.intrb = 0;
    p.intrc = 4;
    p.devcmdpa = 0x13f000000;
    p.devcmddbpa = 0x13f001000;
    p.romsz = 4096;
    p.rompa = 0x13f000000;
    pciehdev_t *pdev = pciehdev_eth_new("eth", &p);
    printf("adding pdev %p lif %u\n", pdev, p.lifb);
    pciemgr->add_device(pdev);

    pciemgr->finalize();

    printf("evutil_run()\n");
    evutil_run();

    delete pciemgr;
    exit(0);

    if (0) verbose("reference");
    if (0) usage();
}
