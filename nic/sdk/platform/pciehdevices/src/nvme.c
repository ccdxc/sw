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
static void
initialize_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    const u_int8_t upd[8] = {
        /* make this table a bit more compact */
#define UPD(U)  PRT_UPD_##U
        /* nvme sq */ [0] = UPD(SCHED_SET)  | UPD(PICI_PISET),
        /* nvme cq */ [1] = UPD(SCHED_NONE) | UPD(PICI_CISET),
#undef UPD
    };
    u_int32_t msixtbloff, msixpbaoff;
    pciehbarreg_t preg;
    pciehbar_t pbar;
    prt_t prt;
    u_int32_t nqids;

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
                pres->port,
                PMT_TYPE_MEM,
                0x1000,         /* pmtsize */
                0x1000,         /* prtsize */
                PMTF_RW);
    prt_res_enc(&prt, pres->nvmeregspa, 0x1000, PRT_RESF_NONE);
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
    nqids = MIN(pres->nvmeqidc, 0x10000);
    nqids = MIN(nqids, 512); /* XXX limit to 512 q's */

    pmt_bar_enc(&preg.pmt,
                pres->port,
                PMT_TYPE_MEM,
                0x1000,         /* pmtsize */
                0x1000,         /* prtsize */
                PMTF_WR);
    /* bit2: 2 qtypes: 0=submission q, 1=completion q */
    pmt_bar_setr_qtype(&preg.pmt, 2, 1);
    /* bit3: qid start */
    pmt_bar_setr_qid(&preg.pmt, 3, 9);

    prt_db32_enc(&prt, pres->lifb, upd);
    /* want 16 bits, but Capri errata allows only 15 */
    prt_db_idxparams(&prt, 15, 0);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x2000 MSIX Interrupt Table
     * +0x3000 MSIX Interrupt PBA */
    assert(pres->intrc <= 256); /* 256 intrs per page (XXX grow bar) */
    msixtbloff = 0x2000;
    msixpbaoff = 0x3000;
    add_msix_region(pbars, &pbar, pres, msixtbloff, msixpbaoff);

    pciehbars_add_bar(pbars, &pbar);
}

static void
initialize_cfg(pciehcfg_t *pcfg, pciehbars_t *pbars,
               const pciehdevice_resources_t *pres)
{
    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_NVME);
    pciehcfg_setconf_classcode(pcfg, 0x010802);
    //pciehcfg_setconf_classcode(pcfg, 0x088000); // XXX
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
nvme_initialize_bars(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehbars_t *pbars;

    pbars = pciehbars_new();
    initialize_bars(pbars, pres);
    pciehdev_set_bars(pdev, pbars);
}

static void
nvme_initialize_cfg(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    initialize_cfg(pcfg, pbars, pres);
    pciehdev_set_cfg(pdev, pcfg);
}

pciehdev_t *
pciehdev_nvme_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    nvme_initialize_bars(pdev, pres);
    nvme_initialize_cfg(pdev, pres);
    return pdev;
}
