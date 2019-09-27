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
    /* eth rxq */ [0] = UPD(SCHED_SET) | UPD(PICI_PISET) | UPD(PID_CHECK),
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

    assert(res->eth.devregs_stride == 0);

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
    /* eth rxq */ [0] = UPD(SCHED_SET) | UPD(PICI_PISET) | UPD(PID_CHECK),
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

    assert(res->eth.devregspa == 0 || res->eth.devregs_stride != 0);

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

static int
eth_initpf(pciehdev_t *pfdev, const pciehdev_res_t *pfres)
{
    if (pfres->eth.devregs_stride != 0) {
        pciesys_logdebug("eth_initpf: %s: devregs_stride %d\n",
                         pfres->name, pfres->eth.devregs_stride);
        return -1;
    }

    if (eth_bars(pfdev, pfres) < 0) {
        return -1;
    }
    if (eth_cfg(pfdev, pfres) < 0) {
        return -1;
    }
    return 0;
}

static int
eth_initvf(pciehdev_t *vfdev, const pciehdev_res_t *vfres)
{
    const int vfidx = pciehdev_get_vfidx(vfdev);
    pciehdev_res_t lvfresbuf, *lvfres = &lvfresbuf;
    pciehdev_ethres_t *lethres = &lvfres->eth;

    if (vfidx > 1) {
        if (vfres->eth.devregssz != 0 && vfres->eth.devregs_stride == 0) {
            pciesys_logdebug("eth_initvf: %s: devregs_stride %d\n",
                             vfres->name, vfres->eth.devregs_stride);
            return -1;
        }
    }

    /* vf adjust resources dependent on stride */
    *lvfres = *vfres;
    lvfres->lifb  = vfres->lifb  + vfidx * vfres->lifc;
    lvfres->intrb = vfres->intrb + vfidx * vfres->intrc;
    lvfres->cmbpa = vfres->cmbpa + vfidx * vfres->cmb_stride;
    lvfres->dsn   = vfres->dsn   + vfidx * vfres->dsn_stride;

    /* adjust eth resources dependent on stride */
    lethres->devregspa = lethres->devregspa + vfidx * lethres->devregs_stride;

    pciehdev_set_lifb(vfdev, lvfres->lifb);
    pciehdev_set_lifc(vfdev, lvfres->lifc);
    pciehdev_set_intrb(vfdev, lvfres->intrb);
    pciehdev_set_intrc(vfdev, lvfres->intrc);
    pciehdev_set_intrm(vfdev, lvfres->intrdmask);

    /* now go initialize with adjusted resources */
    if (eth_vfbars(vfdev, lvfres) < 0) {
        return -1;
    }
    if (eth_vfcfg(vfdev, lvfres) < 0) {
        return -1;
    }
    return 0;
}

static pciehdevice_t eth_device = {
    .type = PCIEHDEVICE_ETH,
    .initpf = eth_initpf,
    .initvf = eth_initvf,
};
PCIEHDEVICE_REGISTER(eth_device);
