/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

static int
eth_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    const u_int8_t upd[8] = {
    /* make this table a bit more compact */
#define UPD(U)  PRT_UPD_##U
    /* eth rxq */ [0] = UPD(SCHED_NONE) | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* eth txq */ [1] = UPD(SCHED_SET)  | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* rdma sq */ [2] = UPD(SCHED_SET)  | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* rdma rq */ [3] = UPD(SCHED_SET)  | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* admin q */ [4] = UPD(SCHED_SET)  | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* rdma cq */ [5] = UPD(SCHED_SET)  | UPD(PICI_CISET) | UPD(PID_CHECK),
    /* rdma eq */ [6] = UPD(SCHED_NONE) | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* unused */  [7] = 0,
#undef UPD
    };
    pciehbars_t *pbars;

    pbars = pciehbars_new();

    add_common_resource_bar(pbars, res);
    add_common_doorbell_bar(pbars, res, upd);
    add_common_cmb_bar(pbars, res);
    add_common_rom_bar(pbars, res);

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
eth_vfbars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    const u_int8_t upd[8] = {
    /* make this table a bit more compact */
#define UPD(U)  PRT_UPD_##U
    /* eth rxq */ [0] = UPD(SCHED_NONE) | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* eth txq */ [1] = UPD(SCHED_SET)  | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* rdma sq */ [2] = UPD(SCHED_SET)  | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* rdma rq */ [3] = UPD(SCHED_SET)  | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* admin q */ [4] = UPD(SCHED_SET)  | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* rdma cq */ [5] = UPD(SCHED_SET)  | UPD(PICI_CISET) | UPD(PID_CHECK),
    /* rdma eq */ [6] = UPD(SCHED_NONE) | UPD(PICI_PISET) | UPD(PID_CHECK),
    /* unused */  [7] = 0,
#undef UPD
    };
    pciehbars_t *pbars;

    pbars = pciehbars_new();

    add_common_resource_bar(pbars, res);
    add_common_doorbell_bar(pbars, res, upd);
    add_common_cmb_bar(pbars, res);

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
eth_cfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);
    pciehbars_t *vfbars = NULL;

    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_ENET);
    pciehcfg_setconf_classcode(pcfg, 0x020000); /*PCI_CLASS_NETWORK_ETHERNET*/
    pciehcfg_setconf_nintrs(pcfg, res->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, res->dsn);
    if (pciehdev_is_pf(pdev)) {
        pciehcfg_setconf_pf(pcfg, 1);
        pciehcfg_setconf_totalvfs(pcfg, pciehdev_get_totalvfs(pdev));
        pciehcfg_setconf_vfdeviceid(pcfg, PCI_DEVICE_ID_PENSANDO_ENETVF);
        vfbars = pciehdev_get_vfbars(pdev);
    }

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_pfcaps(pcfg, vfbars);

    pciehdev_set_cfg(pdev, pcfg);
    return 0;
}

static int
eth_vfcfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    pciehcfg_setconf_vf(pcfg, 1);
    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_ENETVF);
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

static pciehdevice_t eth_device = {
    .name = "eth",
    .init_bars   = eth_bars,
    .init_cfg    = eth_cfg,
    .init_vfbars = eth_vfbars,
    .init_vfcfg  = eth_vfcfg,
};
PCIEHDEVICE_REGISTER(eth_device);
