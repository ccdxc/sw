/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>
#include <linux/pci_regs.h>

#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/pcietlp/include/pcietlp.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/cfgspace/include/cfgspace.h"
#include "platform/pciemgr/include/pciehw.h"
#include "pciehw_impl.h"

int
pciehw_cfg_init()
{
    pciehw_romsk_init();
    return 0;
}

static void
pciehw_set_cfghnd(pciehwdev_t *phwdev,
                  const u_int16_t reg,
                  const u_int32_t pmtf,
                  const pciehw_cfghnd_t hnd)
{
    const u_int16_t regdw = reg >> 2;
    assert(regdw < PCIEHW_CFGHNDSZ);
    phwdev->cfgpmtf[regdw] = pmtf;
    phwdev->cfghnd[regdw] = hnd;
}

static int
set_bars_handlers(pciehwdev_t *phwdev, const int cfgbase,
                  const pciehw_cfghnd_t cfghnd)
{
    pciehwbar_t *phwbar;
    int i, nbars_valid = 0;

    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (phwbar->valid) {
            const int cfgoff = cfgbase + phwbar->cfgidx * 4;
            const int barlen = phwbar->type == PCIEHWBARTYPE_MEM64 ? 8 : 4;
            pciehw_set_cfghnd(phwdev, cfgoff,
                              PMTF_WR | PMTF_INDIRECT,
                              cfghnd);
            if (barlen == 8) {
                pciehw_set_cfghnd(phwdev, cfgoff + 4,
                                  PMTF_WR | PMTF_INDIRECT,
                                  cfghnd);
            }
            nbars_valid++;
        }
    }
    return nbars_valid;
}

static void
pciehw_cfg_set_handlers(pciehwdev_t *phwdev)
{
    cfgspace_t cs;
    int msixcap, sriovcap;
    int nbars_valid = 0;

    if (!phwdev->vf) {
        /* Bars */
        const int cfgbase = 0x10;
        nbars_valid += set_bars_handlers(phwdev, cfgbase,
                                         PCIEHW_CFGHND_DEV_BARS);
        if (phwdev->rombar.valid) {
            /* Oprom Bar */
            pciehw_set_cfghnd(phwdev, PCI_ROM_ADDRESS,
                              PMTF_WR | PMTF_INDIRECT,
                              PCIEHW_CFGHND_ROM_BAR);
            nbars_valid++;
        }
    }

    pciehwdev_get_cfgspace(phwdev, &cs);
    msixcap = cfgspace_findcap(&cs, PCI_CAP_ID_MSIX);
    if (msixcap) {
        /* MSIX message control */
        pciehw_set_cfghnd(phwdev, msixcap,
                          PMTF_WR | PMTF_INDIRECT, PCIEHW_CFGHND_MSIX);
    }

    /*
     * Only capture writes to Command if we have bars, intrs, or lifs.
     *
     * If we have bars,  then CMD.{mem,io}_enable needs attention.
     * If we have intrs, then CMD.interrupt_disable needs attention.
     * If we have lifs,  then CMD.bus_master_enable needs attention.
     */
    if (nbars_valid > 0 || phwdev->intrc > 0 || phwdev->lifc > 0) {
        /* Command/Status register */
        pciehw_set_cfghnd(phwdev, 0x4,
                          PMTF_WR | PMTF_INDIRECT, PCIEHW_CFGHND_CMD);
    }

    sriovcap = cfgspace_findextcap(&cs, PCI_EXT_CAP_ID_SRIOV);
    if (sriovcap) {
        /* SRIOV Control */
        const int cfgbase = sriovcap + 0x24;
        pciehw_set_cfghnd(phwdev, sriovcap + 0x8,
                          PMTF_WR | PMTF_INDIRECT, PCIEHW_CFGHND_SRIOV_CTRL);
        set_bars_handlers(phwdev, cfgbase, PCIEHW_CFGHND_SRIOV_BARS);
    }
}

/*
 * Fill in VENDOR, SUBVENDOR, and SUBDEVICE ids from
 * our configured defaults, if no values were provided.
 */
static void
pciehw_cfg_set_default_ids(pciehcfg_t *pcfg, cfgspace_t *cs)
{
    const pciemgr_params_t *params = pciehw_get_params();

    if (cfgspace_readw(cs, PCI_VENDOR_ID) == 0x0000) {
        cfgspace_setw(cs, PCI_VENDOR_ID, params->vendorid);
    }
    if (cfgspace_get_headertype(cs) == 0) {
        /* headertype == 0, normal device endpoints */
        if (cfgspace_readw(cs, PCI_SUBSYSTEM_VENDOR_ID) == 0x0000) {
            cfgspace_setw(cs, PCI_SUBSYSTEM_VENDOR_ID, params->subvendorid);
        }
        if (cfgspace_readw(cs, PCI_SUBSYSTEM_ID) == 0x0000) {
            cfgspace_setw(cs, PCI_SUBSYSTEM_ID, params->subdeviceid);
        }
    } else {
        /* headertype == 1, bridge devices */
        const u_int8_t subsys = cfgspace_findcap(cs, PCI_CAP_ID_SSVID);
        if (subsys) {
            if (cfgspace_readw(cs, subsys + 0x4) == 0x0000) {
                cfgspace_setw(cs, subsys + 0x4, params->subvendorid);
            }
            if (cfgspace_readw(cs, subsys + 0x6) == 0x0000) {
                cfgspace_setw(cs, subsys + 0x6, params->subdeviceid);
            }
        }
    }
}

int
pciehw_cfg_finalize(pciehdev_t *pdev)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdev_t *phwdev = pciehdev_get_hwdev(pdev);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehcfg_t *pcfg = pciehdev_get_cfg(pdev);
    cfgspace_t cs;
    u_int16_t cfgsz;

    pciehcfg_get_cfgspace(pcfg, &cs);
    pciehw_cfg_set_default_ids(pcfg, &cs);

    /*
     * Init config space contents from device config space.
     * Init config reset contents from device config space.
     * Init config write mask from device config space.
     */
    cfgsz = cfgspace_size(&cs);
    assert(cfgsz <= PCIEHW_CFGSZ);
    pciehw_memcpy(phwmem->cfgcur[hwdevh], cs.cur, cfgsz); /* config space */
    pciehw_memcpy(pshmem->cfgrst[hwdevh], cs.cur, cfgsz); /* reset space */
    pciehw_memcpy(pshmem->cfgmsk[hwdevh], cs.msk, cfgsz); /* write mask */

    pciehw_cfg_set_handlers(phwdev);

    /*
     * Don't load the root into hardware.  root represents the
     * upstream port bridge which is provided by hardware so no
     * need to add to hw tables to virtualize.
     */
    if (pciehdev_get_parent(pdev) == NULL) {
        return 0;
    }

    if (pciehw_romsk_load(phwdev) < 0) {
        pciesys_logerror("%s: romsk_load failed\n", pciehdev_get_name(pdev));
        return -1;
    }
    if (pciehw_pmt_load_cfg(phwdev) < 0) {
        pciesys_logerror("%s: pmt_load_cfg failed\n", pciehdev_get_name(pdev));
        return -1;
    }
    return 0;
}

int
pciehw_cfg_finalize_done(pciehwdev_t *phwroot)
{
    return 0;
}

/*
 * Detect these overlaps:
 *
 * regaddr        regsize
 * v              v
 * +--------------+
 * +--------------+
 * ^              ^
 * tlpaddr        tpsize
 *
 * regaddr        regsize
 * v              v
 * +--------------+
 *           +--------------+
 *           ^              ^
 *           tlpaddr        tpsize
 *
 *     regaddr        regsize
 *     v              v
 *     +--------------+
 * +--------------+
 * ^              ^
 * tlpaddr        tpsize
 */
static int
stlp_overlap(const pcie_stlp_t *stlp,
             const u_int32_t regaddr, const u_int32_t regsize)
{
    const u_int32_t tlpaddr = stlp->addr;
    const u_int32_t tlpsize = stlp->size;

    return tlpaddr < regaddr + regsize && tlpaddr + tlpsize > regaddr;
}

void
pciehw_cfgrd_notify(pciehwdev_t *phwdev,
                    const pcie_stlp_t *stlp,
                    const tlpauxinfo_t *info,
                    const pciehw_spmt_t *spmt)
{
}

static u_int32_t
cfg_bar32(cfgspace_t *cs, const u_int32_t cfgoff)
{
    u_int32_t baraddr = cfgspace_readd(cs, cfgoff);
    return baraddr;
}

static u_int64_t
cfg_bar64(cfgspace_t *cs, const u_int32_t cfgoff)
{
    u_int32_t barlo, barhi;

    barlo = cfgspace_readd(cs, cfgoff + 0);
    barhi = cfgspace_readd(cs, cfgoff + 4);

    return ((u_int64_t)barhi << 32) | barlo;
}

static u_int64_t
cfg_baraddr(cfgspace_t *cs, const u_int32_t cfgoff, const u_int32_t barlen)
{
    u_int64_t baraddr;

    if (barlen == 8) {
        baraddr = cfg_bar64(cs, cfgoff);
    } else {
        baraddr = cfg_bar32(cs, cfgoff);
    }
    return baraddr;
}

static void
pciehw_cfg_bars_enable(pciehwdev_t *phwdev, const u_int16_t cmd)
{
    const int io_en = (cmd & 0x1) != 0;
    const int mem_en = (cmd & 0x2) != 0;
    pciehwbar_t *phwbar;
    int i;

    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (!phwbar->valid) continue;

        if ((phwbar->type == PCIEHWBARTYPE_MEM ||
             phwbar->type == PCIEHWBARTYPE_MEM64)) {
            pciehw_bar_enable(phwbar, mem_en);
        } else if (phwbar->type == PCIEHWBARTYPE_IO) {
            pciehw_bar_enable(phwbar, io_en);
        }
    }
}

/*
 * rombar is enabled iff CMD.memory_space_en && ROMBAR.en.
 */
static void
pciehw_cfg_rombar_enable(pciehwbar_t *phwbar, cfgspace_t *cs)
{
    if (phwbar->valid) {
        const int mem_en = cfgspace_readw(cs, PCI_COMMAND) & 0x2;
        const int rom_en = cfgspace_readd(cs, PCI_ROM_ADDRESS) & 0x1;

        pciehw_bar_enable(phwbar, mem_en && rom_en);
    }
}

static void
pciehw_cfg_busmaster_enable(pciehwdev_t *phwdev, const int on)
{
    /* XXX enable this? */
#if 0
    if (on) {
        pciehw_hdrt_load(phwdev->lifb, phwdev->lifc);
    } else {
        pciehw_hdrt_unload(phwdev->lifb, phwdev->lifc);
    }
#endif
}

static void
pciehw_cfgwr_cmd(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int16_t cmd, msixcap, msixctl;

    pciehwdev_get_cfgspace(phwdev, &cs);
    cmd = cfgspace_readw(&cs, PCI_COMMAND);

    /*
     * PF check cmd reg for bar enables.
     * VF bar enables come from PF sriov capability (see cfgwr_sriov()).
     */
    if (!phwdev->vf) {
        /* bar control */
        pciehw_cfg_bars_enable(phwdev, cmd);
        /* cmd.mem_enable might have enabled rombar */
        pciehw_cfg_rombar_enable(&phwdev->rombar, &cs);

        msixcap = cfgspace_findcap(&cs, PCI_CAP_ID_MSIX);
        if (msixcap) {
            msixctl = cfgspace_readw(&cs, msixcap + PCI_MSIX_FLAGS);
        } else {
            msixctl = 0;
        }

        /* intx_disable */
        if ((msixctl & PCI_MSIX_FLAGS_ENABLE) == 0) {
            const int legacy = 1;
            const int fmask = (cmd & PCI_COMMAND_INTX_DISABLE) != 0;
            pciehw_intr_config(phwdev, legacy, fmask);
        }
    }

    pciehw_cfg_busmaster_enable(phwdev, (cmd & 0x4) != 0);
}

static void
pciehw_cfgwr_bars(pciehwdev_t *phwdev,
                  const pcie_stlp_t *stlp,
                  cfgspace_t *cs,
                  const int cfgbase)
{
    pciehwbar_t *phwbar;
    int i;

    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (phwbar->valid) {
            const int cfgoff = cfgbase + phwbar->cfgidx * 4;
            const int barlen = phwbar->type == PCIEHWBARTYPE_MEM64 ? 8 : 4;
            if (stlp_overlap(stlp, cfgoff, barlen)) {
                const u_int64_t vfbaroff = (pciehw_bar_getsize(phwbar) *
                                            phwdev->vfidx);
                u_int64_t baraddr = cfg_baraddr(cs, cfgoff, barlen);
                if (phwbar->type == PCIEHWBARTYPE_IO) {
                    baraddr &= ~0x3ULL;
                } else {
                    baraddr &= ~0xfULL;
                }
                pciehw_bar_setaddr(phwbar, baraddr + vfbaroff);
            }
        }
    }
}

static void
pciehw_cfgwr_dev_bars(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    const int cfgbase = 0x10;
    cfgspace_t cs;

    pciehwdev_get_cfgspace(phwdev, &cs);
    pciehw_cfgwr_bars(phwdev, stlp, &cs, cfgbase);
}

static void
pciehw_cfgwr_rom_bar(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    pciehwbar_t *phwbar;
    cfgspace_t cs;
    u_int32_t baraddr;

    pciehwdev_get_cfgspace(phwdev, &cs);

    phwbar = &phwdev->rombar;
    baraddr = cfgspace_readd(&cs, PCI_ROM_ADDRESS);
    baraddr &= ~0x1; /* mask enable bit */
    pciehw_bar_setaddr(phwbar, baraddr);

    pciehw_cfg_rombar_enable(phwbar, &cs);
}

static void
pciehw_cfgwr_msix(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    const u_int16_t reg = stlp->addr;
    const u_int16_t regdw = reg >> 2;
    cfgspace_t cs;
    u_int16_t msixctl, cmd;
    int msix_en, msix_mask, fmask, legacy;

    pciehwdev_get_cfgspace(phwdev, &cs);
    msixctl = cfgspace_readw(&cs, (regdw << 2) + 2);
    msix_en = (msixctl & PCI_MSIX_FLAGS_ENABLE) != 0;
    msix_mask = (msixctl & PCI_MSIX_FLAGS_MASKALL) != 0;

    if (msix_en) {
        /* msix mode */
        legacy = 0;
        fmask = msix_mask;
    } else if (phwdev->vf) {
        /* sriov vf disabled */
        legacy = 0;
        fmask = 1;
    } else {
        /* intx mode */
        legacy = 1;
        cmd = cfgspace_readw(&cs, PCI_COMMAND);
        fmask = phwdev->vf || (cmd & PCI_COMMAND_INTX_DISABLE) != 0;
    }

    pciehw_intr_config(phwdev, legacy, fmask);
}

static void
pciehw_sriov_numvfs_event(pciehwdev_t *phwdev, const u_int16_t numvfs)
{
    pciehdev_eventdata_t evd;
    pciehdev_sriov_numvfs_t *sriov_numvfs;

    memset(&evd, 0, sizeof(evd));
    evd.evtype = PCIEHDEV_EV_SRIOV_NUMVFS;
    evd.port = phwdev->port;
    evd.lif = phwdev->lifb;
    sriov_numvfs = &evd.sriov_numvfs;
    sriov_numvfs->numvfs = numvfs;
    pciehw_event(phwdev, &evd);
}

/*
 * Note this depends on all VFs being allocated contiguously,
 * which is true (at the moment).  If that assumption changes
 * we'll need another algorithm to find the VF from the PF.
 */
static pciehwdev_t *
pciehwdev_getvf(pciehwdev_t *phwdev, const int vfidx)
{
    assert(vfidx >= 0 && vfidx < phwdev->totalvfs);
    return pciehwdev_get(phwdev->childh + vfidx);
}

static void
pciehw_sriov_enable_vf(pciehwdev_t *vfhwdev,
                       const int cfg_en, const int bar_en)
{
    u_int16_t cmd;

    /* XXX handled cfg_en load/unload cfg space */
    /* refactor and call pciehw_cfg_load(vfhwdev, cfg_en) */

    /* load/unload the bars */
    cmd = bar_en ? PCI_COMMAND_MEMORY : 0;
    pciehw_cfg_bars_enable(vfhwdev, cmd);
}

static void
pciehw_sriov_enable_numvfs(pciehwdev_t *phwdev, const u_int16_t numvfs,
                           const int cfg_en, const int bar_en)
{
    pciehwdev_t *vfhwdev;
    int onumvfs, vfidx;

    onumvfs = phwdev->numvfs;

    /*
     * If numvfs < onumvfs (our currently enabled vfs),
     * we need to disable some vfs.
     */
    for (vfidx = numvfs; vfidx < onumvfs; vfidx++) {
        vfhwdev = pciehwdev_getvf(phwdev, vfidx);
        pciehw_sriov_enable_vf(vfhwdev, 0, 0);
    }

    /*
     * Now go enable any newly enabled VFs, i.e. numvfs > onumvfs.
     */
    for (vfidx = 0; vfidx < numvfs; vfidx++) {
        vfhwdev = pciehwdev_getvf(phwdev, vfidx);
        pciehw_sriov_enable_vf(vfhwdev, cfg_en, bar_en);
    }

    if (phwdev->numvfs != numvfs) {
        phwdev->numvfs = numvfs;
        pciehw_sriov_numvfs_event(phwdev, numvfs);
    }
}

static void
pciehw_cfgwr_sriov_ctrl(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int16_t sriovcap, sriovctrl, numvfs;
    int cfg_en, bar_en;

    pciehwdev_get_cfgspace(phwdev, &cs);
    sriovcap = cfgspace_findextcap(&cs, PCI_EXT_CAP_ID_SRIOV);
    sriovctrl = cfgspace_readw(&cs, sriovcap + PCI_SRIOV_CTRL);
    numvfs = cfgspace_readw(&cs, sriovcap + PCI_SRIOV_NUM_VF);
    numvfs = MIN(numvfs, phwdev->totalvfs);

    cfg_en = (sriovctrl & PCI_SRIOV_CTRL_VFE) != 0;
    bar_en = cfg_en && (sriovctrl & PCI_SRIOV_CTRL_MSE) != 0;

#ifdef PCIEMGR_DEBUG
    pciesys_loginfo("sriov_ctrl: "
                    "ctrl 0x%04x (cfg_en %d bar_en %d) numvfs %d->%d\n",
                    sriovctrl, cfg_en, bar_en, phwdev->numvfs, numvfs);
#endif

    pciehw_sriov_enable_numvfs(phwdev, numvfs, cfg_en, bar_en);
}

static void
pciehw_cfgwr_sriov_bars(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    pciehwdev_t *vfhwdev;
    cfgspace_t pfcs;
    int vfidx, sriovcap;

    pciehwdev_get_cfgspace(phwdev, &pfcs);
    sriovcap = cfgspace_findextcap(&pfcs, PCI_EXT_CAP_ID_SRIOV);

    /*
     * Distribute the new bar address to all the VFs.
     * Each VF will compute its own offset within
     * the bar for its VF sliced region.
     */
    for (vfidx = 0; vfidx < phwdev->totalvfs; vfidx++) {
        vfhwdev = pciehwdev_getvf(phwdev, vfidx);
        pciehw_cfgwr_bars(vfhwdev, stlp, &pfcs, sriovcap + 0x24);
    }
}

void
pciehw_cfgwr_notify(pciehwdev_t *phwdev,
                    const pcie_stlp_t *stlp,
                    const tlpauxinfo_t *info,
                    const pciehw_spmt_t *spmt)
{
    const u_int16_t reg = stlp->addr;
    const u_int16_t regdw = reg >> 2;
    pciehw_cfghnd_t hnd = PCIEHW_CFGHND_NONE;

    if (regdw < PCIEHW_CFGHNDSZ) {
        hnd = phwdev->cfghnd[regdw];
    }
    switch (hnd) {
    case PCIEHW_CFGHND_NONE:
        break;
    case PCIEHW_CFGHND_CMD:
        pciehw_cfgwr_cmd(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_DEV_BARS:
        pciehw_cfgwr_dev_bars(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_ROM_BAR:
        pciehw_cfgwr_rom_bar(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_MSIX:
        pciehw_cfgwr_msix(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_SRIOV_CTRL:
        pciehw_cfgwr_sriov_ctrl(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_SRIOV_BARS:
        pciehw_cfgwr_sriov_bars(phwdev, stlp);
        break;
    }
}

void
pciehw_cfgrd_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const tlpauxinfo_t *info = &ientry->info;
    const u_int32_t pmti = info->pmti;
    const pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    const pciehwdevh_t hwdevh = spmt->owner;
    pciehwdev_t *phwdev = pciehwdev_get(hwdevh);
    u_int32_t val;

    /*
     * For indirect reads, let the handler run first,
     * then we'll pick up the cfg value.  The handler
     * has a chance to modify the data if desired.
     */
    pciehw_cfgrd_notify(phwdev, stlp, info, spmt);
    if (pciehwdev_cfgrd(phwdev, stlp->addr, stlp->size, &val) < 0) {
        ientry->cpl = PCIECPL_CA;
    } else {
        ientry->data[0] = val;
    }
    pciehw_indirect_complete(ientry);
}

void
pciehw_cfgwr_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const tlpauxinfo_t *info = &ientry->info;
    const u_int32_t pmti = info->pmti;
    const pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    const pciehwdevh_t hwdevh = spmt->owner;
    pciehwdev_t *phwdev = pciehwdev_get(hwdevh);

    /*
     * For indirect writes, write the data first,
     * then let the handler run with the updated data.
     */
    if (pciehwdev_cfgwr(phwdev, stlp->addr, stlp->size, stlp->data) < 0) {
        ientry->cpl = PCIECPL_CA;
    }
    pciehw_cfgwr_notify(phwdev, stlp, info, spmt);
    pciehw_indirect_complete(ientry);
}
