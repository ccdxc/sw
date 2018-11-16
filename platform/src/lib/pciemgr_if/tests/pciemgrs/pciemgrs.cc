/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "platform/src/lib/evutils/include/evutils.h"
#include "platform/src/lib/pciehdevices/include/pciehdevices.h"
#include "platform/src/lib/pciemgrutils/include/pmt.h"
#include "platform/src/lib/pciemgrutils/include/pciehbar.h"
#include "platform/src/lib/pciemgrutils/include/pciehbar_impl.h"
#include "platform/src/lib/pciemgrutils/include/pciehcfg.h"
#include "platform/src/lib/pciemgrutils/include/pciehcfg_impl.h"
#include "platform/src/lib/pciemgrutils/include/pciehdev.h"
#include "platform/src/lib/pciemgrutils/include/pciehdev_impl.h"
#include "platform/src/lib/pciemgrutils/include/pciemgrutils.h"
#include "platform/src/lib/pciemgr/include/pciehw_dev.h"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/lib/pciemgr_if/include/pmserver.h"

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

static size_t
msgdata_left(pmmsg_t *m, char *mp)
{
    return m->hdr.msglen - (mp - (char *)m);
}

static void
do_dev_add(pmmsg_t *m)
{
    pciehdev_t *pdev = pciehdev_new("", NULL);
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehbars_new();
    char *mp;
    int b, r, p, nbars;

    if (m->hdr.msglen < (sizeof(pmmsg_dev_add_t) +
                         sizeof(pciehdev_t) +
                         sizeof(pciehcfg_t) +
                         PCIEHCFGSZ * 2 +
                         sizeof(pciehbars_t))) {
        pciesys_logerror("malformed dev_add msg, len %d\n", m->hdr.msglen);
        goto out;
    }

    mp = (char *)&m->dev_add + sizeof(pmmsg_dev_add_t);
    /* pciehdev_t */
    memcpy(pdev, mp, sizeof(pciehdev_t));
    mp += sizeof(pciehdev_t);
    /* pciehcfg_t */
    memcpy(pcfg, mp, sizeof(pciehcfg_t));
    mp += sizeof(pciehcfg_t);

    pcfg->cur = (u_int8_t *)(pcfg + 1);
    pcfg->msk = pcfg->cur + PCIEHCFGSZ;

    memcpy(pcfg->cur, mp, PCIEHCFGSZ);
    mp += PCIEHCFGSZ;
    memcpy(pcfg->msk, mp, PCIEHCFGSZ);
    mp += PCIEHCFGSZ;

    pciehdev_set_cfg(pdev, pcfg);
    pcfg = NULL; /* now owned by dev */

    /* bars, regs */
    memcpy(pbars, mp, sizeof(pciehbars_t));
    mp += sizeof(pciehbars_t);

    nbars = pbars->nbars;
    pbars->nbars = 0;
    pbars->bars = NULL;
    pbars->rombar = NULL;

    for (b = 0; b < nbars; b++) {
        pciehbar_t bar;
        int nregs;

        if (msgdata_left(m, mp) < sizeof(pciehbar_t)) {
            pciesys_logerror("malformed dev_add msg, bar %d len %ld\n",
                             b, msgdata_left(m, mp));
            goto out;
        }

        memcpy(&bar, mp, sizeof(pciehbar_t));
        mp += sizeof(pciehbar_t);

        nregs = bar.nregs;
        bar.nregs = 0;
        bar.regs = NULL;

        for (r = 0; r < nregs; r++) {
            pciehbarreg_t reg;
            int nprts;

            memcpy(&reg, mp, sizeof(pciehbarreg_t));
            mp += sizeof(pciehbarreg_t);

            nprts = reg.nprts;
            reg.nprts = 0;
            reg.prts = NULL;

            for (p = 0; p < nprts; p++) {
                pciehbarreg_add_prt(&reg, (prt_t *)mp);
                mp += sizeof(prt_t);
            }
            pciehbar_add_reg(&bar, &reg);
        }
        pciehbars_add_bar(pbars, &bar);
    }

    pciehdev_set_bars(pdev, pbars);
    pbars = NULL; /* now owned by dev */

    {
        pciehbar_t *pbar = pdev->pbars->bars;
        pciesys_loginfo("dev_add: %s lif %d\n", pdev->name, pdev->lif);
        for (b = 0; b < pdev->pbars->nbars; b++, pbar++) {
            pciesys_loginfo("  bar %d nregs %d\n", b, pbar->nregs);
            pciehbarreg_t *preg = pbar->regs;
            for (r = 0; r < pbar->nregs; r++, preg++) {
                pciesys_loginfo("    reg %d baroff 0x%09" PRIx64 
                                " nprts %d\n",
                                r, preg->baroff,
                                preg->nprts);
            }
        }
    }

#if 0
    pciehdev_add(pdev);
#endif
    return;

 out:
    if (pbars) pciehbars_delete(pbars);
    if (pcfg) pciehcfg_delete(pcfg);
    if (pdev) pciehdev_delete(pdev);
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
    case PMMSG_DEV_ADD:
        do_dev_add(m);
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
    evutil_timer_start(&timer, timercb, NULL, 3.0, 3.0);
    evutil_run();
    pciemgrs_close();
    exit(0);
    if (0) verbose("reference to verbose func\n");
}
