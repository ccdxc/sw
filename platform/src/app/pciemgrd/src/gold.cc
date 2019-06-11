/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <cinttypes>
#include <sys/types.h>

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/pciemgrutils/include/pciesys.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"

#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"

static int poll_enabled;

static void
poll_hostports(pciemgrenv_t *pme)
{
    int port;

    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pcieport_poll(port);
        }
    }
}

int
gold_loop(pciemgrenv_t *pme)
{
    pciehdevice_resources_t presources, *pres = &presources;
    pciehdev_t *pdev;
    int r, port;

    logger_init();
    pciesys_loginfo("pciemgrd started in gold mode\n");

    pme->poll_port = 1;
    pme->poll_dev = 1;
    pciemgrd_params(pme);
    if ((r = open_hostports()) < 0) {
        goto error_out;
    }
    if ((r = pciehdev_open(&pme->params)) < 0) {
        pciesys_logerror("pciehdev_open failed: %d\n", r);
        goto close_port_error_out;
    }
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pciehdev_initialize(port);
            memset(pres, 0, sizeof(*pres));
            pres->type = PCIEHDEVICE_DEBUG;
            strncpy0(pres->pfres.name, "debug-gold", sizeof(pres->pfres.name));
            pres->pfres.port = port;
            pdev = pciehdevice_new(pres);
            pciehdev_add(pdev);
            pciehdev_finalize(port);
            pcieport_crs_off(port);
        }
    }

    /* Poll mode for these events. */
    pciehw_notify_poll_init();
    pciehw_indirect_poll_init();

    pciesys_loginfo("pciemgrd ready\n");
    poll_enabled = 1;
    while (poll_enabled) {

        poll_hostports(pme);
        pciehdev_poll();

        usleep(10000);
    }

    pciehdev_close();
 close_port_error_out:
    close_hostports();
 error_out:
    pciesys_loginfo("pciemgrd exit gold mode %d\n", r);
    return r;
}
