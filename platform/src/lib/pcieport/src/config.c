/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "pci_ids.h"
#include "pal.h"
#include "pciesys.h"
#include "pciehw_dev.h"
#include "pcieport.h"
#include "pcieport_impl.h"

static void
pcieport_mac_k_gen(pcieport_t *p)
{
    const int pn = p->port;
    u_int64_t gen;

    gen = pal_reg_rd64_safe(PXC_(CFG_C_MAC_K_GEN, pn));

    /* set defaults */
    gen &= 0xffffffff00000000ULL;
    gen |= 0x80e20050;

    if (p->sris) {
        gen |= (1 << 8); /* sris_mode */
    }

    /* select gen speed, and spec_version */
    switch (p->cap_gen) {
    case 1: gen |= (0x1 << 9) | 3; break;
    case 2: gen |= (0x3 << 9) | 3; break;
    case 3: gen |= (0x7 << 9) | 3; break;
    case 4: gen |= (0xf << 9) | 4; break;
    }

    switch (p->cap_width) {
    case 1: gen |= (0xf << 24); break;
    case 2: gen |= (0xe << 24); break;
    case 4: gen |= (0xc << 24); break;
    case 8: gen |= (0x8 << 24); break;
    case 16: /* 16 is default */ break;
    }

    pal_reg_wr64_safe(PXC_(CFG_C_MAC_K_GEN, pn), gen);
}

/*
 * k_pexconf is documented in
 *     PLDA XpressRICH4 Reference Manual
 *     Version 1.9.3 January 2018 - Pre-release draft
 */
static void
pcieport_mac_k_pexconf(pcieport_t *p)
{
    const int pn = p->port;
    u_int32_t pexconf[12];

    pal_reg_rd32w(PXC_(CFG_C_MAC_K_PEXCONF, pn), pexconf, 12);

    /*****************
     * word1
     */
    /* disable Completion Timeout Disable sup - endpoints only */
    pexconf[1] &= ~(1 << 4);  /* [36] Completion Timeout Disable sup */
    if (p->cap_gen < 4) {
        pexconf[1] &= ~(1 << 16); /* [48] 10-bit Completer Tag sup */
        pexconf[1] &= ~(1 << 17); /* [49] 10-bit Requester Tag sup */
    }

    /*****************
     * word2
     */
    /* [64:73] disable gen4-only capabilities if not gen4 */
    if (p->cap_gen < 4) {
        pexconf[2] &= ~0x1ff;
    }
    /* [74:75] claim ASPM L0s, L1 not supported (respectively) */
    pexconf[2] &= ~(0x3 << 10);

    /*****************
     * word5
     */
    /*
     * For compliance testing claim we don't have AER
     * so no surprises show up in the AER registers.
     */
    if (p->compliance) {
        pexconf[5] &= ~(1 << 31); /* [191] AER implemented */
    }

    pal_reg_wr32w(PXC_(CFG_C_MAC_K_PEXCONF, pn), pexconf, 12);
}

static void
pcieport_mac_k_pciconf(pcieport_t *p)
{
    const int pn = p->port;
    u_int32_t val;
    extern int vga_support;

    /* class code */
    val = 0x06040000;
    pal_reg_wr32(PXC_(CFG_C_MAC_K_PCICONF, pn) + 4, val);

    val = pal_reg_rd32(PXC_(CFG_C_MAC_K_PCICONF, pn) + 0xc);
    /* vga supported */
    val &= ~(1 << 4);
    val |= (vga_support << 4);
    /* legacy power management disabled */
    val &= ~(0x7ff << 21);
    pal_reg_wr32(PXC_(CFG_C_MAC_K_PCICONF, pn) + 0xc, val);
}

static void
pcieport_mac_k_rx_cred(pcieport_t *p)
{
    if (!pal_is_asic()) {
        u_int32_t val = 0x00200080;
        pal_reg_wr32(PXC_(CFG_C_MAC_K_RX_CRED, p->port), val);
    }
}

static void
pcieport_mac_set_ids(pcieport_t *p)
{
    const int pn = p->port;
    u_int32_t val;

    /* set subvendor/deviceid */
    val = (p->subdeviceid << 16) | p->subvendorid;
    pal_reg_wr32(PXC_(CFG_C_MAC_SSVID_CAP, pn), val);
}

/*
 * Select local or host as source for pcie refclk.
 * Hw default is local refclk.
 */
static void
pcieport_select_pcie_refclk(const int host_clock)
{
    if (host_clock) {
        pal_reg_wr32(PP_(CFG_PP_PCIE_PLL_REFCLK_SEL), 0xff);
        pal_reg_wr32(PP_(CFG_PP_PCIE_PLL_REFCLK_SOURCE_SEL), 0x3);
    } else {
        pal_reg_wr32(PP_(CFG_PP_PCIE_PLL_REFCLK_SEL), 0x00);
        pal_reg_wr32(PP_(CFG_PP_PCIE_PLL_REFCLK_SOURCE_SEL), 0x0);
    }
}

static void
pcieport_unreset(pcieport_t *p)
{
    u_int32_t val = pal_reg_rd32(PP_(CFG_PP_SW_RESET));
    val &= ~(0x3 << (p->port << 1));
    pal_reg_wr32(PP_(CFG_PP_SW_RESET), val);
}

static int
pcieport_mac_unreset(pcieport_t *p)
{
    u_int16_t phystatus;
    int perstn;
    const int maxpolls = 2000; /* 2 seconds */
    int polls = 0;

    do {
        usleep(1000);
        phystatus = pcieport_get_phystatus(p);
        perstn = pcieport_get_perstn(p);
    } while (phystatus && perstn && ++polls < maxpolls);

    p->phypolllast = polls;
    if (polls > p->phypollmax) {
        p->phypollmax = polls;
    }

    if (!perstn) {
        /*
         * perstn went away - we went back into reset
         */
        p->phypollperstn++;
        return -1;
    }

    if (phystatus != 0) {
        /*
         * PHY didn't come out of reset as expected?
         */
        p->phypollfail++;
        return -1;
    }

    pcieport_set_mac_reset(p, 0); /* mac unreset */
    return 0;
}

static void
pcieport_system(const char *path)
{
    int r;

    pciesys_loginfo("running %s:\n", path);
    r = system(path);
    if (r) pciesys_logerror("%s: failed %d\n", path, r);
}

static void
pcieport_run_script(const char *name, const int port)
{
    char path[80];

#ifdef __aarch64__
    snprintf(path, sizeof(path), "/sysconfig/config0/%s", name);
    if (access(path, X_OK) == 0) pcieport_system(path);

    snprintf(path, sizeof(path), "/sysconfig/config0/%s-%d", name, port);
    if (access(path, X_OK) == 0) pcieport_system(path);

    snprintf(path, sizeof(path), "/tmp/%s", name);
    if (access(path, X_OK) == 0) pcieport_system(path);

    snprintf(path, sizeof(path), "/tmp/%s-%d", name, port);
    if (access(path, X_OK) == 0) pcieport_system(path);
#else
    snprintf(path, sizeof(path), "./%s", name);
    if (access(path, X_OK) == 0) pcieport_system(path);

    snprintf(path, sizeof(path), "./%s-%d", name, port);
    if (access(path, X_OK) == 0) pcieport_system(path);
#endif
}

static void
pcieport_poweron_script(pcieport_t *p)
{
    pcieport_run_script("pcie-poweron", p->port);
}

static int
pcieport_config_host(pcieport_t *p)
{
    int otrace;

    otrace = pal_reg_trace_control(getenv("PCIEPORT_INIT_TRACE") != NULL);
    pal_reg_trace("================ pcieport_hostconfig %d start\n", p->port);

    if (!pal_is_asic()) {
        /* toggle these resets */
        pcieport_set_serdes_reset(p, 1);
        pcieport_set_pcs_reset(p, 1);
        pcieport_set_serdes_reset(p, 0);
        pcieport_set_pcs_reset(p, 0);
    } else {
        static int done_once;

        if (!done_once) {
            int host_clock = 1;
            char *env = getenv("PCIEPORT_HOST_CLOCK");

            if (env) {
                host_clock = strtoul(env, NULL, 0);
                pciesys_loginfo("host_clock override %d\n", host_clock);
            }

            pcieport_select_pcie_refclk(host_clock);
            pcieport_serdes_init();
            done_once = 1;
        }
        pcieport_set_pcs_reset(p, 1);
        pcieport_set_pcs_reset(p, 0);
    }

    pcieport_unreset(p);

    pcieport_mac_k_gen(p);
    pcieport_mac_k_rx_cred(p);
    pcieport_mac_k_pexconf(p);
    pcieport_mac_k_pciconf(p);
    pcieport_mac_set_ids(p);

    pcieport_poweron_script(p);

    /* now ready to unreset mac */
    if (pcieport_mac_unreset(p) < 0) {
        return -1;
    }

    /*
     * "AER common" controls whether we capture errors
     * from any downstream device and report them on the
     * upstream port bridge.  We ran into a BIOS (HPE Proliant)
     * that didn't like seeing "unsupported request" errors reported
     * in the bridge after the bus scan, and would report CATERR
     * and power off the host.  Safer to disable this, although
     * some might want to see all our errors reported on AER.
     */
    pcieport_set_aer_common_en(p, 0);

    if (!pal_is_asic()) {
        /* reduce clock frequency for fpga */
        pcieport_set_clock_freq(p, 8);
    }

    pcieport_set_ltssm_en(p, 1);  /* ready for ltssm */
    pal_reg_trace("================ pcieport_hostconfig %d end\n", p->port);
    pal_reg_trace_control(otrace);
    return 0;
}

static int
pcieport_config_rc(pcieport_t *p)
{
    /* XXX */
    assert(0);
    return 0;
}

int
pcieport_config(pcieport_t *p)
{
    int r;

    if (!p->config) return -1;

    if (p->host) {
        r = pcieport_config_host(p);
    } else {
        r = pcieport_config_rc(p);
    }
    return r;
}
