/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#include "pci_ids.h"
#include "pciehost.h"
#include "pciehsys.h"

static void
init_memtun_bar(pciehbars_t *pbars)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;
    const char *memtun_addr_env = getenv("MEMTUN_ADDR");
    const char *memtun_size_env = getenv("MEMTUN_SIZE");
    u_int64_t memtun_pa = 0x13b000000;
    u_int64_t memtun_sz = 0x100000;

    if (memtun_addr_env) {
        memtun_pa = strtoull(memtun_addr_env, NULL, 0);
        pciehsys_log("$MEMTUN_ADDR override 0x%"PRIx64"\n", memtun_pa);
    }
    if (memtun_size_env) {
        memtun_sz = strtoull(memtun_size_env, NULL, 0);
        pciehsys_log("$MEMTUN_SIZE override 0x%"PRIx64"\n", memtun_sz);
    }

    memset(&pbar, 0, sizeof(pbar));
    memset(&preg, 0, sizeof(preg));
    preg.regtype = PCIEHBARREGT_RES;
    preg.flags = PCIEHBARREGF_RW;
    preg.paddr = memtun_pa;
    preg.size = memtun_sz;
    pciehbar_add_reg(&pbar, &preg);

    pbar.type = PCIEHBARTYPE_MEM64;
    pciehbars_add_bar(pbars, &pbar);
}

static void
bridgedn_initialize_bars(pciehdev_t *pdev, const int memtun_en)
{
    pciehbars_t *pbars;

    if (memtun_en) {
        pbars = pciehbars_new();
        init_memtun_bar(pbars);
        pciehdev_set_bars(pdev, pbars);
    }
}

static void
bridgedn_initialize_cfg(pciehdev_t *pdev)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_BRDN);
    pciehcfg_setconf_classcode(pcfg, 0x060400); /* PCI_CLASS_BRIDGE_PCI */
    pciehcfg_setconf_bridgedn(pcfg, 1);

    pciehcfg_sethdr_type1(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);

    pciehdev_set_cfg(pdev, pcfg);
}

pciehdev_t *
pciehdev_bridgedn_new(const int memtun_en)
{
    static int instance;
    char name[32];

    snprintf(name, sizeof (name), "bridgedn%d", instance++);
    pciehdev_t *pdev = pciehdev_new(name, NULL);
    bridgedn_initialize_bars(pdev, memtun_en);
    bridgedn_initialize_cfg(pdev);
    return pdev;
}
