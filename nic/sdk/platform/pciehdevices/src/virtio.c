/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

static int
virtio_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehbars_t *pbars;
    pciehbar_t pbar;
    pciehbarreg_t preg;
    prt_t prt;

    pbars = pciehbars_new();

    /*****************
     * virtio resource io bar
     */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_IO;
    pbar.size = 0x20;
    pbar.cfgidx = 0;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_IO,
                pbar.size, /* barsize */
                pbar.size, /* prtsize */
                PMTF_RW);

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
                res->port,
                PMT_TYPE_MEM,
                pbar.size, /* barsize */
                0x1000,    /* prtsize */
                PMTF_RW);

    /* MSI-X Interrupt Table */
    prt_res_enc(&prt,
                intr_msixcfg_addr(res->intrb),
                intr_msixcfg_size(res->intrc),
                PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbars_set_msix_tbl(pbars, 1, 0x0000);

    /* MSI-X Interrupt PBA */
    prt_res_enc(&prt,
                intr_pba_addr(res->lifb),
                intr_pba_size(res->intrc),
                PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbars_set_msix_pba(pbars, 1, 0x1000);

    pciehbar_add_reg(&pbar, &preg);
    pciehbars_add_bar(pbars, &pbar);

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
virtio_cfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    pciehcfg_setconf_vendorid(pcfg, 0x1af4);    /* Vendor ID Redhat */
    pciehcfg_setconf_deviceid(pcfg, 0x1000);    /* transitional virtio-net */
    /* XXX */
    //pciehcfg_setconf_deviceid(pcfg, 0x2000);    /* XXXtest: keep drv away */
    /* XXX */
    pciehcfg_setconf_subvendorid(pcfg, 0x1af4); /* Subvendor ID Redhat */
    pciehcfg_setconf_subdeviceid(pcfg, 0x0001); /* Subdevice ID virtio-net */
    pciehcfg_setconf_classcode(pcfg, 0x020000); /*PCI_CLASS_NETWORK_ETHERNET*/
    pciehcfg_setconf_nintrs(pcfg, res->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, res->dsn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);
    pciehdev_set_cfg(pdev, pcfg);
    return 0;
}

static pciehdevice_t virtio_device = {
    .name = "virtio",
    .init_bars = virtio_bars,
    .init_cfg  = virtio_cfg,
};
PCIEHDEVICE_REGISTER(virtio_device);
