/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <cinttypes>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "platform/evutils/include/evutils.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pciehdevices/include/pciehdevices.h"
#include "platform/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/pciemgr_if/include/pmserver.h"

#define NCLIENTS        100

typedef struct serverctx_s {
    int fd;
} serverctx_t;

static serverctx_t serverctx;
static int verbose_flag;

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

static void
usage(void)
{
    printf("Usage: pciemgrs [-v]\n");
}

static serverctx_t *
serverctx_get(void)
{
    return &serverctx;
}

static void
do_open(pmmsg_t *m)
{
    pciesys_loginfo("open: %s\n", m->open.name);
}

static void
do_initialize(pmmsg_t *m)
{
    pciesys_loginfo("initialize: port %d\n", m->initialize.port);
}

static void
do_finalize(pmmsg_t *m)
{
    pciesys_loginfo("finalize: port %d\n", m->finalize.port);
}

static void
do_devres_add(pmmsg_t *m)
{
    assert(0);
}

static void
client_msg_cb(pmmsg_t *m)
{
    switch (m->hdr.msgtype) {
    case PMMSG_OPEN:
        do_open(m);
        break;
    case PMMSG_INITIALIZE:
        do_initialize(m);
        break;
    case PMMSG_FINALIZE:
        do_finalize(m);
        break;
    case PMMSG_DEVRES_ADD:
        do_devres_add(m);
        break;
    default:
        break;
    }

    pciemgr_msgfree(m);
}

static void
timercb(void *arg)
{
    printf("timercb\n");
}

int
main(int argc, char *argv[])
{
    serverctx_t *sc = serverctx_get();
    char *addrstr = NULL;
    evutil_timer timer;
    int opt;

    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
        case 'v':
            verbose_flag = 1;
            break;
        case '?':
        default:
            usage();
            exit(1);
        }
    }

    sc->fd = pciemgrs_open(addrstr, client_msg_cb);
    if (sc->fd < 0) {
        exit(1);
    }
    evutil_timer_start(EV_DEFAULT_ &timer, timercb, NULL, 3.0, 3.0);
    evutil_run(EV_DEFAULT);
    pciemgrs_close();
    exit(0);
    if (0) verbose("reference to verbose func\n");
}
