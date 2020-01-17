/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/time.h>

#include "platform/pal/include/pal.h"
#include "platform/misc/include/misc.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "portcfg.h"
#include "pcieport.h"
#include "pcieport_impl.h"
#include "cap_sw_glue.h"
#include "cap_pcie_api.h"

int fsm_verbose = 1;

const char *
pcieport_stname(const pcieportst_t st)
{
    static const char *stnames[PCIEPORTST_MAX] = {
#define PCIEPORTST_NAME(ST) \
        [PCIEPORTST_##ST] = #ST
        PCIEPORTST_NAME(OFF),
        PCIEPORTST_NAME(DOWN),
        PCIEPORTST_NAME(MACUP),
        PCIEPORTST_NAME(LINKUP),
        PCIEPORTST_NAME(UP),
        PCIEPORTST_NAME(FAULT),
    };
    if (st >= PCIEPORTST_MAX) return "UNKNOWN_ST";
    return stnames[st];
}

const char *
pcieport_evname(const pcieportev_t ev)
{
    static const char *evnames[PCIEPORTEV_MAX] = {
#define PCIEPORTEV_NAME(EV) \
        [PCIEPORTEV_##EV] = #EV
        PCIEPORTEV_NAME(MACDN),
        PCIEPORTEV_NAME(MACUP),
        PCIEPORTEV_NAME(LINKDN),
        PCIEPORTEV_NAME(LINKUP),
        PCIEPORTEV_NAME(BUSCHG),
        PCIEPORTEV_NAME(POWERDN),
    };
    if (ev >= PCIEPORTEV_MAX) return "UNKNOWN_EV";
    return evnames[ev];
}

/*
 * Link has a fault.  Record fault reason and put the link
 * in FAULT state.  FAULT state can be cleared by mac down.
 */
void
pcieport_fault(pcieport_t *p, const char *fmt, ...)
{
    p->stats.faults++;
    if (p->state != PCIEPORTST_FAULT) {
        const size_t reasonsz = sizeof(p->fault_reason);
        va_list ap;

        va_start(ap, fmt);
        vsnprintf(p->fault_reason, reasonsz, fmt, ap);
        va_end(ap);

        p->state = PCIEPORTST_FAULT;
        pcieport_event_fault(p);
        pciesys_logdebug("port%d: fault %s\n", p->port, p->fault_reason);
    }
}

/*
 * Clear any fault reason, save the last fault reason for debug.
 */
static void
pcieport_clear_fault(pcieport_t *p)
{
    if (p->fault_reason[0] != '\0') {
        const size_t bufsz = sizeof(p->last_fault_reason);
        strncpy0(p->last_fault_reason, p->fault_reason, bufsz);
        p->fault_reason[0] = '\0';
    }
}

static int
pcieport_drain(pcieport_t *p)
{
    if (pcieport_tgt_marker_rx_wait(p) < 0) {
        pciesys_logerror("port%d: port tgt_marker_rx failed\n", p->port);
        return -1;
    }
    if (pcieport_tgt_axi_pending_wait(p) < 0) {
        pciesys_logerror("port%d: port tgt_axi_pending failed\n", p->port);
        return -1;
    }
    return 0;
}

static void
pcieport_clear_early_sat_ind_reason(pcieport_t *p)
{
    union {
        struct {
            u_int32_t pmr_force:8;
            u_int32_t prt_force:8;
            u_int32_t msg:8;
            u_int32_t atomic:8;
            u_int32_t poisoned:8;
            u_int32_t unsupp:8;
            u_int32_t pmv:8;
            u_int32_t db_pmv:8;
            u_int32_t pmt_miss:8;
            u_int32_t rc_vfid_miss:8;
            u_int32_t pmr_prt_miss:8;
            u_int32_t prt_oor:8;
            u_int32_t bdf_wcard_oor:8;
            u_int32_t vfid_oor:8;
        } __attribute__((packed));
        u_int32_t w[4];
    } v;
    const u_int64_t sat_tgt_ind_reason =
        (CAP_ADDR_BASE_PXB_PXB_OFFSET +
         CAP_PXB_CSR_SAT_TGT_IND_REASON_BYTE_ADDRESS);

    pal_reg_rd32w(sat_tgt_ind_reason, v.w, 4);
    /*
     * We get many pmt_miss events during BIOS bus scan
     * as the BIOS probes for devices that don't exist.
     * This counter ends up saturated at startup always.
     * We'll clear the counter here so it can count
     * the pmt_miss events we get *after* the BIOS scan.
     *
     * (We could add a catchall entry in the PMT to catch
     * these and return UR?)
     */
    v.pmt_miss = 0;
    pal_reg_wr32w(sat_tgt_ind_reason, v.w, 4);
}

static void
pcieport_clear_early_link_counts(pcieport_t *p)
{
    pcieport_set_ltssm_st_cnt(p, 0);
    pcieport_clear_early_sat_ind_reason(p);
}

static void
pcieport_update_linkinfo(pcieport_t *p)
{
    portcfg_read_genwidth(p->port, &p->cur_gen, &p->cur_width);
    portcfg_read_bus(p->port, &p->pribus, &p->secbus, &p->subbus);
    p->cur_reversed = pcieport_get_mac_lanes_reversed(p);
    pciesys_logdebug("port%d: gen%dx%d%s pri %02x sec %02x sub %02x\n",
                     p->port, p->cur_gen, p->cur_width,
                     p->cur_reversed ? "r" : "",
                     p->pribus, p->secbus, p->subbus);
}

static void
pcieport_buschg(pcieport_t *p)
{
    const u_int8_t secbus_prev = p->secbus;

    pcieport_clear_early_link_counts(p);
    pcieport_update_linkinfo(p);
    /*
     * Ignore changes to secbus=0 which happens at startup
     * and doesn't give us much useful info.  It is transitory
     * as the BIOS settles on the final bus allocation.
     */
    if (p->secbus && p->secbus != secbus_prev) {
        pciesys_logdebug("port%d: secbus 0x%02x\n", p->port, p->secbus);
        pcieport_event_buschg(p);
    }
}

static void
pcieport_hostup(pcieport_t *p)
{
    pcieport_event_hostup(p, ++p->stats.hostup);
}

static void
pcieport_hostdn(pcieport_t *p)
{
#if 0
    /* XXX not yet -- no one to re-enable crs yet */
    /* hostdn triggers automatic crs=1 */
    p->crs = 1;
    pcieport_set_crs(p, p->crs);
#endif
    p->secbus = 0;
    pcieport_event_hostdn(p, p->stats.hostup);
}

static void
pcieport_linkup(pcieport_t *p)
{
    if (pcieport_gate_open(p) < 0) {
        pcieport_fault(p, "gate_open failed");
        return;
    }
    pcieport_set_crs(p, p->crs);
    pcieport_config_linkup(p);
    pcieport_update_linkinfo(p);
    pcieport_event_linkup(p, ++p->stats.linkup);

    if ((p->req_gen   && p->cur_gen   < p->req_gen) ||
        (p->req_width && p->cur_width < p->req_width)) {
        pciesys_logwarn("port%d: req gen%dx%d got gen%dx%d, resetting\n",
                        p->port,
                        p->req_gen, p->req_width,
                        p->cur_gen, p->cur_width);
        cap_pcie_serdes_reset(0, 0);
    }
}

static void
pcieport_linkdn(pcieport_t *p)
{
    pcieport_event_linkdn(p, p->stats.linkup);
}

static void
pcieport_macup(pcieport_t *p)
{
}

static void
pcieport_macdn(pcieport_t *p)
{
    pcieport_clear_fault(p);
    if (pcieport_drain(p) < 0 && 0 /* XXX for HAPS2 */) {
        pcieport_fault(p, "drain failed");
        return;
    }
}

static void
pcieport_powerdn(pcieport_t *p)
{
    pcieport_clear_fault(p);
    pcieport_config_powerdown(p);
}

static void
fsm_nop(pcieport_t *p)
{
    /* nothing to do here, move along... */
}

static void
fsm_inv(pcieport_t *p)
{
    pcieport_fault(p, "fsm_inv: %s + %s",
                   pcieport_stname(p->state),
                   pcieport_evname(p->event));
}

static void
fsm_macup(pcieport_t *p)
{
    p->state = PCIEPORTST_MACUP;
    pcieport_macup(p);
}

static void
fsm_macdn(pcieport_t *p)
{
    p->state = PCIEPORTST_DOWN;
    pcieport_macdn(p);
}

static void
fsm_lup_macdn(pcieport_t *p)
{
    p->state = PCIEPORTST_DOWN;
    pcieport_linkdn(p);
    pcieport_macdn(p);
}

static void
fsm_up_macdn(pcieport_t *p)
{
    p->state = PCIEPORTST_DOWN;
    pcieport_hostdn(p);
    pcieport_linkdn(p);
    pcieport_macdn(p);
}

static void
fsm_linkup(pcieport_t *p)
{
    p->state = PCIEPORTST_LINKUP;
    pcieport_linkup(p);
}

static void
fsm_linkdn(pcieport_t *p)
{
    p->state = PCIEPORTST_MACUP;
    pcieport_linkdn(p);
}

static void
fsm_up_linkdn(pcieport_t *p)
{
    p->state = PCIEPORTST_MACUP;
    pcieport_hostdn(p);
    pcieport_linkdn(p);
}

static void
fsm_up(pcieport_t *p)
{
    p->state = PCIEPORTST_UP;
    pcieport_buschg(p);
    pcieport_hostup(p);
}

static void
fsm_buschg(pcieport_t *p)
{
    pcieport_buschg(p);
}

static void
fsm_powerdn(pcieport_t *p)
{
    p->state = PCIEPORTST_OFF;
    pcieport_powerdn(p);
}

static void
fsm_macup_powerdn(pcieport_t *p)
{
    p->state = PCIEPORTST_OFF;
    pcieport_macdn(p);
    pcieport_powerdn(p);
}

static void
fsm_linkup_powerdn(pcieport_t *p)
{
    p->state = PCIEPORTST_OFF;
    pcieport_linkdn(p);
    pcieport_macdn(p);
    pcieport_powerdn(p);
}

static void
fsm_up_powerdn(pcieport_t *p)
{
    p->state = PCIEPORTST_OFF;
    pcieport_hostdn(p);
    pcieport_linkdn(p);
    pcieport_macdn(p);
    pcieport_powerdn(p);
}

#define NOP fsm_nop
#define INV fsm_inv
#define MCU fsm_macup
#define MCD fsm_macdn
#define LMD fsm_lup_macdn
#define UMD fsm_up_macdn
#define LKU fsm_linkup
#define LKD fsm_linkdn
#define ULD fsm_up_linkdn
#define UP_ fsm_up
#define BUS fsm_buschg
#define OFF fsm_powerdn
#define MUO fsm_macup_powerdn
#define LUO fsm_linkup_powerdn
#define UPO fsm_up_powerdn

typedef void (*fsm_func_t)(pcieport_t *p);
static fsm_func_t
fsm_table[PCIEPORTST_MAX][PCIEPORTEV_MAX] = {
    /*
     * [state]            + event:
     *                      MACDN
     *                      |    MACUP
     *                      |    |    LINKDN
     *                      |    |    |    LINKUP
     *                      |    |    |    |    BUSCHG
     *                      |    |    |    |    |    OFF
     *                      |    |    |    |    |    |   */
    [PCIEPORTST_OFF]    = { NOP, MCU, NOP, INV, NOP, NOP },
    [PCIEPORTST_DOWN]   = { NOP, MCU, NOP, INV, NOP, OFF },
    [PCIEPORTST_MACUP]  = { MCD, INV, NOP, LKU, NOP, MUO },
    [PCIEPORTST_LINKUP] = { LMD, INV, LKD, NOP, UP_, LUO },
    [PCIEPORTST_UP]     = { UMD, INV, ULD, INV, BUS, UPO },
    [PCIEPORTST_FAULT]  = { MCD, NOP, NOP, NOP, NOP, OFF },

    /*
     * NOTES:
     *
     * BUSCHG event     BUSCHG can arrive anytime, when bus resets to 0.
     *                  So NOP for BUSCHG before LINKUP.
     * DOWN + LINKDN    Could happen if link goes up/down quickly,
     *                  before we saw LINKUP, intr() will always send LINKDN.
     * MACUP + LINKDN   Could happen if link goes up/down quickly.
     *                  before we saw LINKUP, intr() will always send LINKDN.
     * LINKUP + LINKUP  Could happen if a link has gone up and back down and
     *                  started coming back up before we started.
     *                  Then we see LINKDN, MACDN, MACUP, LINKUP pending,
     *                  *then* a LINKUP arrives.
     * LINKUP + MACDN   LINK and MAC go down together but we might find
     *                  MACDN *then* LINKDN arrives.
     * UP + MACDN       LINK and MAC go down together but we might find
     *                  MACDN *then* LINKDN arrives.
     * DOWN + LINKDN    LINK and MAC go down together but we found MACDN first,
     *                  went to DOWN, *then* LINKDN arrives while DOWN.
     */
};

void
pcieport_fsm(pcieport_t *p, pcieportev_t ev)
{
    pcieportst_t st;

    st = p->state;
    p->event = ev;

    fsm_table[st][ev](p);

    if (fsm_verbose) {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        pciesys_loginfo("[%ld.%.3ld] fsm port%d: %s + %s => %s\n",
                        tv.tv_sec, tv.tv_usec / 1000,
                        p->port,
                        pcieport_stname(st),
                        pcieport_evname(ev),
                        pcieport_stname(p->state));
    }
}

void
pcieport_fsm_init(pcieport_t *p, pcieportst_t st)
{
    p->state = st;

    switch (p->state) {
    case PCIEPORTST_MACUP:
        pcieport_macup(p);
        break;
    case PCIEPORTST_LINKUP:
        pcieport_macup(p);
        pcieport_linkup(p);
        break;
    case PCIEPORTST_UP:
        pcieport_macup(p);
        pcieport_linkup(p);
        pcieport_buschg(p);
        pcieport_hostup(p);
        break;
    default:
        break;
    }

    if (fsm_verbose) {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        pciesys_loginfo("[%ld.%.3ld] fsm port%d: init %s\n",
                        tv.tv_sec, tv.tv_usec / 1000,
                        p->port, pcieport_stname(p->state));
    }
}

void
pcieport_fsm_dbg(int argc, char *argv[])
{
    int opt;

    optind = 0;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
        case 'v':
            fsm_verbose = !fsm_verbose;
            break;
        default:
            return;
        }
    }
}
