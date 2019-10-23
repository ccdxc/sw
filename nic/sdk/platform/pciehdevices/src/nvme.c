/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/misc/include/misc.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

/*
 * NVMe Resource bar is
 *
 *     +0x0000 device registers
 *     +0x1000 sq,cq doorbells
 *     +0x2000 MSIX Interrupt Table
 *     +0x3000 MSIX Interrupt PBA
 */
static int
nvme_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    const u_int8_t upd[8] = {
        /* make this table a bit more compact */
#define UPD(U)  PRT_UPD_##U
        /* nvme sq */ [0] = UPD(SCHED_SET)  | UPD(PICI_PISET),
        /* nvme cq */ [1] = UPD(SCHED_NONE) | UPD(PICI_CISET),
#undef UPD
    };
    u_int32_t msixtbloff, msixpbaoff;
    pciehbars_t *pbars;
    pciehbar_t pbar;
    pciehbarreg_t preg;
    prt_t prt;
    u_int32_t nqids;

    pbars = pciehbars_new();

    /*****************
     * resource bar
     */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = 0x4000;
    pbar.cfgidx = 0;

    /*****************
     * +0x0000 device registers */
    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                0x1000,         /* pmtsize */
                0x1000,         /* prtsize */
                PMTF_RW);
    prt_res_enc(&prt, res->nvme.regspa, res->nvme.regssz, PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x1000 doorbells */
    memset(&preg, 0, sizeof(preg));
    preg.baroff = 0x1000;

    /*
     * The doorbell region scales with the number of queues we support.
     * Max queues per NVMe spec is 64k (0x10000), but Capri allows only
     * 32k because Capri 32b doorbells pass only 15 bits of "index"
     * through the doorbell, so max "index" is 0x7fff.
     *
     * Our packed 32b doorbells can get 1024 in a 4k page.
     * Each qid requires a submission q and a completion q.
     * So we can support up to 512 q's per 4k page.
     * (XXX For now, limit to 512 q's.  Grow bar for more doorbells.)
     */
    nqids = MIN(res->nvme.qidc, 0x10000);
    nqids = MIN(nqids, 512); /* XXX limit to 512 q's */

    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                0x1000,         /* pmtsize */
                0x1000,         /* prtsize */
                PMTF_WR);
    /* bit2: 2 qtypes: 0=submission q, 1=completion q */
    pmt_bar_setr_qtype(&preg.pmt, 2, 1);
    /* bit3: qid start */
    pmt_bar_setr_qid(&preg.pmt, 3, 9);

    prt_db32_enc(&prt, res->lifb, upd);
    /* want 16 bits, but Capri errata allows only 15 */
    prt_db_idxparams(&prt, 15, 0);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x2000 MSIX Interrupt Table
     * +0x3000 MSIX Interrupt PBA */
    assert(res->intrc <= 256); /* 256 intrs per page (XXX grow bar) */
    msixtbloff = 0x2000;
    msixpbaoff = 0x3000;
    add_msix_region(pbars, &pbar, res, msixtbloff, msixpbaoff);

    pciehbars_add_bar(pbars, &pbar);

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
nvme_pfbars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    return nvme_bars(pdev, res);
}

static int
nvme_vfbars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    return nvme_bars(pdev, res);
}

static int
nvme_cfg(pciehdev_t *pdev, const pciehdev_res_t *res, const u_int16_t devid)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);
    pciehbars_t *vfbars = NULL;

    pciehcfg_setconf_vf(pcfg, res->is_vf);
    pciehcfg_setconf_deviceid(pcfg, devid);
    pciehcfg_setconf_classcode(pcfg, 0x010802);
    if (res->nvme.classcode) {
        // override keeps nvme driver from trying to initialize for testing
        pciehcfg_setconf_classcode(pcfg, res->nvme.classcode);
    }
    pciehcfg_setconf_nintrs(pcfg, res->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, res->dsn);
    if (pciehdev_is_pf(pdev)) {
        pciehcfg_setconf_pf(pcfg, 1);
        pciehcfg_setconf_totalvfs(pcfg, pciehdev_get_totalvfs(pdev));
        pciehcfg_setconf_vfdeviceid(pcfg, PCI_DEVICE_ID_PENSANDO_NVMEVF);
        vfbars = pciehdev_get_vfbars(pdev);
    }

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_vpd(pcfg);
    pciehcfg_add_standard_pfcaps(pcfg, vfbars);
    pciehdev_set_cfg(pdev, pcfg);
    return 0;
}

static int
nvme_pfcfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    return nvme_cfg(pdev, res, PCI_DEVICE_ID_PENSANDO_NVME);
}

static int
nvme_vfcfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    return nvme_cfg(pdev, res, PCI_DEVICE_ID_PENSANDO_NVMEVF);
}

static int
nvme_initpf(pciehdev_t *pfdev, const pciehdev_res_t *pfres)
{
    if (nvme_pfbars(pfdev, pfres) < 0) {
        return -1;
    }
    if (nvme_pfcfg(pfdev, pfres) < 0) {
        return -1;
    }
    return 0;
}

static int
nvme_initvf(pciehdev_t *vfdev, const pciehdev_res_t *vfres)
{
    const int vfidx = pciehdev_get_vfidx(vfdev);
    pciehdev_res_t lvfresbuf, *lvfres = &lvfresbuf;
    pciehdev_nvmeres_t *lnvmeres = &lvfres->nvme;

    if (vfidx > 1) {
        if (vfres->nvme.regssz != 0 && vfres->nvme.regs_stride == 0) {
            pciesys_logerror("nvme_initvf: %s: regs_stride %d\n",
                             vfres->name, vfres->nvme.regs_stride);
            return -1;
        }
    }

    /* vf adjust resources dependent on stride */
    *lvfres = *vfres;
    lvfres->lifb  = vfres->lifb  + vfidx * vfres->lifc;
    lvfres->intrb = vfres->intrb + vfidx * vfres->intrc;
    lvfres->dsn   = vfres->dsn   + vfidx * vfres->dsn_stride;

    /* adjust local resources dependent on stride */
    lnvmeres->regspa = lnvmeres->regspa + vfidx * lnvmeres->regs_stride;

    pciehdev_set_lifb(vfdev, lvfres->lifb);
    pciehdev_set_lifc(vfdev, lvfres->lifc);
    pciehdev_set_intrb(vfdev, lvfres->intrb);
    pciehdev_set_intrc(vfdev, lvfres->intrc);
    pciehdev_set_intrm(vfdev, lvfres->intrdmask);

    if (nvme_vfbars(vfdev, lvfres) < 0) {
        return -1;
    }
    if (nvme_vfcfg(vfdev, lvfres) < 0) {
        return -1;
    }
    return 0;
}

static pciehdevice_t nvme_device = {
    .type = PCIEHDEVICE_NVME,
    .initpf = nvme_initpf,
    .initvf = nvme_initvf,
};
PCIEHDEVICE_REGISTER(nvme_device);
