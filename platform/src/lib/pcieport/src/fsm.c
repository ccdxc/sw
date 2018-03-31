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

#include "pal.h"
#include "pciehsys.h"
#include "pcieport.h"
#include "pcieport_impl.h"

int fsm_verbose = 1;

static const char *
stname(const pcieportst_t st)
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

static const char *
evname(const pcieportev_t ev)
{
    static const char *evnames[PCIEPORTEV_MAX] = {
#define PCIEPORTEV_NAME(EV) \
        [PCIEPORTEV_##EV] = #EV
        PCIEPORTEV_NAME(MACDN),
        PCIEPORTEV_NAME(MACUP),
        PCIEPORTEV_NAME(LINKDN),
        PCIEPORTEV_NAME(LINKUP),
        PCIEPORTEV_NAME(BUSCHG),
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
    p->faults++;
    if (p->state != PCIEPORTST_FAULT) {
        const size_t reasonsz = sizeof(p->fault_reason);
        va_list ap;

        va_start(ap, fmt);
        vsnprintf(p->fault_reason, reasonsz, fmt, ap);
        va_end(ap);

        p->state = PCIEPORTST_FAULT;
        pciehsys_error("port %d fault: %s\n", p->port, p->fault_reason);
    }
}

/*
 * Clear any fault reason, save the last fault reason for debug.
 */
static void
pcieport_clear_fault(pcieport_t *p)
{
    if (p->fault_reason) {
        const size_t bufsz = sizeof(p->last_fault_reason);
        strncpy(p->last_fault_reason, p->fault_reason, bufsz);
        p->fault_reason[0] = '\0';
    }
}

static void
pcieport_linkup(pcieport_t *p)
{
    p->linkup++;
}

static void
pcieport_linkdn(pcieport_t *p)
{
    /* XXX need link down event */
}

static int
pcieport_drain(pcieport_t *p)
{
    if (pcieport_tgt_marker_rx_wait(p) < 0) {
        pcieport_fault(p, "port tgt_marker_rx failed");
        return -1;
    }
    if (pcieport_tgt_axi_pending_wait(p) < 0) {
        pcieport_fault(p, "port tgt_axi_pending failed");
        return -1;
    }
    return 0;
}

static int
pcieport_macup(pcieport_t *p)
{
    if (pcieport_gate_open(p) < 0) {
        pcieport_fault(p, "portgate open failed");
        return -1;
    }
    return 0;
}

static int
pcieport_macdn(pcieport_t *p)
{
    pcieport_clear_fault(p);
    if (pcieport_drain(p) < 0) {
        return -1;
    }
    return 0;
}

static void
fsm_nop(pcieport_t *p)
{
}

static void
fsm_inv(pcieport_t *p)
{
    pcieport_fault(p, "fsm_inv: %s + %s", stname(p->state), evname(p->event));
}

static void
fsm_macup(pcieport_t *p)
{
    if (pcieport_macup(p) < 0) {
        return;
    }
    p->state = PCIEPORTST_MACUP;
}

static void
fsm_macdn(pcieport_t *p)
{
    if (pcieport_macdn(p) < 0) {
        return;
    }
    p->state = PCIEPORTST_DOWN;
}

static void
fsm_linkup(pcieport_t *p)
{
    pcieport_set_crs(p, p->crs);
    p->state = PCIEPORTST_LINKUP;
}

static void
fsm_linkdn(pcieport_t *p)
{
    p->state = PCIEPORTST_MACUP;
}

static void
fsm_up_linkdn(pcieport_t *p)
{
    pcieport_linkdn(p);
    p->state = PCIEPORTST_MACUP;
}

static void
fsm_up(pcieport_t *p)
{
    pcieport_linkup(p);
    p->state = PCIEPORTST_UP;
}

static void
fsm_buschg(pcieport_t *p)
{
}

#define NOP fsm_nop
#define INV fsm_inv
#define MCU fsm_macup
#define MCD fsm_macdn
#define LKU fsm_linkup
#define LKD fsm_linkdn
#define ULD fsm_up_linkdn
#define UP_ fsm_up
#define BUS fsm_buschg

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
     *                      |    |    |    |    |   */
    [PCIEPORTST_OFF]    = { NOP, MCU, NOP, INV, NOP },
    [PCIEPORTST_DOWN]   = { NOP, MCU, NOP, INV, NOP },
    [PCIEPORTST_MACUP]  = { MCD, INV, NOP, LKU, NOP },
    [PCIEPORTST_LINKUP] = { INV, INV, LKD, INV, UP_ },
    [PCIEPORTST_UP]     = { INV, INV, ULD, INV, BUS },
    [PCIEPORTST_FAULT]  = { MCD, NOP, NOP, NOP, NOP },

    /*
     * NOTES:
     *
     * BUSCHG event     BUSCHG can arrive anytime, when bus resets to 0.
     *                  So NOP for BUSCHG before LINKUP.
     * DOWN + LINKDN    Could happen if link goes up/down quickly,
     *                  before we saw LINKUP, intr() will always send LINKDN.
     * MACUP + LINKDN   Could happen if link goes up/down quickly.
     *                  before we saw LINKUP, intr() will always send LINKDN.
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
        pciehsys_log("[%ld.%.3ld] %d: %s + %s => %s\n",
                     tv.tv_sec, tv.tv_usec / 1000,
                     p->port, stname(st), evname(ev), stname(p->state));
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
