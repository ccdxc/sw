/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pcieport/include/pcieport.h"
#include "pcieport_impl.h"

int
pcieport_config_powerup(pcieport_t *p)
{
    int r;

    if (!p->config) return -1;

    if (p->host) {
        r = pcieportpd_config_host(p);
    } else {
        r = pcieportpd_config_rc(p);
    }
    return r;
}

int
pcieport_config_powerdown(pcieport_t *p)
{
    return pcieportpd_config_powerdown(p);
}

int
pcieport_config_linkup(pcieport_t *p)
{
    pcieportpd_mac_set_ids(p);
    return 0;
}
