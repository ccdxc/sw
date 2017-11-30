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

#include "pal.h"
#include "pcieport.h"
#include "pcieport_impl.h"

static void
fsm_nop(pcieport_t *p)
{
}

static void
fsm_inv(pcieport_t *p)
{
}

#define NOP fsm_nop
#define INV fsm_inv
#define MUP fsm_macup
#define MDN fsm_macdn
#define LUP fsm_linkup
#define LDN fsm_linkdn
#define BUS fsm_buschg
#define CFG fsm_config

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
     *                      |    |    |    |    |    CONFIG
     *                      |    |    |    |    |    |    */
    [PCIEPORTST_DOWN]   = { NOP, INV, INV, INV, INV, INV },
    [PCIEPORTST_MACUP]  = { INV, INV, INV, INV, INV, INV },
    [PCIEPORTST_LINKUP] = { INV, INV, INV, INV, INV, INV },
    [PCIEPORTST_UP]     = { INV, INV, INV, INV, INV, INV },
    [PCIEPORTST_FAULT]  = { INV, INV, INV, INV, INV, INV },
};

void
pcieport_fsm(pcieport_t *p, pcieportev_t ev)
{
    pcieportst_t st;

    st = p->state;
    p->event = ev;

    fsm_table[st][ev](p);
}
