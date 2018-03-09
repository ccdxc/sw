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

#define PCI_DEVICE_ID_PENSANDO_PCIESTRESS 0x8001

static void
init_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;
    u_int64_t sz = 0x1000;
    u_int64_t pa = 0x13f000000 + (sz * pres->port);

    /* bar mem64 */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    {
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW | PCIEHBARREGF_MEM;
        preg.paddr = pa;
        preg.size = sz;
        pciehbar_add_reg(&pbar, &preg);

        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = (PCIEHBARREGF_RW |
                      PCIEHBARREGF_NOTIFYRW);
        preg.paddr = pa;
        preg.size = sz;
        pciehbar_add_reg(&pbar, &preg);

        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = (PCIEHBARREGF_RW |
                      PCIEHBARREGF_INDIRECTRW);
        preg.paddr = pa;
        preg.size = sz;
        pciehbar_add_reg(&pbar, &preg);

        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW;
        preg.paddr = pa;
        preg.size = sz;
        pciehbar_add_reg(&pbar, &preg);
    }
    pciehbars_add_bar(pbars, &pbar);

    /* bar io */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_IO;
    {
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW | PCIEHBARREGF_MEM;
        preg.paddr = pa;
        preg.size = 0x20;
        pciehbar_add_reg(&pbar, &preg);

        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = (PCIEHBARREGF_RW |
                      PCIEHBARREGF_NOTIFYRW);
        preg.paddr = pa;
        preg.size = 0x20;
        pciehbar_add_reg(&pbar, &preg);

        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = (PCIEHBARREGF_RW |
                      PCIEHBARREGF_INDIRECTRW);
        preg.paddr = pa;
        preg.size = 0x20;
        pciehbar_add_reg(&pbar, &preg);

        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW;
        preg.paddr = pa;
        preg.size = 0x20;
        pciehbar_add_reg(&pbar, &preg);
    }
    pciehbars_add_bar(pbars, &pbar);
}

static void
init_cfg(pciehcfg_t *pcfg, pciehbars_t *pbars,
         const pciehdevice_resources_t *pres)
{
    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_PCIESTRESS);
    pciehcfg_setconf_classcode(pcfg, 0xff0000); /* unclassified device */

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
pciehdev_pciestress_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    initialize_bars(pdev, pres);
    initialize_cfg(pdev, pres);
    return pdev;
}
