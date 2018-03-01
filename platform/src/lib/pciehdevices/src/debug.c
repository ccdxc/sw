/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "pci_ids.h"
#include "pciehost.h"
#include "pciehdevices.h"

#define PCI_DEVICE_ID_PENSANDO_DEBUG 0x8000

static void
init_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;

    /* bar mem64 */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    {
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW | PCIEHBARREGF_MEM;
        preg.paddr = 0x80000000;
        preg.size = 128 * 1024 * 1024;
        pciehbar_add_reg(&pbar, &preg);
    }
    pciehbars_add_bar(pbars, &pbar);

    /* bar mem64 */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    {
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW;
        preg.paddr = 0;
        preg.size = 128 * 1024 * 1024;
        pciehbar_add_reg(&pbar, &preg);
    }
    pciehbars_add_bar(pbars, &pbar);
}

static void
init_cfg(pciehcfg_t *pcfg, pciehbars_t *pbars,
         const pciehdevice_resources_t *pres)
{
    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_DEBUG);
    pciehcfg_setconf_classcode(pcfg, 0xff0000); /* unclassified device */
    pciehcfg_setconf_nintrs(pcfg, pres->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_fnn(pcfg, pres->fnn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);
}

static void
initialize_bars(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehbars_t *pbars;

    pbars = pciehbars_new();
    init_bars(pbars, pres);
    pciehdev_set_bars(pdev, pbars);
}

static void
initialize_cfg(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    init_cfg(pcfg, pbars, pres);
    pciehdev_set_cfg(pdev, pcfg);
}

pciehdev_t *
pciehdev_debug_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    initialize_bars(pdev, pres);
    initialize_cfg(pdev, pres);
    return pdev;
}
