/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
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
        PCIEPORTEV_NAME(POWERON),
        PCIEPORTEV_NAME(MACDN),
        PCIEPORTEV_NAME(MACUP),
        PCIEPORTEV_NAME(LINKDN),
        PCIEPORTEV_NAME(LINKUP),
        PCIEPORTEV_NAME(BUSCHG),
    };
    if (ev >= PCIEPORTEV_MAX) return "UNKNOWN_EV";
    return evnames[ev];
}

static void
pcieport_poweron(pcieport_t *p)
{
    pcieport_config(p);
}

static void
fsm_nop(pcieport_t *p)
{
}

static void
fsm_inv(pcieport_t *p)
{
}

static void
fsm_poweron(pcieport_t *p)
{
    pcieport_poweron(p);
    p->state = PCIEPORTST_DOWN;
}

/*
 * Power on after macup.  Power on event raced with macup
 * and macup came first.  Apply power on config now, but
 * leave state where it is.
 */
static void
fsm_macup_poweron(pcieport_t *p)
{
    pcieport_poweron(p);
}

static void
fsm_macup(pcieport_t *p)
{
    pcieport_gate_open(p);
    p->state = PCIEPORTST_MACUP;
}

static void
fsm_macdn(pcieport_t *p)
{
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
fsm_up(pcieport_t *p)
{
    p->linkup++;
    p->state = PCIEPORTST_UP;
}

static void
fsm_buschg(pcieport_t *p)
{
}

#define NOP fsm_nop
#define INV fsm_inv
#define PON fsm_poweron
#define MON fsm_macup_poweron
#define MUP fsm_macup
#define MDN fsm_macdn
#define LUP fsm_linkup
#define LDN fsm_linkdn
#define UP_ fsm_up
#define BUS fsm_buschg

typedef void (*fsm_func_t)(pcieport_t *p);
static fsm_func_t
fsm_table[PCIEPORTST_MAX][PCIEPORTEV_MAX] = {
    /*
     * [state]            + event:
     *                      POWERON
     *                      |    MACDN
     *                      |    |    MACUP
     *                      |    |    |    LINKDN
     *                      |    |    |    |    LINKUP
     *                      |    |    |    |    |    BUSCHG
     *                      |    |    |    |    |    |   */
    [PCIEPORTST_OFF]    = { PON, NOP, MUP, NOP, INV, INV },
    [PCIEPORTST_DOWN]   = { PON, NOP, MUP, INV, INV, INV },
    [PCIEPORTST_MACUP]  = { MON, MDN, INV, INV, LUP, INV },
    [PCIEPORTST_LINKUP] = { INV, INV, INV, LDN, INV, UP_ },
    [PCIEPORTST_UP]     = { INV, INV, INV, LDN, INV, BUS },
    [PCIEPORTST_FAULT]  = { PON, MDN, NOP, NOP, NOP, NOP },
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
