/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include "misc.h"
#include "evutils.h"
#include "cfgspace.h"
#include "pciemgrutils.h"
#include "pciehdev_impl.h"
#include "pciehcfg_impl.h"
#include "pciehbar_impl.h"
#include "pciehw_dev.h"
#include "pciesvc.h"
#include "pciemgr_if.hpp"

static pciemgr::evhandler default_evhandler;

pciemgr::pciemgr(const char *name) :
    pciemgr(name, default_evhandler) {}

pciemgr::pciemgr(const char *name, evhandler &evhandlercb) :
        evhandlercb (evhandlercb)
{
    serverfd = pciemgrc_open(name, NULL);
    if (serverfd < 0) {
        // XXX retry connecting to server?
        fprintf(stderr, "No pciemgr server found - exiting\n");
        exit(1);
    }
    evutil_add_fd(serverfd, msgrecv, NULL, this);
}

pciemgr::~pciemgr(void)
{
    evutil_remove_fd(serverfd);
    pciemgrc_close();
}

int
pciemgr::initialize(const int port)
{
    pmmsg_t m = {
        .hdr = {
            .msgtype = PMMSG_INITIALIZE,
            .msglen = sizeof(pmmsg_initialize_t),
        },
    };

    m.initialize.port = port;
    return pciemgrc_msgsend(&m);
}

int
pciemgr::finalize(const int port)
{
    pmmsg_t m = {
        .hdr = {
            .msgtype = PMMSG_FINALIZE,
            .msglen = sizeof(pmmsg_finalize_t),
        },
    };

    m.finalize.port = port;
    return pciemgrc_msgsend(&m);
}

int
pciemgr::add_device(pciehdev_t *pdev)
{
    pmmsg_t *m;
    char *mp;

    pciehbars_t *pbars = pciehdev_get_bars(pdev);
    pciehbar_t *pbar;
    pciehbarreg_t *preg;
    int nregs, nprts;

    nprts = 0;
    nregs = 0;
    for (pbar = pciehbars_get_first(pbars);
         pbar != NULL;
         pbar = pciehbars_get_next(pbars, pbar)) {

        nregs += pbar->nregs;

        preg = pbar->regs;
        for (int r = 0; r < pbar->nregs; r++, preg++) {
            nprts += preg->nprts;
        }
    }

    size_t msglen = (sizeof(pmmsg_dev_add_t) +
                     sizeof(pciehdev_t) +
                     sizeof(pciehcfg_t) +
                     PCIEHCFGSZ * 2 +
                     sizeof(pciehbars_t) +
                     sizeof(pciehbar_t) * pbars->nbars +
                     sizeof(pciehbarreg_t) * nregs +
                     sizeof(prt_t) * nprts);
    pciemgrc_msgalloc(&m, msglen);

    // msg header
    m->hdr.msgtype = PMMSG_DEV_ADD;

    mp = (char *)&m->dev_add + sizeof(pmmsg_dev_add_t);

    // pciehdev_t
    // Make a local copy of the pdev struct so we can assign
    // our own private tag, using the callers pdev as our tag.
    // We can get the tag from the pdev associated with events
    // so we can link the event back to our caller's pdev.
    pciehdev_t lpdev;
    lpdev = *pdev;
    pciehdev_set_priv(&lpdev, pdev);
    memcpy(mp, &lpdev, sizeof(pciehdev_t));
    mp += sizeof(pciehdev_t);

    // pciehcfg_t
    pciehcfg_t *pcfg = pciehdev_get_cfg(pdev);
    memcpy(mp, pcfg, sizeof(pciehcfg_t));
    mp += sizeof(pciehcfg_t);

    // config space
    memcpy(mp, pcfg->cur, PCIEHCFGSZ);
    mp += PCIEHCFGSZ;
    memcpy(mp, pcfg->msk, PCIEHCFGSZ);
    mp += PCIEHCFGSZ;

    // bars
    memcpy(mp, pbars, sizeof(pciehbars_t));
    mp += sizeof(pciehbars_t);

    pciehbar_t *msgbar;
    pciehbarreg_t *msgreg;
    prt_t *msgprt;

    msgbar = (pciehbar_t *)mp;
    for (pbar = pciehbars_get_first(pbars);
         pbar != NULL;
         pbar = pciehbars_get_next(pbars, pbar)) {

        // bar
        memcpy(msgbar, pbar, sizeof(*pbar));
        msgreg = (pciehbarreg_t *)(msgbar + 1);
        preg = pbar->regs;
        for (int r = 0; r < pbar->nregs; r++, preg++) {
            // region
            memcpy(msgreg, preg, sizeof(*preg));
            // prts
            msgprt = (prt_t *)(msgreg + 1);
            memcpy(msgprt, preg->prts, sizeof(*msgprt) * preg->nprts);
            msgreg = (pciehbarreg_t *)&msgprt[preg->nprts];
        }
        msgbar = (pciehbar_t *)msgreg;
    }
    mp = (char *)msgbar;

    // msg complete - send it
    pciemgrc_msgsend(m);
    pciemgrc_msgfree(m);
    return 0;
}

void
pciemgr::msgrecv(void *arg)
{
    pciemgr *pciemgr = (class pciemgr *)arg;
    pmmsg_t *m;

    pciemgrc_msgrecv(&m);
    pciemgr->msghandler(m);
    pciemgrc_msgfree(m);
}

void
pciemgr::msghandler(pmmsg_t *m)
{
    switch (m->hdr.msgtype) {
    case PMMSG_EVENT: {
        char *msgdata = (char *)&m->event + sizeof(pmmsg_event_t);
        pciehdev_eventdata_t *evd = (pciehdev_eventdata_t *)msgdata;
        handle_event(evd);
        break;
    }
    default:
        // we can ignore this event
        break;
    }
}

void
pciemgr::handle_event(const pciehdev_eventdata_t *evd)
{
    switch (evd->evtype) {
    case PCIEHDEV_EV_MEMRD_NOTIFY:
        evhandlercb.memrd(evd->port, evd->pdev, &evd->memrw_notify);
        break;
    case PCIEHDEV_EV_MEMWR_NOTIFY:
        evhandlercb.memwr(evd->port, evd->pdev, &evd->memrw_notify);
        break;
    default:
        break;
    }
}
