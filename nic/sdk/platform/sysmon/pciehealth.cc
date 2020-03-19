/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>
#include <sys/param.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#include "platform/evutils/include/evutils.h"
#include "platform/pal/include/pal.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/pciemgr/include/pciehw.h"
#include "platform/pciemgr/include/pciehw_dev.h"
#include "platform/pcieport/include/pcieport.h"
#include "platform/pcieport/include/portcfg.h"
#include "platform/pcieport/include/portmap.h"

#include "sysmon_internal.hpp"

//
// This module monitors various parameters of the pcie subsystem
// watching for link errors or signs the pcie link is unhealthy.
// If a potential link health issue is detected then we trigger
// the pcie_healthlog.sh script to collect and log pcie link data
// to debug.
//
// We want to make some effort not to fill up the logs once a
// link error has been detected, so we keep track of what we have
// logged about and only log again if more or different errors are
// detected, and we put a throttle on the frequency of logging in
// an attempt not to log errors too frequently when/if they do appear.
//
// We don't rely too much on software infrastructure except for
// periodic timers so we can monitor even if there are software bugs
// in our link state machine or something.  We don't want to depend
// on link events or other software generated events to trigger
// us since those events might not come if there are bugs.
//

#define SEC_TO_USEC(s)          (((uint64_t)(s)) * 1000000)
#define MIN_TO_USEC(m)          SEC_TO_USEC((m) * 60)

#define LOG_DELAY_MIN_US        MIN_TO_USEC(1)
#define LOG_DELAY_MAX_US        MIN_TO_USEC(60)
#define LOG_DELAY_RST_US        MIN_TO_USEC(60)
#define LINK_UP_TIMEOUT         MIN_TO_USEC(5)
#define LTSSMST_THRESHOLD       20

typedef struct pcie_health_info_s {
    pcieportst_t portst;        // pcieport state
    uint64_t intr_ltssmst;      // ltssm_state_changed
    uint32_t aer_uesta;         // AER Uncorrectable Error Status
    uint32_t aer_cesta;         // AER Correctable Error Status
    uint32_t physl_sta;         // physical layer cap status
    uint16_t lnksta2;           // PCIE cap Link Status 2
    int recovery;               // core_initiated_recovery
    int cap_gen;                // capable speed
    int cap_width;              // capable width
    int cur_gen;                // current speed
    int cur_width;              // current width
    int linkuptmo;              // link up timeout
    uint64_t tstamp;            // time this was gathered
} pcie_health_info_t;

typedef struct pcie_health_state_s {
    pcie_health_info_t hinew;   // current health
    pcie_health_info_t hiold;   // last event detected
    pcie_health_info_t hilog;   // last event logged
    uint64_t macuptm;           // mac came up out of reset
    uint64_t logdelay;          // delay to next time to log
    char reason[80];            // reason for current event
} pcie_health_state_t;

static pcie_health_state_t health_state[PCIEPORT_NPORTS];

// should be in pci_regs.h?
#define PCI_EXP_LNKSTA2_EQ      0x0002  // equalization complete
#define PCI_EXP_LNKSTA2_EQ1     0x0004  // equalization phase1 success
#define PCI_EXP_LNKSTA2_EQ2     0x0008  // equalization phase2 success
#define PCI_EXP_LNKSTA2_EQ3     0x0010  // equalization phase3 success
#define PCI_EXP_LNKSTA2_EQ_ALL  (PCI_EXP_LNKSTA2_EQ  | \
                                 PCI_EXP_LNKSTA2_EQ1 | \
                                 PCI_EXP_LNKSTA2_EQ2 | \
                                 PCI_EXP_LNKSTA2_EQ3)

// Get timestamp in microseconds.
static uint64_t
get_timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

static void set_reason(pcie_health_state_t *hs, const char *fmt, ...)
    __attribute__((format (printf, 2, 3)));
static void
set_reason(pcie_health_state_t *hs, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(hs->reason, sizeof(hs->reason), fmt, ap);
    va_end(ap);
}

static void
pcie_healthlog(const int port, const char *reason)
{
    // increment stats here before running healthlog below
    pciemgr_stats_t *stats = pciehw_stats_get(port);
    stats->healthlog++;

    char cmd[80];
    snprintf(cmd, sizeof(cmd),
             "/nic/tools/pcie_healthlog.sh %d \"%s\"", port, reason);

    int r = evutil_system(EV_DEFAULT_ cmd, NULL, NULL);
    if (r < 0) pciesys_logerror("pcie_healthlog %d failed %d\n", port, r);
}

static void
gather_health_info(const int port)
{
    pcie_health_state_t *hs = &health_state[port];
    pcie_health_info_t *nhi = &hs->hinew;

    memset(nhi, 0, sizeof(*nhi));
    nhi->tstamp = get_timestamp();
    pcieport_t *p = pcieport_get(port);
    if (p) {
        nhi->portst = p->state;
        nhi->intr_ltssmst = p->stats.intr_ltssmst;
        if (p->state >= PCIEPORTST_MACUP) {
            // save (first) mac up time, so we can check linkup timeout
            if (!hs->macuptm) hs->macuptm = nhi->tstamp;
            if (p->state < PCIEPORTST_UP &&
                nhi->tstamp - hs->macuptm > LINK_UP_TIMEOUT) {
                nhi->linkuptmo = 1; // waited long enough for link up
            }
        } else {
            // not up, reset macuptm
            if (hs->macuptm) hs->macuptm = 0;
        }
        if (pcieport_is_accessible(port)) {
            nhi->cap_gen = p->cap_gen;
            nhi->cap_width = p->cap_width;
            portcfg_read_genwidth(port, &nhi->cur_gen, &nhi->cur_width);
        }
    }
    if (pcieport_is_accessible(port)) {
        nhi->lnksta2 = portcfg_readw(port, PORTCFG_CAP_PCIE + 0x32);
        nhi->aer_uesta = portcfg_readd(port, PORTCFG_CAP_AER + 0x4);
        nhi->aer_cesta = portcfg_readd(port, PORTCFG_CAP_AER + 0x10);

        uint64_t pa = PXP_(SAT_P_PORT_CNT_CORE_INITIATED_RECOVERY, port);
        nhi->recovery = pal_reg_rd32(pa);
#if 0 // XXX gen4 TODO
        if (nhi->cur_width == 4) {
            nhi->physl_sta = portcfg_readd(port, PORTCFG_CAP_PHYSLAYER + 0xc);
        }
#endif
    }
}

static void
process_health_info(const int port)
{
    pcie_health_state_t *hs = &health_state[port];
    pcie_health_info_t *nhi = &hs->hinew;
    pcie_health_info_t *ohi = &hs->hiold;
    pcie_health_info_t *lhi = &hs->hilog;

    // check for (first) link up timeout
    if (nhi->linkuptmo && !lhi->linkuptmo) {
        set_reason(hs, "link up timeout");
        goto want_log;
    }
    // current link parameters match expected capability
    // Only complain if we don't match expected, *and*
    // we didn't complain about this last time.
    if ((nhi->cur_gen &&
         nhi->cur_gen <  nhi->cap_gen &&
         nhi->cur_gen != lhi->cur_gen) ||
        (nhi->cur_width &&
         nhi->cur_width <  nhi->cap_width &&
         nhi->cur_width != lhi->cur_width)) {
        set_reason(hs, "gen%dx%d (capable gen%dx%d)",
                   nhi->cur_gen, nhi->cur_width, nhi->cap_gen, nhi->cap_width);
        goto want_log;
    }
    // new core_initiated_recovery events
    if (nhi->recovery > lhi->recovery) {
        set_reason(hs, "recovery %d", nhi->recovery);
        goto want_log;
    }
    // newly up in gen3, check gen3 equalization complete
    if ((nhi->portst >= PCIEPORTST_UP && nhi->cur_gen == 3) &&
        (lhi->portst <  PCIEPORTST_UP || lhi->cur_gen != 3) &&
        (nhi->lnksta2 & PCI_EXP_LNKSTA2_EQ_ALL) != PCI_EXP_LNKSTA2_EQ_ALL) {
        set_reason(hs, "lnksta2 0x%04x", nhi->lnksta2);
        goto want_log;
    }
    // any new AER uncorrectable errors
    if (nhi->aer_uesta ^ lhi->aer_uesta) {
        set_reason(hs, "uesta 0x%08x", nhi->aer_uesta);
        goto want_log;
    }
    // any new AER correctable errors (disabled due to NonFatalErr+)
    if (0 && nhi->aer_cesta ^ lhi->aer_cesta) {
        set_reason(hs, "cesta 0x%08x", nhi->aer_cesta);
        goto want_log;
    }
    // ltssmst changes happen occasionally,
    // wait for "enough" to report.
    if (nhi->intr_ltssmst - lhi->intr_ltssmst >= LTSSMST_THRESHOLD) {
        set_reason(hs, "intr_ltssmst %" PRIu64, nhi->intr_ltssmst);
        goto want_log;
    }
    return;

 want_log:
    // reset delay if enough time elapsed with no errors detected
    if (nhi->tstamp - ohi->tstamp > LOG_DELAY_RST_US) {
        hs->logdelay = 0;
    }
    // no more log events until enough time since last logged event
    if (nhi->tstamp - lhi->tstamp > hs->logdelay) {
        pciesys_logerror("port%d healthlog %s\n", port, hs->reason);
        pcie_healthlog(port, hs->reason);
        hs->hilog = *nhi; // save last logged event

        // increase log throttle delay
        hs->logdelay = hs->logdelay ? hs->logdelay * 2 : LOG_DELAY_MIN_US;
        hs->logdelay = MIN(hs->logdelay, LOG_DELAY_MAX_US);
    }
    hs->hiold = *nhi; // save last event
}

static void
pciehealth_port(const int port, void *arg)
{
    gather_health_info(port);
    process_health_info(port);
}

static int
portmap_init_from_catalog(sdk::lib::catalog *catalog)
{
    static int inited;

    if (inited) return 0;

    portmap_init();
#ifdef __aarch64__
    int nportspecs = catalog->pcie_nportspecs();
    for (int i = 0; i < nportspecs; i++) {
        pcieport_spec_t ps = { 0 };
        ps.host  = catalog->pcie_host(i);
        ps.port  = catalog->pcie_port(i);
        ps.gen   = catalog->pcie_gen(i);
        ps.width = catalog->pcie_width(i);
        if (portmap_addhost(&ps) < 0) {
            pciesys_logerror("portmap_add i %d h%d p%d gen%dx%d failed\n",
                             i, ps.host, ps.port, ps.gen, ps.width);
            return -1;
        }
    }
#else
    pcieport_spec_t ps = { 0 };
    ps.host  = 0;
    ps.port  = 0;
    ps.gen   = 3;
    ps.width = 16;

    if (portmap_addhost(&ps) < 0) {
        pciesys_logerror("portmap_add h%d p%d gen%dx%d failed\n",
                         ps.host, ps.port, ps.gen, ps.width);
        return -1;
    }
#endif
    inited = 1;
    return 0;
}

static char *
logfmt(const char *fmt, va_list ap)
{
    static char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    const int len = strlen(buf);
    if (len && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return buf;
}

static void
logdebug(const char *fmt, va_list ap)
{
    SDK_TRACE_DEBUG("%s", logfmt(fmt, ap));
}

static void
loginfo(const char *fmt, va_list ap)
{
    SDK_TRACE_INFO("%s", logfmt(fmt, ap));
}

static void
logwarn(const char *fmt, va_list ap)
{
    SDK_TRACE_WARN("%s", logfmt(fmt, ap));
}

static void
logerror(const char *fmt, va_list ap)
{
    SDK_TRACE_ERR("%s", logfmt(fmt, ap));
}

static void
logflush(void)
{
}

static pciesys_logger_t pciesys_logger = {
    .logdebug = logdebug,
    .loginfo  = loginfo,
    .logwarn  = logwarn,
    .logerror = logerror,
    .logflush = logflush,
};

void
checkpciehealth(void)
{
    static int inited;

    if (!inited) {
        pciesys_set_logger(&pciesys_logger);
        pciehdev_open(NULL);
        portmap_init_from_catalog(g_sysmon_cfg.catalog);
        inited = 1;
    }
    portmap_foreach_port(pciehealth_port, NULL);
}
