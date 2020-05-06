/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/time.h>

#include "lib/catalog/catalog.hpp"
#include "nic/sdk/platform/pciemgrutils/include/pciesys.h"
#include "nic/sdk/platform/pcieport/include/portmap.h"

int
portmap_init_from_catalog()
{
    static int inited;

    if (inited) return 0;

    portmap_init();
#ifdef __aarch64__
    sdk::lib::catalog *catalog = sdk::lib::catalog::factory();
    if (catalog == NULL) {
        pciesys_logerror("no catalog!\n");
        return -1;
    }

    int nportspecs = catalog->pcie_nportspecs();
    for (int i = 0; i < nportspecs; i++) {
        pcieport_spec_t ps = { 0 };
        ps.host  = catalog->pcie_host(i);
        ps.port  = catalog->pcie_port(i);
        ps.gen   = catalog->pcie_gen(i);
        ps.width = catalog->pcie_width(i);
        if (portmap_addhost(&ps) < 0) {
            pciesys_logerror("portmap_add i %d h%d p%d gen%dx%d failed\n",
                             i, ps.host, ps.port, ps.gen, ps.width);
            return -1;
        }
    }
    sdk::lib::catalog::destroy(catalog);
#else
    pcieport_spec_t ps = { 0 };
    ps.host  = 0;
    ps.port  = 0;
    ps.gen   = 3;
    ps.width = 16;

    if (portmap_addhost(&ps) < 0) {
        pciesys_logerror("portmap_add h%d p%d gen%dx%d failed\n",
                         ps.host, ps.port, ps.gen, ps.width);
        return -1;
    }
#endif
    inited = 1;
    return 0;
}

int
default_pcieport()
{
    portmap_init_from_catalog();
    const int hostport = 0;
    const int pcieport = portmap_pcieport(hostport);
    return pcieport >= 0 ? pcieport : 0;
}
