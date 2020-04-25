/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <sys/param.h>
#include <linux/pci_regs.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#include "platform/evutils/include/evutils.h"
#include "platform/misc/include/misc.h"
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

//
// LOG_DELAY_MIN_US - Minimum throttle time to wait after first
//     log event of a series.
//
#define LOG_DELAY_MIN_US        MIN_TO_USEC(1)
//
// LOG_DELAY_MAX_US - Maximum time to wait for next log event of a series.
//     Once we reach this log throttle time we don't increase the timeout.
//
#define LOG_DELAY_MAX_US        MIN_TO_USEC(60)
//
// LOG_DELAY_RST_US - Quiet period between log events to trigger delay reset.
//     If we don't get any events for this time, reset log throttle to 0.
//
#define LOG_DELAY_RST_US        MIN_TO_USEC(60)
//
// LINK_UP_TIMEOUT - Time to wait after mac comes out of reset for the link to
//     come up.  If the link doesn't come up in this time trigger a log event.
//
#define LINK_UP_TIMEOUT         MIN_TO_USEC(5)
//
// HOST_UP_CHECKTM - Time after "host up" (bios scan) to wait before checking
//     settings controlled by OS (exttags, maxpayload, maxreadreq).  Some AMD
//     systems BIOS's set conservative maxpayload, then the OS later sets
//     better performance settings.
//
#define HOST_UP_CHECKTM         MIN_TO_USEC(5)
//
// MACUP_THRESHOLD - Number of macup events before the link comes up
//     that trigger a log event.  This will catch a link bouncing
//     up/down before the LINKUP event.  (After the LINKUP event then
//     LTSSMST_THRESHOLD will catch link issues.)
//
#define MACUP_THRESHOLD         10
//
// LTSSMST_THRESHOLD - Number of ltssm_state_changed interrupts we need
//     in this session before we consider this a log event.  We sometimes
//     see ltssm_state_changed interrupts at startup as the link negotiation
//     happens and the link settles.  Sometimes see 22 of these.
//
#define LTSSMST_THRESHOLD       25
//
// TXFC_PHDR_MIN  - For writes, minimum tx fc credits posted header.
// TXFC_PDATA_MIN - For writes, minimum tx fc credits posted data.
// TXFC_NPHDR_MIN - For reads,  minimum tx fc credits nonposted header.
//
// Wanted PHDR=127, PDATA=500, NPHDR=127 here,
// but found HPE DL380 server provides PHDR=120, PDATA=352, NPHDR=94,
// so lower mins to accommodate DL380.
//
#define TXFC_PHDR_MIN           100
#define TXFC_PDATA_MIN          350
#define TXFC_NPHDR_MIN          90

typedef struct pcie_health_info_s {
    uint64_t tstamp;            // time this was gathered
    uint64_t faults;            // portstats faults
    uint64_t phypollfail;       // portstats phypollfail
    uint64_t intr_ltssmst;      // portstats ltssm_state_changed
    pcieportst_t portst;        // pcieport state
    uint32_t macup;             // port macup counter
    uint32_t aer_uesta;         // AER Uncorrectable Error Status
    uint32_t aer_cesta;         // AER Correctable Error Status
    uint32_t physl_sta;         // physical layer cap status
    uint16_t devctl;            // PCIE cap Device Control
    uint16_t lnksta2;           // PCIE cap Link Status 2
    uint32_t recovery;          // core_initiated_recovery
    uint32_t linkuptmo;         // link up timeout
    uint32_t host_checks;       // enable host-side cfg checks
    int cap_gen;                // capable speed
    int cap_width;              // capable width
    int cur_gen;                // current speed
    int cur_width;              // current width
    int txfc_phdr;              // tx fc credits    posted hdr
    int txfc_pdata;             // tx fc credits    posted data
    int txfc_nphdr;             // tx fc credits nonposted hdr
    int txfc_npdata;            // tx fc credits nonposted data
} pcie_health_info_t;

typedef struct pcie_health_state_s {
    pcie_health_info_t hinew;   // current health
    pcie_health_info_t hiold;   // last event detected
    pcie_health_info_t hilog;   // last event logged
    uint32_t macup_base;        // port macup baseline
    uint32_t macupgen;          // mac up generation
    uint64_t macuptm;           // mac came up out of reset
    uint64_t linkuptm;          // link came up
    uint64_t hostuptm;          // host came up (bios scan started)
    uint64_t lasteventtm;       // last event time
    uint64_t lastlogtm;         // last event logged time
    uint64_t logdelay;          // delay to next time to log
    uint64_t ltssmst_base;      // intr_ltssmst baseline
    sysmon_pciehealth_severity_t sev; // severity of current event
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

#define set_reason(hs, severity, fmt, ...) \
    do { \
        (hs)->sev = severity; \
        snprintf((hs)->reason, sizeof((hs)->reason), fmt, ##__VA_ARGS__); \
    } while (0)

#define set_warn(hs, fmt, ...) \
    set_reason(hs, SYSMON_PCIEHEALTH_WARN, fmt, ##__VA_ARGS__)

#define set_error(hs, fmt, ...) \
    set_reason(hs, SYSMON_PCIEHEALTH_ERROR, fmt, ##__VA_ARGS__)

// Get timestamp in microseconds.
static uint64_t
get_timestamp(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static void
pcie_healthlog(const int port, const char *reason)
{
    pciesys_logerror("port%d healthlog %s\n", port, reason);

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
pcie_health_event(const int port)
{
    pcie_health_state_t *hs = &health_state[port];

    pcie_healthlog(port, hs->reason);

    if (g_sysmon_cfg.pciehealth_event_cb) {
        g_sysmon_cfg.pciehealth_event_cb(hs->sev, hs->reason);
    }
}

static void
gather_health_info(const int port)
{
    pcie_health_state_t *hs = &health_state[port];
    pcie_health_info_t *nhi = &hs->hinew;
    const uint64_t now = get_timestamp();

    memset(nhi, 0, sizeof(*nhi));
    nhi->tstamp = now;

    pcieport_t *p = pcieport_get(port);
    if (!p) return;

    // not up (or bounced), reset macuptm/gen
    if (p->state < PCIEPORTST_MACUP || hs->macupgen != p->macup) {
        if (hs->macuptm) {
            pciesys_loginfo("gather%d: reset with gen %d\n", port, p->macup);
            hs->macuptm = 0;
        }
        hs->ltssmst_base = 0;
        hs->linkuptm = 0;
        hs->hostuptm = 0;
        hs->macupgen = p->macup;
    }

    // mac is up
    if (p->state >= PCIEPORTST_MACUP) {
        // save (first) mac up time, so we can check linkup timeout
        if (!hs->macuptm) {
            hs->macuptm = now;
            pciesys_loginfo("gather%d: new mac up time\n", port);
        }
    }

    // link is up
    if (p->state >= PCIEPORTST_LINKUP) {
        // reset ltssmst/macup baselines once the link is up
        if (!hs->linkuptm) {
            hs->linkuptm = now;
            hs->ltssmst_base = p->stats.intr_ltssmst; // save baseline
            hs->macup_base = p->macup;
            pciesys_loginfo("gather%d: new linkup baselines "
                            "macup %u ltssmst %" PRIu64 "\n",
                            port, hs->macup_base, hs->ltssmst_base);
        }
    }

    // link not yet up, but mac up for a while
    if (p->state < PCIEPORTST_UP && hs->macuptm &&
        now - hs->macuptm > LINK_UP_TIMEOUT) {
        nhi->linkuptmo = hs->macupgen; // waited long enough for link up
    }

    // host up - save host up time, enable host checks
    if (p->state >= PCIEPORTST_UP) {
        if (!hs->hostuptm) {
            hs->hostuptm = now;
            pciesys_loginfo("gather%d: new host up time\n", port);
        }
        // wait for host os to get a chance to change host settings
        if (now - hs->hostuptm > HOST_UP_CHECKTM) {
            nhi->host_checks = hs->macupgen;
        }
    }

    nhi->portst = p->state;
    nhi->faults = p->stats.faults;
    nhi->phypollfail = p->stats.phypollfail;
    nhi->intr_ltssmst = p->stats.intr_ltssmst;
    nhi->macup = p->macup;
    // tx fc credits sample
    nhi->txfc_pdata  = p->txfc_pdata;
    nhi->txfc_phdr   = p->txfc_phdr;
    nhi->txfc_npdata = p->txfc_npdata;
    nhi->txfc_nphdr  = p->txfc_nphdr;

    if (pcieport_is_accessible(port)) {
        nhi->cap_gen   = p->cap_gen;
        nhi->cap_width = p->cap_width;
        portcfg_read_genwidth(port, &nhi->cur_gen, &nhi->cur_width);
        nhi->devctl    = portcfg_readw(port, PORTCFG_CAP_PCIE + 0x8);
        nhi->lnksta2   = portcfg_readw(port, PORTCFG_CAP_PCIE + 0x32);
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

//
// Process the health information to determine if a health log event
// needs to be generated for the current state.
//
// N.B. Each test should somehow take into account the state at the
// last log event in "hilog".  Compare against what was logged about
// before to generate log events only for *new* conditions.  This will
// prevent an error condition from continuously flooding the logs with
// the same health event.
//
static void
process_health_info(const int port)
{
    pcie_health_state_t *hs = &health_state[port];
    const pcie_health_info_t *nhi = &hs->hinew;
    const pcie_health_info_t *lhi = &hs->hilog;

// check if new and log events are from same boot session
#define same_session(nhi, lhi) \
    (nhi->macup == lhi->macup)

    // faults stat
    if (nhi->faults > lhi->faults) {
        set_error(hs, "faults %" PRIu64, nhi->faults);
        goto want_log;
    }
    // phypollfail stat
    if (nhi->phypollfail > lhi->phypollfail) {
        set_error(hs, "phypollfail %" PRIu64, nhi->phypollfail);
        goto want_log;
    }
    // core_initiated_recovery events
    if (nhi->recovery > lhi->recovery) {
        set_warn(hs, "recovery %d", nhi->recovery);
        goto want_log;
    }
    // mac bouncing up/down
    if ((same_session(nhi, lhi) &&
         nhi->macup - lhi->macup >= MACUP_THRESHOLD) ||
        (!same_session(nhi, lhi) &&
         nhi->macup - hs->macup_base >= MACUP_THRESHOLD)) {
        set_error(hs, "mac flaps %u", nhi->macup);
        goto want_log;
    }
    // link up timeout
    if (nhi->linkuptmo > lhi->linkuptmo) {
        set_warn(hs, "link up timeout");
        goto want_log;
    }
    // current link parameters match expected capability
    // Only complain if we don't match expected, *and*
    // we didn't complain about this last time.
#define check_genwidth(field) \
    (nhi->cur_##field && \
     nhi->cur_##field < nhi->cap_##field && \
     (nhi->cur_##field != lhi->cur_##field || !same_session(nhi, lhi)))
    if (check_genwidth(gen) || check_genwidth(width)) {
        set_warn(hs, "gen%dx%d (capable gen%dx%d)",
                 nhi->cur_gen, nhi->cur_width, nhi->cap_gen, nhi->cap_width);
        goto want_log;
    }
    // newly up in gen3, check gen3 equalization complete
    if ((nhi->portst >= PCIEPORTST_UP && nhi->cur_gen == 3) &&
        (lhi->portst <  PCIEPORTST_UP || lhi->cur_gen != 3 ||
         !same_session(nhi, lhi)) &&
        (nhi->lnksta2 & PCI_EXP_LNKSTA2_EQ_ALL) != PCI_EXP_LNKSTA2_EQ_ALL) {
        set_warn(hs, "lnksta2 0x%04x", nhi->lnksta2);
        goto want_log;
    }
    // any new AER uncorrectable errors
    if (nhi->aer_uesta &&
        (nhi->aer_uesta != lhi->aer_uesta || !same_session(nhi, lhi))) {
        set_error(hs, "uesta 0x%08x", nhi->aer_uesta);
        goto want_log;
    }
    // any new AER correctable errors (disabled due to NonFatalErr+)
    if (0 && nhi->aer_cesta &&
        (nhi->aer_cesta != lhi->aer_cesta || !same_session(nhi, lhi))) {
        set_warn(hs, "cesta 0x%08x", nhi->aer_cesta);
        goto want_log;
    }
    // ltssmst changes happen occasionally,
    // wait for "enough" to report.
    if ((same_session(nhi, lhi) &&
         (nhi->intr_ltssmst - lhi->intr_ltssmst >= LTSSMST_THRESHOLD)) ||
        (!same_session(nhi, lhi) &&
         (nhi->intr_ltssmst - hs->ltssmst_base >= LTSSMST_THRESHOLD))) {
        set_warn(hs, "intr_ltssmst %" PRIu64, nhi->intr_ltssmst);
        goto want_log;
    }
    // perf: check tx fc credits
    // Note -1 means infinite credits here.
#define check_txfc(txfc_field, lowval, hival) \
    (nhi->txfc_field >= (lowval) && nhi->txfc_field < (hival) && \
     nhi->txfc_field != lhi->txfc_field)
    if (nhi->portst >= PCIEPORTST_UP &&
        (check_txfc(txfc_phdr,  0, TXFC_PHDR_MIN) ||
         check_txfc(txfc_pdata, 0, TXFC_PDATA_MIN) ||
         check_txfc(txfc_nphdr, 0, TXFC_NPHDR_MIN))) {
        set_warn(hs, "tx fc credits post %d/%d nonpost %d/%d",
                 nhi->txfc_phdr,  nhi->txfc_pdata,
                 nhi->txfc_nphdr, nhi->txfc_npdata);
        goto want_log;
    }
    // perf: check devctl ExtTag, MaxPayload, MaxReadReq
    // for best results, we want full hw capability selected
    if (nhi->host_checks > lhi->host_checks) {
        const int enc_to_sz[8] = { 128, 256, 512, 1024, 2048, 4096, -6, -7 };

        if ((nhi->devctl & PCI_EXP_DEVCTL_EXT_TAG) == 0) {
            set_warn(hs, "devctl 0x%04x exttag disabled", nhi->devctl);
            goto want_log;
        }
        const uint16_t payloadenc = (nhi->devctl >>  5) & 0x7;
        const int payloadsz = enc_to_sz[payloadenc];
        if (payloadsz < 256) {
            set_warn(hs, "devctl 0x%04x maxpayload %d",
                     nhi->devctl, payloadsz);
            goto want_log;
        }
        const uint16_t readreqenc = (nhi->devctl >> 12) & 0x7;
        const int readreqsz = enc_to_sz[readreqenc];
        if (readreqsz < 512) {
            set_warn(hs, "devctl 0x%04x maxreadreq %d",
                     nhi->devctl, readreqsz);
            goto want_log;
        }
    }
    return;

 want_log:
    // reset delay if enough time elapsed with no errors detected
    if (nhi->tstamp - hs->lasteventtm > LOG_DELAY_RST_US) {
        pciesys_loginfo("port%d reset logdelay\n", port);
        hs->logdelay = 0;
    }
    // no more log events until enough time since last logged event
    if (nhi->tstamp - hs->lastlogtm > hs->logdelay) {
        pcie_health_event(port);
        hs->hilog = *nhi; // save last logged event
        hs->lastlogtm = nhi->tstamp;

        // increase log throttle delay
        hs->logdelay = hs->logdelay ? hs->logdelay * 2 : LOG_DELAY_MIN_US;
        hs->logdelay = MIN(hs->logdelay, LOG_DELAY_MAX_US);
    }
    hs->hiold = *nhi; // save last event
    hs->lasteventtm = nhi->tstamp;
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
    SDK_HMON_TRACE_DEBUG("%s", logfmt(fmt, ap));
}

static void
loginfo(const char *fmt, va_list ap)
{
    SDK_HMON_TRACE_INFO("%s", logfmt(fmt, ap));
}

static void
logwarn(const char *fmt, va_list ap)
{
    SDK_HMON_TRACE_WARN("%s", logfmt(fmt, ap));
}

static void
logerror(const char *fmt, va_list ap)
{
    SDK_HMON_TRACE_ERR("%s", logfmt(fmt, ap));
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
        pciesys_set_logger(&pciesys_logger, PCIESYS_PRI_SYSMON);
        pciehdev_open(NULL);
        portmap_init_from_catalog(g_sysmon_cfg.catalog);
        inited = 1;
    }
    portmap_foreach_port(pciehealth_port, NULL);
}
