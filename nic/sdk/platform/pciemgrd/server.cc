/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <cinttypes>

#include "platform/evutils/include/evutils.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pciehdevices/include/pciehdevices.h"
#include "platform/pcieport/include/pcieport.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/pciemgrutils/include/pciehdev.h"
#include "platform/pciemgrutils/include/pciehdev_impl.h"
#include "platform/pciemgrutils/include/pciehcfg_impl.h"
#include "platform/pciemgrutils/include/pciehbar_impl.h"
#include "platform/pciemgrd/pciemgrd_impl.hpp"
#include "platform/pciemgr_if/include/pmserver.h"

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

static void
log_pdev(pciehdev_t *pdev)
{
    pciesys_loginfo("device: %s port %d lifs %d-%d pf %d totalvfs %d\n",
                    pdev->name, pdev->port, pdev->lifb,
                    pdev->lifb + pdev->lifc - 1, pdev->pf, pdev->totalvfs);

    pciehbar_t *pbar = pdev->pbars->bars;
    for (int b = 0; b < PCIEHBAR_NBARS; b++, pbar++) {
        if (pbar->size == 0) continue;
        pciesys_logdebug("  bar %d nregs %d\n", b, pbar->nregs);
        pciehbarreg_t *preg = pbar->regs;
        for (int r = 0; r < pbar->nregs; r++, preg++) {
            pciesys_logdebug("    reg %d baroff 0x%09" PRIx64 " nprts %d\n",
                             r, preg->baroff, preg->nprts);
        }
    }
    pbar = &pdev->pbars->rombar;
    if (pbar->size) {
        pciesys_logdebug("  rom   nregs %d\n", pbar->nregs);
        pciehbarreg_t *preg = pbar->regs;
        for (int r = 0; r < pbar->nregs; r++, preg++) {
            pciesys_logdebug("    reg %d baroff 0x%09" PRIx64 " nprts %d\n",
                             r, preg->baroff, preg->nprts);
        }
    }
}

static size_t
msgdata_left(pmmsg_t *m, char *mp)
{
    return m->hdr.msglen - (mp - &m->msgdata);
}

static void
do_devres_add(pmmsg_t *m)
{
    pciehdevice_resources_t *pres = NULL;;
    pciehdev_t *pdev = NULL;
    int r;

    char *mp = (char *)&m->devres_add + sizeof(pmmsg_devres_add_t);

    if (msgdata_left(m, mp) < sizeof(pciehdevice_resources_t)) {
        pciesys_logerror("bad devres_add msg, res len %ld\n",
                         msgdata_left(m, mp));
        goto out;
    }

    pres = (pciehdevice_resources_t *)mp;
    mp += sizeof(pciehdevice_resources_t);
    if (msgdata_left(m, mp)) {
        pciesys_logerror("bad devres_add msg, %s remainder len %ld\n",
                         "devname", msgdata_left(m, mp));
        goto out;
    }

    pdev = pciehdevice_new(pres);
    if ((r = pciehdev_add(pdev)) < 0) {
        pciesys_logerror("devres_add: port %d %s lif %d: failed %d\n",
                         pdev->port, pdev->name, pdev->lifb, r);
    }

    /* log what we got */
    log_pdev(pdev);
    return;

 out:
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
    case PMMSG_DEVRES_ADD:
        do_devres_add(m);
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

#ifdef IRIS
#ifdef __aarch64__
static void
update_stats(void *arg)
{
    pciemgrenv_t *pme = (pciemgrenv_t *)arg;

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            update_pcie_metrics(port);
        }
    }
}
#endif
#endif

int
server_loop(pciemgrenv_t *pme)
{
    int r = 0;

    // logger_init();
    pciesys_loginfo("pciemgrd started\n");

#ifdef IRIS
#ifdef __aarch64__
    // connect to delphi
    delphi_client_start();
#endif
#endif

    pciemgrd_params(pme);

    pciesys_loginfo("---------------- config ----------------\n");
    pciesys_loginfo("enabled_ports 0x%x\n", pme->enabled_ports);
    pciesys_loginfo("port capabilities: gen%dx%d\n",
                    pme->params.cap_gen, pme->params.cap_width);
    pciesys_loginfo("vendorid: %04x\n", pme->params.vendorid);
    pciesys_loginfo("subvendorid: %04x\n", pme->params.subvendorid);
    pciesys_loginfo("subdeviceid: %04x\n", pme->params.subdeviceid);
    pciesys_loginfo("initmode: %d\n", pme->params.initmode);
    pciesys_loginfo("restart: %d\n", pme->params.restart);
    pciesys_loginfo("cpumask: %d\n", pme->cpumask);
    pciesys_loginfo("fifopri: %d\n", pme->fifopri);
    pciesys_loginfo("mlockall: %d\n", pme->mlockall);

    pciemgr_params_t *params = &pme->params;
    pciesys_loginfo("single_pnd: %d\n", params->single_pnd);

    pciesys_loginfo("---------------- config ----------------\n");

    pciemgrd_sys_init(pme);
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

#ifdef IRIS
#ifdef __aarch64__
    // initialize stats timer to update stats to delphi
    static evutil_timer stats_timer;
    evutil_timer_start(EV_DEFAULT_
                       &stats_timer, update_stats, pme, 10.0, 10.0);
#endif
#endif

    pciemgrs_open(NULL, pciemgr_msg_cb);
    pciesys_loginfo("pciemgrd ready\n");
    evutil_run(EV_DEFAULT);
    pciemgrs_close();

 close_dev_error_out:
    pciehdev_close();
 close_port_error_out:
    close_hostports();
 error_out:
    pciesys_loginfo("pciemgrd exit %d\n", r);
    return r;
}
