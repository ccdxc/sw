/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <cinttypes>
#include <sys/types.h>

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciemgrutils/include/pciesys.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"
#include "pciemgrd_impl.hpp"

int
gold_loop(pciemgrenv_t *pme)
{
    int r;

    logger_init();
    pciesys_loginfo("pciemgrd started in gold mode\n");

    pciemgrd_params(pme);
    pciemgrd_logconfig(pme);
    pciemgrd_sys_init(pme);

    if ((r = pciehdev_open(&pme->params)) < 0) {
        pciesys_logerror("pciehdev_open failed: %d\n", r);
        goto close_port_error_out;
    }
    if ((r = open_hostports()) < 0) {
        goto error_out;
    }
    intr_init(pme);

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        pciehdevice_resources_t presources, *pres = &presources;
        pciehdev_t *pdev;

        //
        // Create a place-holder debug device on the bus for each
        // active port.  This prevents the host from putting the
        // port into low-power mode which effectively disables the
        // memtun bar on the bridge.
        //
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

    pciesys_loginfo("pciemgrd ready\n");
    evutil_run(EV_DEFAULT);

    pciehdev_close();
 close_port_error_out:
    close_hostports();
 error_out:
    pciesys_loginfo("pciemgrd exit gold mode %d\n", r);
    return r;
}
