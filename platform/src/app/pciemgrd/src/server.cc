/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <cinttypes>

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciemgrutils/include/pciehdev.h"
#include "nic/sdk/platform/pciemgrutils/include/pciehdev_impl.h"
#include "nic/sdk/platform/pciemgrutils/include/pciehcfg_impl.h"
#include "nic/sdk/platform/pciemgrutils/include/pciehbar_impl.h"
#include "platform/src/app/pciemgrd/src/pciemgrd_impl.hpp"
#include "platform/src/lib/pciemgr_if/include/pmserver.h"

#include "delphic.h"

static void
do_open(pmmsg_t *m)
{
    pciesys_loginfo("open: %s\n", m->open.name);
}

static void
do_initialize(pmmsg_t *m)
{
    const int port = m->initialize.port;
    int r;

    pciesys_loginfo("initialize: port %d\n", port);
    r = pciehdev_initialize(port);
    if (r < 0) {
        pciesys_logerror("pciehdev_initialize(%d) failed %d\n", port, r);
    }
}

static void
do_finalize(pmmsg_t *m)
{
    const int port = m->finalize.port;
    int r;

    pciesys_loginfo("finalize: port %d\n", port);
    r = pciehdev_finalize(port);
    if (r < 0) {
        pciesys_logerror("pciehdev_finalize(%d) failed %d\n", port, r);
    }

    r = pcieport_crs_off(port);
    if (r < 0) {
        pciesys_logerror("pcieport_crs_off(%d) failed %d\n", port, r);
    }

    // log some info about the final config
    pciehw_dev_show(0, NULL);
    pciehw_bar_show(0, NULL);
    pciesys_loginfo("finalize: port %d ready\n", port);
}

static size_t
msgdata_left(pmmsg_t *m, char *mp)
{
    return m->hdr.msglen - (mp - &m->msgdata);
}

static int
msgread_prt(pmmsg_t *m, char **mpp, prt_t *prt)
{
    if (msgdata_left(m, *mpp) < sizeof(prt_t)) {
        pciesys_logerror("bad dev_add msg, prt len %ld\n",
                         msgdata_left(m, *mpp));
        goto out;
    }

    memcpy(prt, *mpp, sizeof(prt_t));
    *mpp += sizeof(prt_t);
    return 0;

 out:
    return -1;
}

static int
msgread_reg(pmmsg_t *m, char **mpp, pciehbarreg_t *preg)
{
    int nprts = 0;

    if (msgdata_left(m, *mpp) < sizeof(pciehbarreg_t)) {
        pciesys_logerror("bad dev_add msg, reg len %ld\n",
                         msgdata_left(m, *mpp));
        goto out;
    }

    memcpy(preg, *mpp, sizeof(pciehbarreg_t));
    *mpp += sizeof(pciehbarreg_t);

    nprts = preg->nprts;
    preg->nprts = 0;
    preg->prts = NULL;

    for (int p = 0; p < nprts; p++) {
        prt_t prt;
        if (msgread_prt(m, mpp, &prt) < 0) goto out;
        pciehbarreg_add_prt(preg, &prt);
    }
    return 0;

 out:
    return -1;
}

static int
msgread_bar(pmmsg_t *m, char **mpp, pciehbar_t *pbar)
{
    int nregs = pbar->nregs;

    pbar->nregs = 0;
    pbar->regs = NULL;
    for (int r = 0; r < nregs; r++) {
        pciehbarreg_t reg;

        if (msgread_reg(m, mpp, &reg) < 0) goto out;
        pciehbar_add_reg(pbar, &reg);
    }
    return 0;

 out:
    return -1;
}

static pciehbars_t *
msgread_bars(pmmsg_t *m, char **mpp)
{
    pciehbars_t *pbars;

    if (msgdata_left(m, *mpp) < sizeof(pciehbars_t)) {
        pciesys_logerror("bad dev_add msg, bars %d\n", m->hdr.msglen);
        return NULL;
    }

    pbars = pciehbars_new();
    memcpy(pbars, *mpp, sizeof(pciehbars_t));
    *mpp += sizeof(pciehbars_t);

    pciehbar_t *pbar = pbars->bars;
    for (int b = 0; b < PCIEHBAR_NBARS; b++, pbar++) {
        if (pbar->size == 0) continue;
        if (msgread_bar(m, mpp, pbar) < 0) goto out;
        pciehbars_add_bar(pbars, pbar);
    }
    /* rombar */
    pbar = &pbars->rombar;
    if (pbar->size) {
        if (msgread_bar(m, mpp, pbar) < 0) goto out;
        pciehbars_add_rombar(pbars, pbar);
    }
    return pbars;

 out:
    if (pbars) pciehbars_delete(pbars);
    return NULL;
}

static pciehcfg_t *
msgread_cfg(pmmsg_t *m, char **mpp)
{
    pciehcfg_t *pcfg;

    if (msgdata_left(m, *mpp) < (sizeof(pciehcfg_t) + PCIEHCFGSZ * 2)) {
        pciesys_logerror("bad dev_add msg, cfg %d\n", m->hdr.msglen);
        return NULL;
    }

    pcfg = pciehcfg_new();
    memcpy(pcfg, *mpp, sizeof(pciehcfg_t));
    *mpp += sizeof(pciehcfg_t);

    pcfg->cur = (u_int8_t *)(pcfg + 1);
    pcfg->msk = pcfg->cur + PCIEHCFGSZ;

    memcpy(pcfg->cur, *mpp, PCIEHCFGSZ);
    *mpp += PCIEHCFGSZ;
    memcpy(pcfg->msk, *mpp, PCIEHCFGSZ);
    *mpp += PCIEHCFGSZ;

    return pcfg;
}

static pciehdev_t *
msgread_dev(pmmsg_t *m, char **mpp)
{
    pciehdev_t *pdev;

    if (msgdata_left(m, *mpp) < sizeof(pciehdev_t)) {
        pciesys_logerror("bad dev_add msg, dev len %d\n", m->hdr.msglen);
        return NULL;
    }

    pdev = pciehdev_new();
    memcpy(pdev, *mpp, sizeof(pciehdev_t));
    *mpp += sizeof(pciehdev_t);
    pdev->pbars = NULL;
    pdev->pcfg = NULL;
    pdev->phwdev = NULL;
    pdev->parent = NULL;
    pdev->peer = NULL;
    pdev->child = NULL;

    return pdev;
}

static pciehdev_t *
msgread_dev_structs(pmmsg_t *m, char **mpp)
{
    pciehdev_t *pdev = NULL;
    pciehcfg_t *pcfg = NULL;
    pciehbars_t *pbars = NULL;

    pdev = msgread_dev(m, mpp);
    if (pdev == NULL) goto out;

    pcfg = msgread_cfg(m, mpp);
    if (pcfg == NULL) goto out;

    pciehdev_set_cfg(pdev, pcfg);
    pcfg = NULL; /* now owned by dev */

    pbars = msgread_bars(m, mpp);
    if (pbars == NULL) goto out;

    pciehdev_set_bars(pdev, pbars);
    pbars = NULL; /* now owned by dev */

    return pdev;

 out:
    if (pbars) pciehbars_delete(pbars);
    if (pcfg) pciehcfg_delete(pcfg);
    if (pdev) {
        pciesys_logerror("%s: failed dev structs\n", pciehdev_get_name(pdev));
        pciehdev_delete(pdev);
    }
    return NULL;
}

static void
do_dev_add(pmmsg_t *m)
{
    int r;

    char *mp = (char *)&m->dev_add + sizeof(pmmsg_dev_add_t);

    pciehdev_t *pdev = msgread_dev_structs(m, &mp);
    if (pdev == NULL) goto out;

    if (pdev->pf) {
        pciehdev_t *vfdev = msgread_dev_structs(m, &mp);
        if (vfdev == NULL) {
            pciesys_logerror("bad dev_add msg, %s vf len %ld\n",
                             pciehdev_get_name(pdev), msgdata_left(m, mp));
            goto out;
        }

        pciehdev_linkvf(pdev, vfdev, pdev->totalvfs);
    }

    if (msgdata_left(m, mp)) {
        pciesys_logerror("bad dev_add msg, %s remainder len %ld\n",
                         pciehdev_get_name(pdev), msgdata_left(m, mp));
        goto out;
    }

    /* log what we got */
    {
        pciehbar_t *pbar = pdev->pbars->bars;
        int b;

        pciesys_loginfo("dev_add: port %d %s lifs %d-%d pf %d totalvfs %d\n",
                        pdev->port, pdev->name, pdev->lifb,
                        pdev->lifb + pdev->lifc - 1, pdev->pf, pdev->totalvfs);

        for (b = 0; b < PCIEHBAR_NBARS; b++, pbar++) {
            if (pbar->size == 0) continue;
            pciesys_logdebug("  bar %d nregs %d\n", b, pbar->nregs);
            pciehbarreg_t *preg = pbar->regs;
            for (r = 0; r < pbar->nregs; r++, preg++) {
                pciesys_logdebug("    reg %d baroff 0x%09" PRIx64
                                 " nprts %d\n",
                                 r, preg->baroff,
                                 preg->nprts);
            }
        }
        pbar = &pdev->pbars->rombar;
        if (pbar->size) {
            pciesys_logdebug("  rom   nregs %d\n", pbar->nregs);
            pciehbarreg_t *preg = pbar->regs;
            for (r = 0; r < pbar->nregs; r++, preg++) {
                pciesys_logdebug("    reg %d baroff 0x%09" PRIx64
                                 " nprts %d\n",
                                 r, preg->baroff,
                                 preg->nprts);
            }
        }
    }

    if ((r = pciehdev_add(pdev)) < 0) {
        pciesys_logerror("dev_add: port %d %s lif %d: failed %d\n",
                         pdev->port, pdev->name, pdev->lifb, r);
    }
    return;

 out:
    if (pdev) {
        pciesys_logerror("%s: dev_add failed\n", pciehdev_get_name(pdev));
        pciehdev_delete(pdev);
    }
    pciesys_logerror("dev_add failed msglen %u\n", m->hdr.msglen);
}

static void
pciemgr_msg_cb(pmmsg_t *m)
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
dev_evhandler(const pciehdev_eventdata_t *evd)
{
    pmmsg_t *m;
    const size_t msglen = (sizeof(pmmsg_event_t) +
                           sizeof(pciehdev_eventdata_t));
    pciemgrs_msgalloc(&m, msglen);

    m->hdr.msgtype = PMMSG_EVENT;

    char *mp = (char *)&m->event + sizeof(pmmsg_event_t);
    memcpy(mp, evd, sizeof(pciehdev_eventdata_t));

    // msg complete - send it
    pciemgrs_msgsend(m);
    pciemgrs_msgfree(m);
}

#ifdef __aarch64__
static void
update_stats(void *arg)
{
    pciemgrenv_t *pme = pciemgrenv_get();

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            delphi_update_pcie_metrics(port);
        }
    }
}
#endif

int
server_loop(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    int r = 0;

    logger_init();
    pciesys_loginfo("pciemgrd started\n");

#ifdef __aarch64__
    // connect to delphi
    delphi_client_start();
#endif

    pciemgrd_params(pme);

    pciesys_loginfo("---------------- config ----------------\n");
    pciesys_loginfo("enabled_ports 0x%x\n", pme->enabled_ports);
    pciesys_loginfo("gen%dx%d\n", pme->params.cap_gen, pme->params.cap_width);
    pciesys_loginfo("vendorid: %04x\n", pme->params.vendorid);
    pciesys_loginfo("subvendorid: %04x\n", pme->params.subvendorid);
    pciesys_loginfo("subdeviceid: %04x\n", pme->params.subdeviceid);
    pciesys_loginfo("---------------- config ----------------\n");

    if ((r = open_hostports()) < 0) {
        goto error_out;
    }
    if ((r = pciehdev_open(&pme->params)) < 0) {
        pciesys_logerror("pciehdev_open failed: %d\n", r);
        goto close_port_error_out;
    }
    if ((r = pciehdev_register_event_handler(dev_evhandler)) < 0) {
        pciesys_logerror("pciehdev_register_event_handler failed %d\n", r);
        goto close_dev_error_out;
    }
    intr_init();

#ifdef __aarch64__
    // initialize stats timer to update stats to delphi
    static evutil_timer stats_timer;
    evutil_timer_start(&stats_timer, update_stats, NULL, 10.0, 10.0);
#endif

    pciemgrs_open(NULL, pciemgr_msg_cb);
    pciesys_loginfo("pciemgrd ready\n");
    evutil_run();
    pciemgrs_close();

 close_dev_error_out:
    pciehdev_close();
 close_port_error_out:
    close_hostports();
 error_out:
    pciesys_loginfo("pciemgrd exit %d\n", r);
    return r;
}
