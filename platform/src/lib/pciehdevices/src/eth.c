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
#include "utils_impl.h"

static void
initialize_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;

    /* resource bar - mem64 */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    {
        /* Device Cmd Regs */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW;
        preg.paddr = 0x1000;
        preg.size = 0x1000;
        preg.align = 0;
        pciehbar_add_reg(&pbar, &preg);

        /* Device Cmd Doorbell */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_WR;
        preg.paddr = 0x2000;
        preg.size = 0x1000;
        preg.align = 0;
        pciehbar_add_reg(&pbar, &preg);

        /* Interrupt Control regs */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RW;
        preg.paddr = intr_drvcfg_addr(pres->intrb);
        preg.size = 0x1000;
        preg.align = 0;
        pciehbar_add_reg(&pbar, &preg);

        /* Interrupt Status regs */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RD;
        preg.paddr = intr_pba_addr(pres->intrb);
        preg.size = 0x1000;
        preg.align = 0;
        pciehbar_add_reg(&pbar, &preg);

        /* <reserved> */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RD;
        preg.paddr = 0;
        preg.size = 0x1000;
        preg.align = 0;
        pciehbar_add_reg(&pbar, &preg);

        /* <reserved> */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = PCIEHBARREGF_RD;
        preg.paddr = 0;
        preg.size = 0x1000;
        preg.align = 0;
        pciehbar_add_reg(&pbar, &preg);

        /* MSI-X Interrupt Table */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = (PCIEHBARREGF_RW | PCIEHBARREGF_MSIX_TBL);
        preg.paddr = intr_msixcfg_addr(pres->intrb);
        preg.size = 0x1000;
        preg.align = 0;
        pciehbar_add_reg(&pbar, &preg);

        /* MSI-X Interrupt PBA */
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_RES;
        preg.flags = (PCIEHBARREGF_RD | PCIEHBARREGF_MSIX_PBA);
        preg.paddr = intr_pba_addr(pres->intrb);
        preg.size = 0x1000;
        preg.align = 0;
        pciehbar_add_reg(&pbar, &preg);
    }
    pciehbars_add_bar(pbars, &pbar);

    /* doorbell bar - mem64 */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    {
        memset(&preg, 0, sizeof(preg));
        preg.regtype = PCIEHBARREGT_DB64;
        preg.flags = PCIEHBARREGF_WR;
        preg.paddr = 0x100000;
        preg.size = 0x1000;
        preg.align = 0;
        preg.qtyshift = 3;
        preg.qtywidth = 3;
        /* eth rxq */
        preg.upd[0] = (/* PCIEHBARUPD_SCHED_NONE | */
                       PCIEHBARUPD_PICI_PISET |
                       PCIEHBARUPD_PID_CHECK);
        /* eth txq */
        preg.upd[1] = (PCIEHBARUPD_SCHED_SET |
                       PCIEHBARUPD_PICI_PISET |
                       PCIEHBARUPD_PID_CHECK);
        /* rdma sq */
        preg.upd[2] = (PCIEHBARUPD_SCHED_SET |
                       PCIEHBARUPD_PICI_PISET |
                       PCIEHBARUPD_PID_CHECK);
        /* rdma rq */
        preg.upd[3] = (PCIEHBARUPD_SCHED_SET |
                       PCIEHBARUPD_PICI_PISET |
                       PCIEHBARUPD_PID_CHECK);
        /* admin q */
        preg.upd[4] = (PCIEHBARUPD_SCHED_SET |
                       PCIEHBARUPD_PICI_PISET |
                       PCIEHBARUPD_PID_CHECK);
        /* rdma cq */
        preg.upd[5] = (/* PCIEHBARUPD_SCHED_NONE | */
                       PCIEHBARUPD_PICI_PISET |
                       PCIEHBARUPD_PID_CHECK);
        /* rdma eq */
        preg.upd[6] = (/* PCIEHBARUPD_SCHED_NONE | */
                       PCIEHBARUPD_PICI_PISET |
                       PCIEHBARUPD_PID_CHECK);
        /* unused */
        preg.upd[7] = PCIEHBARUPD_NONE;

        pciehbar_add_reg(&pbar, &preg);
    }
    pciehbars_add_bar(pbars, &pbar);
}

static void
initialize_cfg(pciehcfg_t *pcfg, pciehbars_t *pbars,
               const pciehdevice_resources_t *pres)
{
    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_ENET);
    pciehcfg_setconf_classcode(pcfg, 0x020000); /*PCI_CLASS_NETWORK_ETHERNET*/
    pciehcfg_setconf_nintrs(pcfg, pres->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, 0x123456789abcdeffULL);
    pciehcfg_setconf_fnn(pcfg, pres->fnn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);
}

static void
enet_initialize_bars(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehbars_t *pbars;

    pbars = pciehbars_new();
    initialize_bars(pbars, pres);
    pciehdev_set_bars(pdev, pbars);
}

static void
enet_initialize_cfg(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    initialize_cfg(pcfg, pbars, pres);
    pciehdev_set_cfg(pdev, pcfg);
}

pciehdev_t *
pciehdev_enet_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    enet_initialize_bars(pdev, pres);
    enet_initialize_cfg(pdev, pres);
    return pdev;
}
