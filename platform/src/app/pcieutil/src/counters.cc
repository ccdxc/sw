/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <cinttypes>

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pcieport/include/portmap.h"

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"
#include "cap_wa_c_hdr.h"

#include "cmd.h"
#include "utils.hpp"
#include "counter_defs.h"

// field width for name
#define NAMEWIDTH       50

#define F_NONE          0x0
#define F_SHOWALL       0x1 // show all counters, even if val=0
#define F_CLEAR         0x2 // clear all counters

static uint32_t global_flags = F_NONE;

#ifndef __aarch64__
#define pal_reg_rd32(a) (a)
#define pal_reg_rd64(a) (a)
#define pal_reg_rd32w(a, d, n) memset(d, 0xff, n * 4)
#endif

static char *
strlower(char *str)
{
    for (char *s = str; *s; s++) *s = tolower(*s);
    return str;
}

static void
show_value(const char *name, const uint64_t val)
{
    if (val || (global_flags & F_SHOWALL)) {
        printf("%-*s %" PRId64 "\n", NAMEWIDTH, name, val);
    }
}

static void
show_counter(const int port,
             const uint64_t val, const char *grp, const char *ctr)
{
    char *lgrp = strlower(strdup(grp));
    char *lctr = strlower(strdup(ctr));
    char name[80];

    if (port == -1) {
        snprintf(name, sizeof(name), "%s_%s", lgrp, lctr);
    } else {
        snprintf(name, sizeof(name), "port%d:%s_%s", port, lgrp, lctr);
    }
    show_value(name, val);
    free(lgrp);
    free(lctr);
}

static void
show_counterf(const int port,
              const uint64_t fullval,
              const char *grp, const char *ctr, const char *fld,
              const uint8_t bits, const uint8_t bitc)
{
    const uint64_t val = (fullval >> bits) & ((1ULL << bitc) - 1);
    char *lgrp = strlower(strdup(grp));
    char *lctr = strlower(strdup(ctr));
    char name[80];

    if (port == -1) {
        snprintf(name, sizeof(name), "%s_%s.%s", lgrp, lctr, fld);
    } else {
        snprintf(name, sizeof(name), "port%d:%s_%s.%s", port, lgrp, lctr, fld);
    }
    show_value(name, val);
    free(lgrp);
    free(lctr);
}

static uint32_t
read_or_clear_32(const uint64_t addr)
{
    if (global_flags & F_CLEAR) {
        pal_reg_wr32(addr, 0);
        return 0;
    } else {
        return pal_reg_rd32(addr);
    }
}

static uint64_t
read_or_clear_64(const uint64_t addr)
{
    if (global_flags & F_CLEAR) {
        pal_reg_wr64(addr, 0);
        return 0;
    } else {
        return pal_reg_rd64(addr);
    }
}

static void
show_counter_32(const int port,
                const uint64_t addr,
                const char *grp, const char *ctr, const char *fld,
                const uint8_t bits, const uint8_t bitc)
{
    const uint32_t v = read_or_clear_32(addr);
    show_counter(port, v, grp, ctr);
}

static void
show_counter_64(const int port,
                const uint64_t addr,
                const char *grp, const char *ctr, const char *fld,
                const uint8_t bits, const uint8_t bitc)
{
    const uint64_t v = read_or_clear_64(addr);
    show_counter(port, v, grp, ctr);
}

static void
show_counter_32f(const int port,
                 const uint64_t addr,
                 const char *grp, const char *ctr, const char *fld,
                 const uint8_t bits, const uint8_t bitc)
{
    const uint32_t v = read_or_clear_32(addr);
    show_counterf(port, v, grp, ctr, fld, bits, bitc);
}

static void
show_counter_64f(const int port,
                 const uint64_t addr,
                 const char *grp, const char *ctr, const char *fld,
                 const uint8_t bits, const uint8_t bitc)
{
    const uint64_t v = read_or_clear_64(addr);
    show_counterf(port, v, grp, ctr, fld, bits, bitc);
}

typedef union {
    struct {
        u_int32_t pmr_force:8;
        u_int32_t pmt_force:8;
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
} pxb_sat_tgt_ind_reason_t;

static void
show_pxb_sat_tgt_ind_reason(void)
{
    pxb_sat_tgt_ind_reason_t v;

    pal_reg_rd32w(PXB_(SAT_TGT_IND_REASON), v.w, 4);

#define SHOW_VALUE(F) \
    show_value("pxb_sat_tgt_ind_reason." #F, v.F)

    SHOW_VALUE(pmr_force);
    SHOW_VALUE(pmt_force);
    SHOW_VALUE(msg);
    SHOW_VALUE(atomic);
    SHOW_VALUE(poisoned);
    SHOW_VALUE(unsupp);
    SHOW_VALUE(pmv);
    SHOW_VALUE(db_pmv);
    SHOW_VALUE(pmt_miss);
    SHOW_VALUE(rc_vfid_miss);
    SHOW_VALUE(pmr_prt_miss);
    SHOW_VALUE(prt_oor);
    SHOW_VALUE(bdf_wcard_oor);
    SHOW_VALUE(vfid_oor);
}

static void
clear_pxb_sat_tgt_ind_reason(void)
{
    pxb_sat_tgt_ind_reason_t v;

    memset(&v, 0, sizeof(v));
    pal_reg_wr32w(PXB_(SAT_TGT_IND_REASON), v.w, 4);
}

static void
show_counter_by_name(const int port,
                     const uint64_t addr,
                     const char *grp, const char *ctr, const char *fld,
                     const uint8_t bits, const uint8_t bitc)
{
    char name[80];

    if (port == -1) {
        snprintf(name, sizeof(name), "%s_%s", grp, ctr);
    } else {
        snprintf(name, sizeof(name), "port%d:%s_%s", port, grp, ctr);
    }
    if (strcmp(name, "PXB_SAT_TGT_IND_REASON") == 0) {
        if (global_flags & F_CLEAR) {
            clear_pxb_sat_tgt_ind_reason();
        } else {
            show_pxb_sat_tgt_ind_reason();
        }
    } else {
        fprintf(stderr, "unknown counter name: %s\n", name);
    }
}

static void
show_pxb_sat_itr(void)
{
#define GROUP           "PXB_SAT_ITR"
#define ADDR(CTR)       PXB_(SAT_ITR_##CTR)

#define PXB_SAT_ITR_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(-1, ADDR(CTR), GROUP, #CTR, #fld, bits, bitc);

    PXB_SAT_ITR_GENERATOR(PXB_SAT_ITR_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_pxb_sat_tgt(void)
{
#define GROUP           "PXB_SAT_TGT"
#define ADDR(CTR)       PXB_(SAT_TGT_##CTR)

#define PXB_SAT_TGT_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(-1, ADDR(CTR), GROUP, #CTR, #fld, bits, bitc);

    PXB_SAT_TGT_GENERATOR(PXB_SAT_TGT_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_pxb_cnt_axi(void)
{
#define GROUP           "PXB_CNT_AXI"
#define ADDR(CTR)       PXB_(CNT_AXI_##CTR)

#define PXB_CNT_AXI_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(-1, ADDR(CTR), GROUP, #CTR, #fld, bits, bitc);

    PXB_CNT_AXI_GENERATOR(PXB_CNT_AXI_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_pxb_cnt_itr(void)
{
#define GROUP           "PXB_CNT_ITR"
#define ADDR(CTR)       PXB_(CNT_ITR_##CTR)

#define PXB_CNT_ITR_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(-1, ADDR(CTR), GROUP, #CTR, #fld, bits, bitc);

    PXB_CNT_ITR_GENERATOR(PXB_CNT_ITR_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_pxb_cnt_tgt(void)
{
#define GROUP           "PXB_CNT_TGT"
#define ADDR(CTR)       PXB_(CNT_TGT_##CTR)

#define PXB_CNT_TGT_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(-1, ADDR(CTR), GROUP, #CTR, #fld, bits, bitc);

    PXB_CNT_TGT_GENERATOR(PXB_CNT_TGT_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_pp_sat_pp_pipe(void)
{
#define GROUP           "PP_SAT_PP_PIPE"
#define ADDR(CTR, LN)   PP_(SAT_PP_PIPE_## CTR ##_## LN)

#define SHOW_COUNTER_PIPE(LN, ty, CTR, fld, bits, bitc) \
    show_counter_##ty(-1, ADDR(CTR,LN), GROUP, #CTR "_" #LN,  #fld, bits, bitc)

#define PP_SAT_PP_PIPE_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    SHOW_COUNTER_PIPE(0,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(1,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(2,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(3,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(4,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(5,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(6,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(7,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(8,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(9,  ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(10, ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(11, ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(12, ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(13, ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(14, ty, CTR, fld, bits, bitc); \
    SHOW_COUNTER_PIPE(15, ty, CTR, fld, bits, bitc);

    PP_SAT_PP_PIPE_GENERATOR(PP_SAT_PP_PIPE_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_pp_port_c_cnt_c(const int port)
{
#define GROUP           "PP_PORT_C_CNT_C"
#define ADDR(CTR, port) PXC_(CNT_C_##CTR, port)

#define PP_PORT_C_CNT_C_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(port, ADDR(CTR, port), GROUP, #CTR, #fld, bits, bitc);

    PP_PORT_C_CNT_C_GENERATOR(PP_PORT_C_CNT_C_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_pp_port_c_sat_c_port_cnt(const int port)
{
#define GROUP           "PP_PORT_C_SAT_C_PORT_CNT"
#define ADDR(CTR, port) PXC_(SAT_C_PORT_CNT_##CTR, port)

#define PP_PORT_C_SAT_C_PORT_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(port, ADDR(CTR, port), GROUP, #CTR, #fld, bits, bitc);

    PP_PORT_C_SAT_C_PORT_CNT_GENERATOR(PP_PORT_C_SAT_C_PORT_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_pp_port_p_sat_p_port_cnt(const int port)
{
#define GROUP           "PP_PORT_P_SAT_P_PORT_CNT"
#define ADDR(CTR, port) PXP_(SAT_P_PORT_CNT_##CTR, port)

#define PP_PORT_P_SAT_P_PORT_CNT_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(port, ADDR(CTR, port), GROUP, #CTR, #fld, bits, bitc);

    PP_PORT_P_SAT_P_PORT_CNT_GENERATOR(PP_PORT_P_SAT_P_PORT_CNT_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_db_wa_sat_wa(void)
{
#define GROUP           "DB_WA_SAT_WA"
#define ADDR(CTR)       (CAP_ADDR_BASE_DB_WA_OFFSET + \
                         CAP_WA_CSR_SAT_WA_ ##CTR## _BYTE_ADDRESS)

#define DB_WA_SAT_WA_SHOW_GEN(ty, CTR, fld, bits, bitc) \
    show_counter_##ty(-1, ADDR(CTR), GROUP, #CTR, #fld, bits, bitc);

    DB_WA_SAT_WA_GENERATOR(DB_WA_SAT_WA_SHOW_GEN)

#undef GROUP
#undef ADDR
}

static void
show_per_port_stats(const int port, void *arg)
{
    show_pp_port_c_cnt_c(port);
    show_pp_port_c_sat_c_port_cnt(port);
    if (pcieport_is_accessible(port)) {
        show_pp_port_p_sat_p_port_cnt(port);
    } else {
        printf("port%d:pp_port_p_sat_p_port_cnt not accessible\n", port);
    }
}

static void
counters(int argc, char *argv[])
{
    int opt;

    global_flags = F_NONE;
    optind = 0;
    while ((opt = getopt(argc, argv, "ac")) != -1) {
        switch (opt) {
        case 'a':
            global_flags |= F_SHOWALL;
            break;
        case 'c':
            global_flags |= F_CLEAR;
            break;
        default:
            return;
        }
    }

    show_pxb_sat_itr();
    show_pxb_sat_tgt();

    show_pxb_cnt_axi();
    show_pxb_cnt_itr();
    show_pxb_cnt_tgt();

    show_pp_sat_pp_pipe();

    portmap_init_from_catalog();
    portmap_foreach_port(show_per_port_stats, NULL);

    show_db_wa_sat_wa();
}
CMDFUNC(counters,
"show pcie counters",
"counters [-ac]\n"
"    -a         show all counters, even if 0\n"
"    -c         clear all counters to 0\n");
