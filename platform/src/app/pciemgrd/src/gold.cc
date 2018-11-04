/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>

#include "pciesys.h"
#include "pciehdevices.h"
#include "pciehw_dev.h"
#include "pcieport.h"
#include "pciemgrd_impl.hpp"

static int poll_enabled;

static void
poll_hostports(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    int port;

    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pcieport_poll(port);
        }
    }
}

int
gold_loop(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    pciehdevice_resources_t presources, *pres = &presources;
    pciehdev_t *pdev;
    int r, port;

    logger_init();
    pciesys_loginfo("pciemgrd started in gold mode\n");

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
            pres->port = port;
            pdev = pciehdev_debug_new("debug-gold", pres);
            pciehdev_add(pdev);
            pciehdev_finalize(port);
            pcieport_crs_off(port);
        }
    }

    pciesys_loginfo("pciemgrd ready\n");
    poll_enabled = 1;
    while (poll_enabled) {

        poll_hostports();
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
