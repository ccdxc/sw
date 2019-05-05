/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
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

void
add_msix_region(pciehbars_t *pbars,
                pciehbar_t *pbar,
                const pciehdev_res_t *res,
                const u_int32_t msixtbloff,
                const u_int32_t msixpbaoff)
{
    const u_int32_t msixtbl_stride = roundup_power2(intr_msixcfg_size(1));
    u_int32_t intrs_to_map, intrb;
    u_int64_t baroff;
    pciehbarreg_t preg;
    prt_t prt;

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
        pciehbar_add_reg(pbar, &preg);

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
        pciehbar_add_reg(pbar, &preg);
    }

    /* set msix cap info */
    if (res->intrc) {
        const int baridx = pbar->cfgidx;
        pciehbars_set_msix_tbl(pbars, baridx, msixtbloff);
        pciehbars_set_msix_pba(pbars, baridx, msixpbaoff);
    }
}

void
add_common_resource_bar(pciehbars_t *pbars,
                        const pciehdev_res_t *res)
{
    const u_int32_t msixtbl_stride = roundup_power2(intr_msixcfg_size(1));
    const u_int32_t intrctrl_stride = roundup_power2(intr_drvcfg_size(1));
    const u_int32_t dboff = 0x0800;
    u_int64_t intrctrlsz, msixtblsz, total_barsz, baroff;
    u_int32_t msixtbloff, msixpbaoff, intrs_to_map, intrb;
    pciehbarreg_t preg;
    pciehbar_t pbar;
    prt_t prt;

    /*
     * Our minimum bar size is 4*0x1000=16k.  This is for a device with
     * no interrupts.  We allocate a page for each of these regions
     *     Info Regs, Devcmd Regs (+0x0000)
     *     Intr Status Regs (+0x1000) (unused)
     *     Intr Ctrl Regs (+0x2000) (unused)
     *     (+0x3000 unused)
     *
     * If this device has interrupts (res->intrc > 0)
     * then we have these regions:
     *     Info Regs, Devcmd Regs (+0x0000)
     *     Intr Status Regs (+0x1000)
     *     Intr Ctrl Regs (+0x2000)
     *     MSIX Interrupt Table
     *     MSIX Interrupt PBA
     *
     * Intr Ctrl Regs and MSIX Interrupt Table regions are variable size
     * depending on the number of interrupts, but always a multiple of 4k.
     * MSIX Interrupt PBA, if present, is the last 4k region of the bar.
     */

    /*
     * Expecting devregs to fit in first page.
     * The address must be aligned to at least a 4096 boundary.
     */
    assert((res->eth.devregspa & (4096 - 1)) == 0);
    assert(res->eth.devregssz <= 4096);
    assert(res->eth.devregssz > dboff + 4); /* must cover doorbell */

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
     * The common  bar size is 32k  if we have [1-384]     interrupts.
     * The big     bar size is 64k  if we have [385-1024]  interrupts.
     * The maximum bar size is 128k if we have [1025-2048] interrupts.
     */
    total_barsz = roundup_power2(3 * 0x1000 + intrctrlsz + msixtblsz);

    /*
     * PCIe spec recommends putting these on 4k boundaries,
     * alone without any other resources in those pages.
     *
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
     * +0x0000 Device Info Regs read-only */
    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                0x800, /* pmtsize */
                0x800, /* prtsize */
                PMTF_WR);
    /* 0-size res mapping claims writes but discards them */
    prt_res_enc(&prt, res->eth.devregspa, 0, PRT_RESF_PMVDIS);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x0800 Device Cmd Regs 4-byte doorbell */
    memset(&preg, 0, sizeof(preg));
    preg.baroff = dboff;
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                0x4,    /* pmtsize */
                0x4,    /* prtsize */
                PMTF_WR);
    prt_res_enc(&prt, res->eth.devregspa + dboff, 4,
                PRT_RESF_PMVDIS | PRT_RESF_NOTIFY);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x0000 Device Info/Cmd Regs (rw for devcmd section)
     */
    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                0x1000, /* pmtsize */
                0x1000, /* prtsize */
                PMTF_RW);
    prt_res_enc(&prt, res->eth.devregspa, res->eth.devregssz, PRT_RESF_PMVDIS);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);

    /*****************
     * +0x1000 Interrupt Status reg */
    memset(&preg, 0, sizeof(preg));
    preg.baroff = 0x1000;
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
     * +0x2000 Interrupt Control regs */
    intrs_to_map = res->intrc;
    intrb = res->intrb;
    baroff = 0x2000;
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
     * +0x3000 <reserved>
     * +0x4000 <reserved>
     * +0x5000 <reserved>
     * For larger bars with more intrs, these regions contain more intrs.
     */

    add_msix_region(pbars, &pbar, res, msixtbloff, msixpbaoff);

    /*
     * add this bar to our bars
     */
    pciehbars_add_bar(pbars, &pbar);
}

void
add_common_doorbell_bar(pciehbars_t *pbars,
                        const pciehdev_res_t *res,
                        const u_int8_t upd[8])
{
    pciehbarreg_t preg;
    pciehbar_t pbar;
    prt_t prt;
    u_int32_t nlifs, nlifs2, npids, npids2;
    int bitb, bitc;

    /*****************
     * doorbell bar - mem64
     */
    memset(&pbar, 0, sizeof(pbar));

    /*
     * At least 1 lif even if none specified.
     * Then we roundup to a power-of-2 because we are using an
     * adjacent group of db bar address bits for this field.
     * If nlifs is not an even power-of-2 then we'll cap to
     * nlifs in the vf_params fields below.
     */
    nlifs = MAX(res->lifc, 1);
    nlifs2 = roundup_power2(nlifs);

    /*
     * At least 1 pid even if none specified.
     * Then we roundup to a power-of-2 because we are using an
     * adjacent group of db bar address bits for this field.
     * It is ok to roundup above "npids"--we'll consume more
     * bar space, but if the doorbell is using pidcheck then
     * we won't be able to access the doorbell from the
     * unconfigured pids because the driver will never give
     * permission to access from the unused pids.
     */
    npids = MAX(res->npids, 1);
    npids2 = roundup_power2(npids);

    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = 0x1000 * nlifs2 * npids2;
    pbar.cfgidx = 2;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                pbar.size,       /* pmtsize */
                npids2 * 0x1000, /* prtsize */
                PMTF_WR);

    pmt_bar_set_qtype(&preg.pmt, 3, 0x7);

    /*
     * Default page size 4k=1<<12, then
     * lif bits start above pid bits.
     */
    if (nlifs > 1) {
        bitb = 12 + ffs(npids2) - 1;
        bitc = ffs(nlifs2) - 1;
        pmt_bar_set_vfparams(&preg.pmt, bitb, bitc, res->lifb, nlifs);
    }

    prt_db64_enc(&prt, res->lifb, upd);
    if (nlifs > 1) {
        /* if multiple lifs, set stride between lif db's */
        prt_db_set_vfstride(&prt, 1 << 6);
    }
    pciehbarreg_add_prt(&preg, &prt);

    pciehbar_add_reg(&pbar, &preg);
    pciehbars_add_bar(pbars, &pbar);
}

void
add_common_cmb_bar(pciehbars_t *pbars, const pciehdev_res_t *res)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;
    prt_t prt;

    /*****************
     * optional cmb bar
     */
    if (res->cmbsz) {
        memset(&pbar, 0, sizeof(pbar));
        pbar.type = PCIEHBARTYPE_MEM64;
        pbar.size = roundup_power2(res->cmbsz);
        pbar.cfgidx = 4;
        pbar.prefetch = res->cmbprefetch;

        memset(&preg, 0, sizeof(preg));
        pmt_bar_enc(&preg.pmt,
                    res->port,
                    PMT_TYPE_MEM,
                    pbar.size,  /* pmtsize */
                    pbar.size,  /* prtsize */
                    PMTF_RW);

        prt_res_enc(&prt, res->cmbpa, res->cmbsz, PRT_RESF_PMVDIS);
        pciehbarreg_add_prt(&preg, &prt);
        pciehbar_add_reg(&pbar, &preg);
        pciehbars_add_bar(pbars, &pbar);
    }
}

void
add_common_rom_bar(pciehbars_t *pbars, const pciehdev_res_t *res)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;
    prt_t prt;

    /*****************
     * optional oprom bar
     */
    if (res->romsz) {
        memset(&pbar, 0, sizeof(pbar));
        pbar.type = PCIEHBARTYPE_MEM;
        pbar.size = roundup_power2(res->romsz);

        memset(&preg, 0, sizeof(preg));
        pmt_bar_enc(&preg.pmt,
                    res->port,
                    PMT_TYPE_MEM,
                    pbar.size,  /* pmtsize */
                    pbar.size,  /* prtsize */
                    PMTF_RD);

        prt_res_enc(&prt, res->rompa, res->romsz, PRT_RESF_PMVDIS);
        pciehbarreg_add_prt(&preg, &prt);
        pciehbar_add_reg(&pbar, &preg);
        pciehbars_add_rombar(pbars, &pbar);
    }
}
