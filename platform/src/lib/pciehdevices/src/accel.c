/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "pci_ids.h"
#include "pciehost.h"
#include "pciehdevices.h"
#include "utils_impl.h"

#define PCI_DEVICE_ID_PENSANDO_ACCEL    0x8001

static void
init_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;

    /* bar 0 mem64 */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    {
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW;
        preg.paddr = 0xc1000000;
        preg.size = 0x1000;
        pciehbar_add_reg(&pbar, &preg);

        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW;
        preg.paddr = 0xc1008000;
        preg.size = 0x1000;
        pciehbar_add_reg(&pbar, &preg);

        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW;
        preg.paddr = 0xc1000400;
        preg.size = 0x1000;
        pciehbar_add_reg(&pbar, &preg);
    }
    pciehbars_add_bar(pbars, &pbar);

    /* msix bar */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    {
        /* MSI-X Interrupt Table */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = (PCIEHBARREGF_RW | PCIEHBARREGF_MSIX_TBL);
        preg.paddr = intr_msixcfg_addr(pres->intrb);
        preg.size = 0x1000;
        pciehbar_add_reg(&pbar, &preg);

        /* MSI-X Interrupt PBA */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = (PCIEHBARREGF_RD | PCIEHBARREGF_MSIX_PBA);
        preg.paddr = intr_pba_addr(pres->lif);
        preg.size = 0x1000;
        pciehbar_add_reg(&pbar, &preg);
    }
    pciehbars_add_bar(pbars, &pbar);
}

static void
init_cfg(pciehcfg_t *pcfg, pciehbars_t *pbars,
         const pciehdevice_resources_t *pres)
{
    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_ACCEL);
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
pciehdev_accel_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    initialize_bars(pdev, pres);
    initialize_cfg(pdev, pres);
    return pdev;
}
