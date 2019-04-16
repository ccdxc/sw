/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/cfgspace/include/cfgspace.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciemgrutils/include/pciehdev_impl.h"
#include "nic/sdk/platform/pciemgrutils/include/pciehcfg_impl.h"
#include "nic/sdk/platform/pciemgrutils/include/pciehbar_impl.h"

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

static void
msg_copy(pmmsg_t *m, char **mpp, void *buf, size_t len)
{
    if (m) {
        memcpy(*mpp, buf, len);
    }
    *mpp += len;
}

static void
msgwrite_reg(pmmsg_t *m, char **mpp, pciehbarreg_t *preg)
{
    msg_copy(m, mpp, preg, sizeof(pciehbarreg_t));
    msg_copy(m, mpp, preg->prts, sizeof(prt_t) * preg->nprts);
}

static void
msgwrite_bar(pmmsg_t *m, char **mpp, pciehbar_t *pbar)
{
    pciehbarreg_t *preg = pbar->regs;

    for (int r = 0; r < pbar->nregs; r++, preg++) {
        msgwrite_reg(m, mpp, preg);
    }
}

static void
msgwrite_bars(pmmsg_t *m, char **mpp, pciehbars_t *pbars)
{
    pciehbar_t *pbar;

    // copy our bars header structure
    msg_copy(m, mpp, pbars, sizeof(pciehbars_t));

    // copy each bar
    for (pbar = pciehbars_get_first(pbars);
         pbar != NULL;
         pbar = pciehbars_get_next(pbars, pbar)) {
        msgwrite_bar(m, mpp, pbar);
    }
    // rom bar
    pbar = pciehbars_get_rombar(pbars);
    if (pbar) {
        msgwrite_bar(m, mpp, pbar);
    }
}

static void
msgwrite_cfg(pmmsg_t *m, char **mpp, pciehcfg_t *pcfg)
{
    msg_copy(m, mpp, pcfg, sizeof(pciehcfg_t));
    msg_copy(m, mpp, pcfg->cur, PCIEHCFGSZ);
    msg_copy(m, mpp, pcfg->msk, PCIEHCFGSZ);
}

static void
msgwrite_dev(pmmsg_t *m, char **mpp, pciehdev_t *pdev)
{
    msg_copy(m, mpp, pdev, sizeof(pciehdev_t));
}

static void
msgwrite_dev_structs(pmmsg_t *m, char **mpp, pciehdev_t *pdev)
{
    msgwrite_dev(m, mpp, pdev);

    pciehcfg_t *pcfg = pciehdev_get_cfg(pdev);
    msgwrite_cfg(m, mpp, pcfg);

    pciehbars_t *pbars = pciehdev_get_bars(pdev);
    msgwrite_bars(m, mpp, pbars);
}

int
pciemgr::add_device(pciehdev_t *pdev)
{
    pmmsg_t *m;
    char *mp;

    //
    // Determine the total size of our msg payload.
    // NULL msg here indicates just count bytes in mp.
    //
    mp = NULL;
    msgwrite_dev_structs(NULL, &mp, pdev);
    if (pdev->pf) {
        pciehdev_t *vfdev = pdev->child;
        msgwrite_dev_structs(NULL, &mp, vfdev);
    }

    //
    // Alloc a msg of the determined size.
    //
    const size_t msglen = sizeof(pmmsg_dev_add_t) + (size_t)mp;
    pciemgrc_msgalloc(&m, msglen);
    // msg header
    m->hdr.msgtype = PMMSG_DEV_ADD;
    mp = (char *)&m->dev_add + sizeof(pmmsg_dev_add_t);

    //
    // Now copy our data to the msg.
    //
    msgwrite_dev_structs(m, &mp, pdev);
    if (pdev->pf) {
        pciehdev_t *vfdev = pdev->child;
        msgwrite_dev_structs(m, &mp, vfdev);
    }

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
        evhandlercb.memrd(evd->port, evd->lif, &evd->memrw_notify);
        break;
    case PCIEHDEV_EV_MEMWR_NOTIFY:
        evhandlercb.memwr(evd->port, evd->lif, &evd->memrw_notify);
        break;
    case PCIEHDEV_EV_SRIOV_NUMVFS:
        evhandlercb.sriov_numvfs(evd->port, evd->lif,
                                 evd->sriov_numvfs.numvfs);
        break;
    default:
        break;
    }
}
