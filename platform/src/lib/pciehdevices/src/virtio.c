/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "pci_ids.h"
#include "intrutils.h"
#include "pciemgrutils.h"
#include "pciehdevices.h"

static void
init_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;
    prt_t prt;

    /*****************
     * virtio resource io bar
     */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_IO;
    pbar.size = 0x20;
    pbar.cfgidx = 0;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                pres->port,
                PMT_TYPE_IO,
                pbar.size, /* barsize */
                pbar.size, /* prtsize */
                PMT_BARF_RW);

    /* XXX */
    prt_res_enc(&prt, 0, 0, PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);

    pciehbar_add_reg(&pbar, &preg);
    pciehbars_add_bar(pbars, &pbar);

    /*****************
     * msix bar - mem64
     */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = 2 * 0x1000;
    pbar.cfgidx = 1;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                pres->port,
                PMT_TYPE_MEM,
                pbar.size, /* barsize */
                0x1000,    /* prtsize */
                PMT_BARF_RW);

    /* MSI-X Interrupt Table */
    prt_res_enc(&prt,
                intr_msixcfg_addr(pres->intrb),
                intr_msixcfg_size(pres->intrc),
                PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbars_set_msix_tbl(pbars, 1, 0x0000);

    /* MSI-X Interrupt PBA */
    prt_res_enc(&prt,
                intr_pba_addr(pres->lif),
                intr_pba_size(pres->intrc),
                PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbars_set_msix_pba(pbars, 1, 0x1000);

    pciehbar_add_reg(&pbar, &preg);
    pciehbars_add_bar(pbars, &pbar);
}

static void
init_cfg(pciehcfg_t *pcfg, pciehbars_t *pbars,
         const pciehdevice_resources_t *pres)
{
    pciehcfg_setconf_vendorid(pcfg, 0x1af4);    /* Vendor ID Redhat */
    pciehcfg_setconf_deviceid(pcfg, 0x1000);    /* transitional virtio-net */
    /* XXX */
    //pciehcfg_setconf_deviceid(pcfg, 0x2000);    /* XXXtest: keep drv away */
    /* XXX */
    pciehcfg_setconf_subvendorid(pcfg, 0x1af4); /* Subvendor ID Redhat */
    pciehcfg_setconf_subdeviceid(pcfg, 0x0001); /* Subdevice ID virtio-net */
    pciehcfg_setconf_classcode(pcfg, 0x020000); /*PCI_CLASS_NETWORK_ETHERNET*/
    pciehcfg_setconf_nintrs(pcfg, pres->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, pres->dsn);
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
pciehdev_virtio_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    initialize_bars(pdev, pres);
    initialize_cfg(pdev, pres);
    return pdev;
}
