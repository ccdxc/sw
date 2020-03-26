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

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/pcietlp/include/pcietlp.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/pciemgrutils/include/pciehcfg_impl.h"
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
    cfgspace_t cfgspace, *cs = &cfgspace;
    const int pmtf_wrind = PMTF_WR | PMTF_INDIRECT;
    int nbars_valid = 0;
    int cap;

    pciehwdev_get_cfgspace(phwdev, cs);

    /*****************
     * Bars
     */
    if (!phwdev->vf) {
        /* Bars */
        const int cfgbase = 0x10;
        nbars_valid += set_bars_handlers(phwdev, cfgbase,
                                         PCIEHW_CFGHND_DEV_BARS);
        if (phwdev->rombar.valid) {
            /* Oprom Bar */
            pciehw_set_cfghnd(phwdev, PCI_ROM_ADDRESS, pmtf_wrind,
                              PCIEHW_CFGHND_ROM_BAR);
            nbars_valid++;
        }
    }

    /*****************
     * Bridge Control for bus reset
     */
    /* if bridge */
    if (cfgspace_get_headertype(cs) == 1) {
        pciehw_set_cfghnd(phwdev, PCI_BRIDGE_CONTROL, pmtf_wrind,
                          PCIEHW_CFGHND_BRIDGECTL);
    }

    /*****************
     * Command
     *
     * Only capture writes to Command if we have bars, intrs, or lifs.
     *
     * If we have bars,  then CMD.{mem,io}_enable needs attention.
     * If we have intrs, then CMD.interrupt_disable needs attention.
     * If we have lifs,  then CMD.bus_master_enable needs attention.
     */
    if (nbars_valid > 0 || phwdev->intrc > 0 || phwdev->lifc > 0) {
        /* Command/Status register */
        pciehw_set_cfghnd(phwdev, 0x4, pmtf_wrind, PCIEHW_CFGHND_CMD);
    }

    /*****************
     * MSIX cap
     */
    cap = cfgspace_findcap(cs, PCI_CAP_ID_MSIX);
    if (cap) {
        /* MSIX message control */
        pciehw_set_cfghnd(phwdev, cap, pmtf_wrind, PCIEHW_CFGHND_MSIX);
    }

    /*****************
     * Express cap, FLR and Device Control
     */
    cap = cfgspace_findcap(cs, PCI_CAP_ID_EXP);
    if (cap) {
        const u_int32_t devcap = cfgspace_readd(cs, cap + 0x4);
        if (devcap & (1 << 28)) { /* FLR capability */
            /*
             * If FLR capability claimed, then watch for FLR requests
             * as writes to Device Control (+0x8) register of PCIE capability.
             */
            pciehw_set_cfghnd(phwdev, cap + 0x8,
                              pmtf_wrind, PCIEHW_CFGHND_PCIE_DEVCTL);
        }
    }

    /*****************
     * VPD
     */
    cap = cfgspace_findcap(cs, PCI_CAP_ID_VPD);
    if (cap) {
        pciehw_set_cfghnd(phwdev, cap, PMTF_WR | PMTF_NOTIFY,
                          PCIEHW_CFGHND_VPD);
    }

    /*****************
     * SRIOV
     */
    cap = cfgspace_findextcap(cs, PCI_EXT_CAP_ID_SRIOV);
    if (cap) {
        /* SRIOV Control */
        const int cfgbase = cap + 0x24;
        pciehw_set_cfghnd(phwdev, cap + 0x8,
                          pmtf_wrind, PCIEHW_CFGHND_SRIOV_CTRL);
        set_bars_handlers(phwdev, cfgbase, PCIEHW_CFGHND_SRIOV_BARS);
    }

    /*****************
     * pciestress debug device,
     * set indirect handler with programmable delay to
     * test pcie timeouts.
     */
    if (strncmp(phwdev->name, "pciestress", strlen("pciestress")) == 0) {
        cfgspace_setdm(cs, 0x400, 0, 0xffffffff);
        pciehw_set_cfghnd(phwdev, 0x400, PMTF_RW | PMTF_INDIRECT,
                          PCIEHW_CFGHND_DBG_DELAY);
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
    pciehw_vpd_finalize(phwdev, &pcfg->vpdtab);

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

static void
pciehw_cfgrd_delay(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int32_t delayus;

    pciehwdev_get_cfgspace(phwdev, &cs);
    delayus = cfgspace_readd(&cs, 0x400);
    if (delayus) {
        pciesys_logdebug("cfgrd delay %uus\n", delayus);
        usleep(delayus);
    }
}

void
pciehw_cfgrd_notify(pciehwdev_t *phwdev,
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
    default:
    case PCIEHW_CFGHND_NONE:
        break;
    case PCIEHW_CFGHND_DBG_DELAY:
        pciehw_cfgrd_delay(phwdev, stlp);
        break;
    }
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
    const int io_en = (cmd & PCI_COMMAND_IO) != 0;
    const int mem_en = (cmd & PCI_COMMAND_MEMORY) != 0;
    pciehwbar_t *phwbar;
    int i;

#ifdef PCIEMGR_DEBUG
    pciesys_logdebug("bars_enable: %s %smem %sio\n",
                     pciehwdev_get_name(phwdev),
                     mem_en ? "" : "!",
                     io_en ? "" : "!");
#endif
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
#ifdef PCIEMGR_DEBUG
    pciesys_logdebug("busmaster_enable: %s %s\n",
                     pciehwdev_get_name(phwdev), on ? "on" : "off");
#endif
    if (on) {
        pciehw_hdrt_load(phwdev->lifb, phwdev->lifc, phwdev->bdf);
    } else {
        pciehw_hdrt_unload(phwdev->lifb, phwdev->lifc);
    }
}

static void
pciehw_cfg_cmd(pciehwdev_t *phwdev, cfgspace_t *cs, const u_int16_t cmd)
{
    u_int16_t msixcap, msixctl;

    /*
     * PF check cmd reg for bar enables.
     * VF bar enables come from PF sriov capability (see cfgwr_sriov()).
     */
    if (!phwdev->vf) {
        /* bar control */
        pciehw_cfg_bars_enable(phwdev, cmd);
        /* cmd.mem_enable might have enabled rombar */
        pciehw_cfg_rombar_enable(&phwdev->rombar, cs);

        msixcap = cfgspace_findcap(cs, PCI_CAP_ID_MSIX);
        if (msixcap) {
            msixctl = cfgspace_readw(cs, msixcap + PCI_MSIX_FLAGS);
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

    pciehw_cfg_busmaster_enable(phwdev, (cmd & PCI_COMMAND_MASTER) != 0);
}

static void
pciehw_cfgwr_cmd(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int16_t cmd;

    pciehwdev_get_cfgspace(phwdev, &cs);
    cmd = cfgspace_readw(&cs, PCI_COMMAND);
    pciehw_cfg_cmd(phwdev, &cs, cmd);
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
pciehw_cfgwr_bridgectl(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int16_t brctl;

    pciehwdev_get_cfgspace(phwdev, &cs);
    brctl = cfgspace_readw(&cs, PCI_BRIDGE_CONTROL);
    if (brctl & PCI_BRIDGE_CTL_BUS_RESET) {
        const u_int8_t secbus = cfgspace_get_secbus(&cs);
        pciehw_reset_bus(phwdev, secbus);
    }
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
pciehw_cfgwr_vpd(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int16_t vpdcap, addr, f;
    u_int32_t data;

    pciehwdev_get_cfgspace(phwdev, &cs);
    vpdcap = cfgspace_findcap(&cs, PCI_CAP_ID_VPD);
    addr = cfgspace_readw(&cs, vpdcap + PCI_VPD_ADDR);
    f = addr & PCI_VPD_ADDR_F;
    addr &= PCI_VPD_ADDR_MASK;

    /*
     * Flag set indicates write data, clear flag when complete.
     * Flag clear indicates read data, set flag when complete.
     */
    if (f) {
        /* vpd write */
        data = cfgspace_readd(&cs, vpdcap + PCI_VPD_DATA);
        pciehw_vpd_write(phwdev, addr, data);
        cfgspace_writew(&cs, vpdcap + PCI_VPD_ADDR, addr);
    } else {
        /* vpd read */
        data = pciehw_vpd_read(phwdev, addr);
        cfgspace_writed(&cs, vpdcap + PCI_VPD_DATA, data);
        cfgspace_writew(&cs, vpdcap + PCI_VPD_ADDR, addr | PCI_VPD_ADDR_F);
    }
}

static void
pciehw_cfgwr_pcie_devctl(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int16_t pciecap, devctl;

    pciehwdev_get_cfgspace(phwdev, &cs);
    pciecap = cfgspace_findcap(&cs, PCI_CAP_ID_EXP);
    if (stlp_overlap(stlp, pciecap + 0x8, sizeof(u_int16_t))) {
        devctl = cfgspace_readw(&cs, pciecap + 0x8);
        if (devctl & (0x1 << 15)) {
            pciehw_reset_flr(phwdev);
        }
    }
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
 * Enable this VF.  Make it visible on the PCIe bus in cfg space,
 * and enable bars too if Memory Space Enable (mse) is set.
 */
static void
pciehw_sriov_enable_vf(pciehwdev_t *vfhwdev, const int mse)
{
    u_int16_t cmd;

    /* XXX handle vfe load/unload cfg space */
    /* refactor and call pciehw_cfg_load(vfhwdev) */

    /* load/unload the bars */
    cmd = mse ? PCI_COMMAND_MEMORY : 0;
    pciehw_cfg_bars_enable(vfhwdev, cmd);
}

static void
pciehw_sriov_enable_vfs(pciehwdev_t *phwdev, const int numvfs, const int mse)
{
    int vfidx;

    for (vfidx = 0; vfidx < numvfs; vfidx++) {
        pciehwdev_t *vfhwdev = pciehwdev_getvf(phwdev, vfidx);
        pciehw_sriov_enable_vf(vfhwdev, mse);
    }
}

static void
pciehw_sriov_disable_vf(pciehwdev_t *vfhwdev)
{
    const u_int16_t cmd = 0;
    pciehw_cfg_bars_enable(vfhwdev, cmd);

    /* XXX handle vfe load/unload cfg space */
    /* refactor and call pciehw_cfg_unload(vfhwdev) */
}

/*
 * Disable VFs.  Unload the bars and clear bus master enable.
 * We'll reset cfg space for the disable VFs which clears bus master enable.
 */
static void
pciehw_sriov_disable_vfs(pciehwdev_t *phwdev, const int vfb, const int vfc)
{
    int vfidx;

    for (vfidx = vfb; vfidx < vfb + vfc; vfidx++) {
        pciehwdev_t *vfhwdev = pciehwdev_getvf(phwdev, vfidx);
        pciehw_sriov_disable_vf(vfhwdev);
    }
    /* Park disabled vf's in reset state. */
    pciehw_reset_vfs(phwdev, vfb, vfc);
}

/*
 * If VF Enable (vfe) is set, then enable VFs and possibly enable bars
 * if Memory Space Enable (mse) is also set.
 *
 * If VF Enable (vfe) is clear, then disable VFs (mse is ignored).
 */
static void
pciehw_sriov_ctrl_numvfs(pciehwdev_t *phwdev,
                         const u_int16_t ctrl, const u_int16_t numvfs)
{
    const int vfe = (ctrl & PCI_SRIOV_CTRL_VFE) != 0; /* VF Enable */
    const int mse = (ctrl & PCI_SRIOV_CTRL_MSE) != 0; /* Memory Space Enable */

    if (vfe) {
        /*
         * VF Enable set, first disable any enabled VFs greater than numvfs,
         * then enable [0-numvfs) range.
         */
        if (phwdev->enabledvfs > numvfs) {
            pciehw_sriov_disable_vfs(phwdev,
                                     numvfs, phwdev->enabledvfs - numvfs);
        }
        pciehw_sriov_enable_vfs(phwdev, numvfs, mse);
        phwdev->enabledvfs = numvfs;

    } else {
        /*
         * VF Enable clear, disable all enabled VFs.
         */
        if (phwdev->enabledvfs) {
            pciehw_sriov_disable_vfs(phwdev, 0, phwdev->enabledvfs);
            phwdev->enabledvfs = 0;
        }
    }

    /*
     * Generate an event for numvfs change.
     */
    if (phwdev->numvfs != numvfs) {
        pciehw_sriov_numvfs_event(phwdev, numvfs);
        phwdev->numvfs = numvfs;
    }
}

static void
pciehw_sriov_ctrl(pciehwdev_t *phwdev,
                  const u_int16_t ctrl, const u_int16_t numvfs)
{
    if (ctrl != phwdev->sriovctrl) {

        pciesys_loginfo("%s "
                        "sriov_ctrl 0x%04x vfe%c mse%c ari%c numvfs %d\n",
                        pciehwdev_get_name(phwdev),
                        ctrl,
                        ctrl & PCI_SRIOV_CTRL_VFE ? '+' : '-',
                        ctrl & PCI_SRIOV_CTRL_MSE ? '+' : '-',
                        ctrl & PCI_SRIOV_CTRL_ARI ? '+' : '-',
                        numvfs);

        pciehw_sriov_ctrl_numvfs(phwdev, ctrl, numvfs);
        phwdev->sriovctrl = ctrl;
    }
}

static void
pciehw_cfgwr_sriov_ctrl(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int16_t sriovcap, sriovctrl, numvfs;

    pciehwdev_get_cfgspace(phwdev, &cs);
    sriovcap = cfgspace_findextcap(&cs, PCI_EXT_CAP_ID_SRIOV);
    sriovctrl = cfgspace_readw(&cs, sriovcap + PCI_SRIOV_CTRL);

    numvfs = cfgspace_readw(&cs, sriovcap + PCI_SRIOV_NUM_VF);
    numvfs = MIN(numvfs, phwdev->totalvfs);

    pciehw_sriov_ctrl(phwdev, sriovctrl, numvfs);
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
    default:
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
    case PCIEHW_CFGHND_BRIDGECTL:
        pciehw_cfgwr_bridgectl(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_MSIX:
        pciehw_cfgwr_msix(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_VPD:
        pciehw_cfgwr_vpd(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_PCIE_DEVCTL:
        pciehw_cfgwr_pcie_devctl(phwdev, stlp);
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
    const pciehwdevh_t hwdevh = spmt->owner + info->vfid;
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
    const pciehwdevh_t hwdevh = spmt->owner + info->vfid;
    pciehwdev_t *phwdev = pciehwdev_get(hwdevh);

#ifdef PCIEMGR_DEBUG
    pciesys_logdebug("cfgwr_indirect: %s vfid %d wr 0x%lx sz %d data 0x%lx\n",
                     pciehwdev_get_name(phwdev), info->vfid,
                     stlp->addr, stlp->size, stlp->data);
#endif

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

void
pciehw_cfg_reset(pciehwdev_t *phwdev, const pciehdev_rsttype_t rsttype)
{
    cfgspace_t cs;
    u_int16_t cfgsz, cmd;

    pciehwdev_get_cfgspace(phwdev, &cs);
    cfgsz = cfgspace_size(&cs);

    /*****************
     * reset cfg space
     */
    pciehw_memcpy(cs.cur, cs.rst, cfgsz);

    /* Read reset value for cmd */
    cmd = cfgspace_readw(&cs, PCI_COMMAND);
    pciehw_cfg_cmd(phwdev, &cs, cmd);
    /* XXX Reset bar addrs? */

    if (phwdev->pf) {
        u_int16_t sriovcap, sriovctrl, numvfs;

        /* Read reset values for sriovctrl, numvfs. */
        sriovcap = cfgspace_findextcap(&cs, PCI_EXT_CAP_ID_SRIOV);
        sriovctrl = cfgspace_readw(&cs, sriovcap + PCI_SRIOV_CTRL);
        numvfs = cfgspace_readw(&cs, sriovcap + PCI_SRIOV_NUM_VF);
        numvfs = MIN(numvfs, phwdev->totalvfs);

        /* ARI-Capable bit preserved across FLR reset */
        if (rsttype == PCIEHDEV_RSTTYPE_FLR) {
            sriovctrl |= (phwdev->sriovctrl & PCI_SRIOV_CTRL_ARI);
            cfgspace_writew(&cs, sriovcap + PCI_SRIOV_CTRL, sriovctrl);
        }

        pciehw_sriov_ctrl(phwdev, sriovctrl, numvfs);
        /* XXX Reset VF bar addrs? */
    }
}

/******************************************************************
 * debug
 */

/* cfg show flags */
#define CFGF_NONE       0x0
#define CFGF_CUR        0x1     /* show current */
#define CFGF_MSK        0x2     /* show mask */
#define CFGF_RST        0x4     /* show reset */

/*
 * Dump the config space of this device.
 *
 * If we have the default flags CFGF_CUR then dump the output in a
 * format compatible with "lspci".  Then we can take this output and
 * hand it to "lspci -F <file>" for friendly decode.
 */
static void
cfg_show_dev(pciehwdev_t *phwdev, const u_int32_t flags, const int cfgsz)
{
    const char *name = pciehwdev_get_name(phwdev);
    const u_int16_t bdf = pciehwdev_get_hostbdf(phwdev);
    cfgspace_t cfgspace, *cs = &cfgspace;
    int offset, sz;

    pciesys_loginfo("%s %s\n", bdf_to_str(bdf),  name);

    pciehwdev_get_cfgspace(phwdev, cs);
    sz = MIN(cfgspace_size(cs), cfgsz);
    for (offset = 0; offset < sz; offset += 16) {
        const int n = MIN(16, sz - offset);
        char buf[80], *s;

#define SHOW(FLG, fld) \
        if (flags & CFGF_##FLG) { \
            s = hex_format(buf, sizeof(buf), &cs->fld[offset], n); \
            pciesys_loginfo("%s%02x: %s\n", \
                            flags == CFGF_CUR ? "" : #fld " ", offset, s); \
        }

        SHOW(CUR, cur);
        SHOW(RST, rst);
        SHOW(MSK, msk);
    }
}

void
pciehw_cfg_show(int argc, char *argv[])
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    int opt;
    char *name;
    pciehwdev_t *phwdev;
    u_int32_t flags;
    int cfgszidx, cfgsz;
    const int cfgszs[] = { 64, 64, 64, 256, 4096 }; /* match lspci -xxxx */
    const int ncfgszs = sizeof(cfgszs) / sizeof(cfgszs[0]);

    name = NULL;
    flags = CFGF_NONE;
    cfgszidx = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "d:cmrx")) != -1) {
        switch (opt) {
        case 'd':
            name = optarg;
            break;
        case 'c':
            flags |= CFGF_CUR;
            break;
        case 'm':
            flags |= CFGF_MSK;
            break;
        case 'r':
            flags |= CFGF_RST;
            break;
        case 'x':
            cfgszidx++;
            break;
        default:
            return;
        }
    }

    if (flags == CFGF_NONE) flags = CFGF_CUR;
    cfgsz = cfgszs[MIN(cfgszidx, ncfgszs - 1)];

    /*
     * No device name specified, show all devs
     */
    if (name == NULL ) {
        int i;

        phwdev = &pshmem->dev[1];
        for (i = 1; i <= pshmem->allocdev; i++, phwdev++) {
            cfg_show_dev(phwdev, flags, cfgsz);
        }
    } else {
        phwdev = pciehwdev_find_by_name(name);
        if (phwdev == NULL) {
            pciesys_logerror("device %s not found\n", name);
            return;
        }
        cfg_show_dev(phwdev, flags, cfgsz);
    }
}
