/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>

#include "cap_top_csr_defines.h"
#include "cap_wa_c_hdr.h"

#include "nic/sdk/platform/pal/include/pal.h"
#include "cmd.h"

typedef union {
    struct {
        uint64_t vld:1;
        uint64_t qstateaddr_or_qid:29;
        uint64_t pid_or_lif_type:16;
        uint64_t cnt:11;
        uint64_t doorbell_merged:1;
        uint64_t addr_conflict:1;
        uint64_t tot_ring_err:1;
        uint64_t host_ring_err:1;
        uint64_t pid_fail:1;
        uint64_t qid_ovflow:1;
    } __attribute__((packed));
    uint32_t w[2];
    uint64_t w64;
} db_err_activity_log_entry_t;

#define DBERR_BASE \
    (CAP_ADDR_BASE_DB_WA_OFFSET + \
     CAP_WA_CSR_DHS_DOORBELL_ERR_ACTIVITY_LOG_ENTRY_BYTE_ADDRESS)
#define DBERR_STRIDE 0x8
#define DBERR_COUNT \
    CAP_WA_CSR_DHS_DOORBELL_ERR_ACTIVITY_LOG_ENTRY_ARRAY_COUNT

typedef union {
    struct {
        uint64_t vld:1;
        uint64_t qstate_base:22;
        uint64_t length0:5;
        uint64_t size0:3;
        uint64_t length1:5;
        uint64_t size1:3;
        uint64_t length2:5;
        uint64_t size2:3;
        uint64_t length3:5;
        uint64_t size3:3;
        uint64_t length4:5;
        uint64_t size4:3;
        uint64_t length5:5;
        uint64_t size5:3;
        uint64_t length6:5;
        uint64_t size6:3;
        uint64_t length7:5;
        uint64_t size7:3;
        uint64_t sched_hint_en:1;
        uint64_t sched_hint_cos:4;
        uint64_t spare:4;
        uint64_t ecc:8;
    } __attribute__((packed));
    uint32_t w[4];
} lif_qstate_map_entry_t;

#define LIF_QSTATE_MAP_BASE \
    (CAP_ADDR_BASE_DB_WA_OFFSET + \
     CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_BYTE_ADDRESS)
#define LIF_QSTATE_MAP_STRIDE   0x10
#define LIF_QSTATE_MAP_COUNT \
    CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ARRAY_COUNT

static uint64_t
lif_qstate_map_addr(const int lif)
{
    return LIF_QSTATE_MAP_BASE + lif * LIF_QSTATE_MAP_STRIDE;
}

static void
lif_qstate_map_read(const int lif, lif_qstate_map_entry_t *qstmap)
{
    pal_reg_rd32w(lif_qstate_map_addr(lif), qstmap->w, 4);
}

static unsigned int
qstate_raw_length(const lif_qstate_map_entry_t *qstmap, const int type)
{
    unsigned int length = 0;

    switch (type) {
    case 0: length = qstmap->length0; break;
    case 1: length = qstmap->length1; break;
    case 2: length = qstmap->length2; break;
    case 3: length = qstmap->length3; break;
    case 4: length = qstmap->length4; break;
    case 5: length = qstmap->length5; break;
    case 6: length = qstmap->length6; break;
    case 7: length = qstmap->length7; break;
    default: length = 0; break;
    }
    return length;
}

static unsigned int
qstate_raw_size(const lif_qstate_map_entry_t *qstmap, const int type)
{
    unsigned int size = 0;

    switch (type) {
    case 0: size = qstmap->size0; break;
    case 1: size = qstmap->size1; break;
    case 2: size = qstmap->size2; break;
    case 3: size = qstmap->size3; break;
    case 4: size = qstmap->size4; break;
    case 5: size = qstmap->size5; break;
    case 6: size = qstmap->size6; break;
    case 7: size = qstmap->size7; break;
    default: size = 0; break;
    }
    return size;
}

static unsigned int
qstate_length(const lif_qstate_map_entry_t *qstmap, const int type)
{
    return 1 << qstate_raw_length(qstmap, type);
}

static unsigned int
qstate_size(const lif_qstate_map_entry_t *qstmap, const int type)
{
    return 1 << (qstate_raw_size(qstmap, type) + 5);
}

typedef struct {
    int lif;
    int qtype;
    int qid;
} qstate_qinfo_t;

static int
qstate_qinfo(const lif_qstate_map_entry_t *qstmap,
             const uint64_t addr,
             qstate_qinfo_t *qinfo)
{
    uint64_t base = (uint64_t)qstmap->qstate_base << 12;

    for (int type = 0; type < 8; type++) {
        const uint32_t qsize = qstate_size(qstmap, type);
        const uint32_t qcount = qstate_length(qstmap, type);
        const uint64_t qstate_end = base + qcount * qsize;

        if (base <= addr && addr < qstate_end) {
            const uint32_t qstate_offset = addr - base;

            qinfo->qtype = type;
            qinfo->qid = qstate_offset / qsize;
            return 1;
        }
        base = qstate_end;
    }
    return 0;
}

static int
lif_qstate_map_qinfo(const uint64_t qstate_addr,
                     qstate_qinfo_t *qinfo)
{
    lif_qstate_map_entry_t qstmap;
    int lif;

    for (lif = 0; lif < LIF_QSTATE_MAP_COUNT; lif++) {
        lif_qstate_map_read(lif, &qstmap);
        if (!qstmap.vld) continue;
        if (qstate_qinfo(&qstmap, qstate_addr, qinfo)) {
            qinfo->lif = lif;
            return 1;
        }
    }
    return 0;
}

static uint64_t
dberr_addr(const int entry)
{
    return DBERR_BASE + entry * DBERR_STRIDE;
}

static void
dberr_read(const int entry, db_err_activity_log_entry_t *dberr)
{
    pal_reg_rd32w(dberr_addr(entry), dberr->w, 2);
}

static void
dberr_display(db_err_activity_log_entry_t *dberr)
{
    const int w = 20;

    const uint64_t qstate_addr = (uint64_t)dberr->qstateaddr_or_qid << 5;
    qstate_qinfo_t qinfo;

#define PDB(fmt, field) \
    printf("%-*s: " fmt "\n", w, #field, dberr->field)

    PDB("%ld", vld);
    if (lif_qstate_map_qinfo(qstate_addr, &qinfo)) {
        printf("%-*s: 0x%lx (qstateaddr 0x%lx lif %d qtype %d qid %d)\n",
               w, "qstateaddr_or_qid", dberr->qstateaddr_or_qid,
               qstate_addr,
               qinfo.lif, qinfo.qtype, qinfo.qid);
    } else {
        PDB("0x%lx", qstateaddr_or_qid);
    }
    PDB("0x%lx", pid_or_lif_type);
    PDB("%ld", cnt);
    PDB("%ld", doorbell_merged);
    PDB("%ld", addr_conflict);
    PDB("%ld", tot_ring_err);
    PDB("%ld", host_ring_err);
    PDB("%ld", pid_fail);
    PDB("%ld", qid_ovflow);
}

static void
dberr(int argc, char *argv[])
{
    db_err_activity_log_entry_t dberr;
    int opt, i, single_entry;

    single_entry = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "e:")) != -1) {
        switch (opt) {
        case 'e':
            dberr.w64 = strtoull(optarg, NULL, 0);
            single_entry = 1;
            break;
        default:
            return;
        }
    }

    if (single_entry) {
        dberr_display(&dberr);
    } else {
        for (i = 0; i < DBERR_COUNT; i++) {
            dberr_read(i, &dberr);
            if (!dberr.vld) continue;
            printf("\nentry %d\n", i);
            dberr_display(&dberr);
        }
    }
}
CMDFUNC(dberr,
"display doorbell_err_activity_log entries",
"dberr -e <hexdigits>\n");
