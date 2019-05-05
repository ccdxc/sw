/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/param.h>

#include "storage/storage_seq_common.h"
#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/misc/include/misc.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

static void
add_accel_resource_bar(pciehbars_t *pbars,
                       const pciehdev_res_t *res)
{
    const u_int32_t intrctrl_stride = roundup_power2(intr_drvcfg_size(1));
    const u_int32_t msixtbl_stride = roundup_power2(intr_msixcfg_size(1));
    u_int64_t intrctrlsz, msixtblsz, total_barsz, baroff;
    u_int32_t msixtbloff, msixpbaoff, intrs_to_map, intrb;
    pciehbarreg_t preg;
    pciehbar_t pbar;
    prt_t prt;

    /*
     * Our minimum bar size is 4*0x1000=16k.  This is for a device with
     * no interrupts.  We allocate a page for each of these regions
     *     Devcmd Regs (+0x0000)
     *     Devcmd Doorbell (+0x1000)
     *     Intr Status Regs (+0x2000) (unused)
     *     Intr Ctrl Regs (+0x3000) (unused)
     *
     * If this device has interrupts (res->intrc > 0)
     * then we have these regions:
     *     Devcmd Regs (+0x0000)
     *     Devcmd Doorbell (+0x1000)
     *     Intr Status Regs (+0x2000)
     *     Intr Ctrl Regs (+0x3000)
     *     MSIX Interrupt Table
     *     MSIX Interrupt PBA
     *
     * Intr Ctrl Regs and MSIX Interrupt Table regions are variable size
     * depending on the number of interrupts, but always a multiple of 4k.
     * MSIX Interrupt PBA, if present, is the last 4k region of the bar.
     */

    /* Max MSIX Interrupt Table entries is 2048 per PCIe spec. */
    assert(res->intrc <= 2048);

    /*
     * PCIe spec recommends putting these on page boundaries,
     * alone without any other resources in those pages.
     */
    intrctrlsz = roundup(intrctrl_stride * res->intrc, 0x1000);
    msixtblsz  = roundup(msixtbl_stride  * res->intrc, 0x1000);

    /*
     * Total bar size must be a power-of-2 per PCIe spec.
     * The minimum bar size is 16k  if we have no          interrupts.
     * The common  bar size is 32k  if we have [1-256]     interrupts.
     * The big     bar size is 64k  if we have [257-1024]  interrupts.
     * The maximum bar size is 128k if we have [1025-2048] interrupts.
     */
    total_barsz = roundup_power2(4 * 0x1000 + intrctrlsz + msixtblsz);

    /*
     * MSIX Interrupt PBA is the last page of the bar.
     * MSIX Interrupt Table in the page(s) before PBA.
     */
    msixpbaoff = total_barsz - 0x1000;
    msixtbloff = msixpbaoff - msixtblsz;

    /*****************
     * resource bar - mem64
     */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = total_barsz;
    pbar.cfgidx = 0;

    /*****************
     * +0x0000 Device Cmd Regs 4-byte signature read-only */
    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                0x4,    /* pmtsize */
                0x4,    /* prtsize */
                PMTF_WR);
    /* 0-size res mapping claims writes but discards them */
    prt_res_enc(&prt, res->accel.devcmdpa, 0, PRT_RESF_PMVDIS);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x0000 Device Cmd Regs
     * +0x1000 Device Cmd Doorbell
     */
    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                0x2000, /* pmtsize */
                0x1000, /* prtsize */
                PMTF_RW);
    pmt_bar_setr_prt(&preg.pmt, 12, 1);

    /* +0x0000 Device Cmd Regs */
    prt_res_enc(&prt, res->accel.devcmdpa, 0x1000, PRT_RESF_PMVDIS);
    pciehbarreg_add_prt(&preg, &prt);

    /* +0x1000 Device Cmd Doorbell */
    prt_res_enc(&prt, res->accel.devcmddbpa, 0x4, (PRT_RESF_NOTIFY |
                                                   PRT_RESF_PMVDIS));
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x2000 Interrupt Status reg */
    memset(&preg, 0, sizeof(preg));
    preg.baroff = 0x2000;
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                0x8,    /* pmtsize */
                0x8,    /* prtsize */
                PMTF_RD);
    prt_res_enc(&prt,
                intr_pba_addr(res->lifb),
                intr_pba_size(res->intrc),
                PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x3000 Interrupt Control regs */
    intrs_to_map = res->intrc;
    intrb = res->intrb;
    baroff = 0x3000;
    while (intrs_to_map > 0) {
        /*
         * Each page can map 128 intr control regs.
         * 0x1000 / intrctrl_stride = 128.
         */
        const int intrctrl_per_page = 0x1000 / intrctrl_stride;
        const u_int32_t intrc = MIN(intrs_to_map, intrctrl_per_page);
        const u_int64_t pmtsize = roundup_power2(intr_drvcfg_size(intrc));
        const int vfbitb = ffs(intrctrl_stride) - 1;
        const int vfbitc = (ffs(pmtsize) - 1) - vfbitb;

        memset(&preg, 0, sizeof(preg));
        preg.baroff = baroff;
        pmt_bar_enc(&preg.pmt,
                    res->port,
                    PMT_TYPE_MEM,
                    pmtsize,
                    intrctrl_stride, /* prtsize */
                    PMTF_RW);
        pmt_bar_set_vfparams(&preg.pmt, vfbitb, vfbitc, 0, intrc);
        prt_res_enc(&prt,
                    intr_drvcfg_addr(intrb),
                    intr_drvcfg_size(intrc),
                    PRT_RESF_NONE);
        prt_res_set_vfstride(&prt, intrctrl_stride);
        pciehbarreg_add_prt(&preg, &prt);
        pciehbar_add_reg(&pbar, &preg);

        intrs_to_map -= intrc;
        intrb += intrc;
        baroff += 0x1000;
    }

    /*****************
     * For common bar sizes (intrs 1-128) these next pages are reserved.
     * +0x4000 <reserved>
     * +0x5000 <reserved>
     * For larger bars with more intrs, these regions contain more
     *
     */

    /*****************
     * MSI-X Interrupt Table */
    intrs_to_map = res->intrc;
    intrb = res->intrb;
    baroff = msixtbloff;
    while (intrs_to_map > 0) {
        /*
         * Each page can map 256 msi-x intr control regs.
         * 0x1000 / msixtbl_stride = 256.
         */
        const int msixtbl_per_page = 0x1000 / msixtbl_stride;
        const u_int32_t intrc = MIN(intrs_to_map, msixtbl_per_page);
        const u_int64_t pmtsize = roundup_power2(intr_msixcfg_size(intrc));
        const int vfbitb = ffs(intr_msixcfg_size(1)) - 1;
        const int vfbitc = (ffs(pmtsize) - 1) - vfbitb;

        memset(&preg, 0, sizeof(preg));
        preg.baroff = baroff;
        pmt_bar_enc(&preg.pmt,
                    res->port,
                    PMT_TYPE_MEM,
                    pmtsize,
                    msixtbl_stride, /* prtsize */
                    PMTF_RW);
        pmt_bar_set_vfparams(&preg.pmt, vfbitb, vfbitc, 0, intrc);
        prt_res_enc(&prt,
                    intr_msixcfg_addr(intrb),
                    intr_msixcfg_size(intrc),
                    PRT_RESF_NONE);
        prt_res_set_vfstride(&prt, msixtbl_stride);
        pciehbarreg_add_prt(&preg, &prt);
        pciehbar_add_reg(&pbar, &preg);

        intrs_to_map -= intrc;
        intrb += intrc;
        baroff += 0x1000;
    }

    /*****************
     * MSI-X Interrupt PBA */
    if (res->intrc) {
        memset(&preg, 0, sizeof(preg));
        preg.baroff = msixpbaoff;
        pmt_bar_enc(&preg.pmt,
                    res->port,
                    PMT_TYPE_MEM,
                    0x8,    /* pmtsize */
                    0x8,    /* prtsize */
                    PMTF_RD);
        prt_res_enc(&prt,
                    intr_pba_addr(res->lifb),
                    intr_pba_size(res->intrc),
                    PRT_RESF_NONE);
        pciehbarreg_add_prt(&preg, &prt);
        pciehbar_add_reg(&pbar, &preg);
    }

    /*
     * add this bar to our bars
     */
    pciehbars_add_bar(pbars, &pbar);

    /* set msix cap info */
    if (res->intrc) {
        const int baridx = pbar.cfgidx;
        pciehbars_set_msix_tbl(pbars, baridx, msixtbloff);
        pciehbars_set_msix_pba(pbars, baridx, msixpbaoff);
    }
}

static int
accel_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    const u_int8_t upd[8] = {
        [STORAGE_SEQ_QTYPE_SQ] =
            PRT_UPD_SCHED_SET | PRT_UPD_PICI_PISET | PRT_UPD_PID_CHECK,
        [STORAGE_SEQ_QTYPE_ADMIN] =
            PRT_UPD_SCHED_SET | PRT_UPD_PICI_PISET | PRT_UPD_PID_CHECK,
    };
    pciehbars_t *pbars;

    pbars = pciehbars_new();

    add_accel_resource_bar(pbars, res);
    add_common_doorbell_bar(pbars, res, upd);
    add_common_cmb_bar(pbars, res);
    add_common_rom_bar(pbars, res);

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
accel_cfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_ACCEL);
    pciehcfg_setconf_classcode(pcfg, 0x120000); /* processing accelerator */
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
accel_initpf(pciehdev_t *pfdev, const pciehdev_res_t *pfres)
{
    if (accel_bars(pfdev, pfres) < 0) {
        return -1;
    }
    if (accel_cfg(pfdev, pfres) < 0) {
        return -1;
    }
    return 0;
}

static pciehdevice_t accel_device = {
    .type = PCIEHDEVICE_ACCEL,
    .initpf = accel_initpf,
};
PCIEHDEVICE_REGISTER(accel_device);
