/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/misc/include/misc.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/utils/mpart_rsvd.hpp"

static void
init_memtun_bar(const int port, pciehbars_t *pbars)
{
    const char *memtun_addr_env = getenv("MEMTUN_ADDR");
    const char *memtun_size_env = getenv("MEMTUN_SIZE");
    // TODO. Need to read it from memory or file. Temporary fix for SDK compilation
    u_int64_t memtun_pa = (MREGION_BASE_ADDR + MREGION_MEMTUN_START_OFFSET);
    u_int64_t memtun_sz = MREGION_MEMTUN_SIZE;
    pciehbarreg_t preg;
    pciehbar_t pbar;
    prt_t prt;

    if (memtun_addr_env) {
        memtun_pa = strtoull(memtun_addr_env, NULL, 0);
        pciesys_loginfo("$MEMTUN_ADDR override 0x%"PRIx64"\n", memtun_pa);
    }
    if (memtun_size_env) {
        memtun_sz = strtoull(memtun_size_env, NULL, 0);
        pciesys_loginfo("$MEMTUN_SIZE override 0x%"PRIx64"\n", memtun_sz);
    }

    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = roundup_power2(memtun_sz);
    pbar.cfgidx = 0;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                port,
                PMT_TYPE_MEM,
                pbar.size,
                pbar.size, /* prtsize */
                PMTF_RW);

    prt_res_enc(&prt, memtun_pa, memtun_sz, PRT_RESF_PMVDIS);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    pciehbars_add_bar(pbars, &pbar);
}

static void
bridgedn_initialize_bars(pciehdev_t *pdev, const int port, const int memtun_en)
{
    pciehbars_t *pbars;

    if (memtun_en) {
        pbars = pciehbars_new();
        init_memtun_bar(port, pbars);
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
    pciehcfg_add_standard_vpd(pcfg);
    pciehcfg_add_standard_caps(pcfg);

    pciehdev_set_cfg(pdev, pcfg);
}

pciehdev_t *
pciehdev_bridgedn_new(const int port, const int memtun_en)
{
    static int instance;
    char name[32];
    pciehdev_t *pdev = pciehdev_new();

    snprintf(name, sizeof (name), "bridgedn%d", instance++);
    pciehdev_set_name(pdev, name);
    bridgedn_initialize_bars(pdev, port, memtun_en);
    bridgedn_initialize_cfg(pdev);
    return pdev;
}
