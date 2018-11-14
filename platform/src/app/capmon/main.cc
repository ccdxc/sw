/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <getopt.h>
#include <inttypes.h>
#include <iostream>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

#include "dtls.hpp"
#include "pal.h"

#include "cap_mpu_c_hdr.h"
#include "cap_pbc_c_hdr.h"
#include "cap_ppa_c_hdr.h"
#include "cap_te_c_hdr.h"
#include "cap_top_csr_defines.h"
#include "cap_wa_c_hdr.h"
namespace pt
{
#include "cap_pt_c_hdr.h"
}
namespace pr
{
#include "cap_pr_c_hdr.h"
}
namespace psp
{
#include "cap_psp_c_hdr.h"
}

void read_counters();
void reset_counters();
void qstate_lif_dump(int);
void dump_qstate(int, int, uint64_t);
void measure_pps(int);

using namespace std;

// Default options
int verbose = 0;
int bwmon = 0;
int queues = 0;
int crypto = 0;
int no_opt = 1;

int
main(int argc, char *argv[])
{
    int i = 1;
    int interval = 0;

    while (i < (argc)) {
        if (strcmp(argv[i], "-r") == 0) {
            reset_counters();
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-q") == 0) {
            printf("==Queues==\n");
            qstate_lif_dump(verbose);
            return 0;
        } else if (strcmp(argv[i], "-n") == 0) {
            no_opt = 0;
        } else if (strcmp(argv[i], "-b") == 0) {
            bwmon = 1;
        } else if (strcmp(argv[i], "-c") == 0) {
            crypto = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            i++;
            if (argc > i) {
                interval = atoi(argv[i]);
            } else {
                interval = 1;
            }
            measure_pps(interval);
            return (0);
        } else {
            printf("usage: capmon -v[erbose] -r[eset] -q[ueues] -p[cie] -b[wmon] -s[pps] "
                   "<interval>\n");
            return (0);
        }
        i++;
    }

    read_counters();

    exit(0);
}

void
measure_pps(int interval)
{
    uint64_t timestamp_start, timestamp_end;
    uint32_t txd_start_cnt, txd_end_cnt;
    uint32_t rxd_start_cnt, rxd_end_cnt;
    uint64_t p4ig_start_cnt, p4ig_end_cnt;
    uint64_t p4eg_start_cnt, p4eg_end_cnt;
    uint32_t zero[4] = {0};
    uint32_t cnt[4] = {0};

    // Clear Packet Counters
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, zero, 4);
    pal_reg_wr32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, zero, 4);

    // Capture Timestamp
    pal_reg_rd32w(CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_STA_HBM_TIMESTAMP_BYTE_ADDRESS,
                  cnt, 2);
    timestamp_start =
        ((uint64_t)CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_0_2_VALUE_31_0_GET(cnt[0])) |
        ((uint64_t)CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_1_2_VALUE_47_32_GET(cnt[1])
         << 32);

    // TXDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, cnt, 3);
    txd_start_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

    // RXDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, cnt, 3);
    rxd_start_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

    // P4 EG
    pal_reg_rd32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
    p4eg_start_cnt =
        ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

    // P4 IG
    pal_reg_rd32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
    p4ig_start_cnt =
        ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

    sleep(interval);

    // Capture Timestamp
    pal_reg_rd32w(CAP_ADDR_BASE_PB_PBC_OFFSET + CAP_PBC_CSR_HBM_STA_HBM_TIMESTAMP_BYTE_ADDRESS,
                  cnt, 2);
    timestamp_end =
        ((uint64_t)CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_0_2_VALUE_31_0_GET(cnt[0])) |
        ((uint64_t)CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_1_2_VALUE_47_32_GET(cnt[1])
         << 32);

    // TXDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, cnt, 3);
    txd_end_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

    // RXDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, cnt, 3);
    rxd_end_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

    // P4 EG
    pal_reg_rd32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
    p4eg_end_cnt = ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
                    ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

    // P4 IG
    pal_reg_rd32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET + CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS, cnt, 4);
    p4ig_end_cnt = ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
                    ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1]) << 32));

    uint64_t ts_delta = timestamp_end - timestamp_start;
    uint64_t txd_delta = txd_end_cnt - txd_start_cnt;
    uint64_t rxd_delta = rxd_end_cnt - rxd_start_cnt;
    uint64_t p4ig_delta = p4ig_end_cnt - p4ig_start_cnt;
    uint64_t p4eg_delta = p4eg_end_cnt - p4eg_start_cnt;

    printf("PPS: TXDMA=%.0f RXDMA=%.0f P4IG=%.0f P4EG=%.0f\n", txd_delta * 1e9 / (ts_delta * 1.2),
           rxd_delta * 1e9 / (ts_delta * 1.2), p4ig_delta * 1e9 / (ts_delta * 1.2),
           p4eg_delta * 1e9 / (ts_delta * 1.2));
}

void
read_counters()
{
    uint8_t mpu, pipeline, stage;

    if (bwmon) {
        printf("==AXI==\n");
        bwmon_read_counters();
    }

    if (crypto) {
        printf("==Crypto==\n");
        crypto_read_counters(verbose);
    }

    if (!no_opt) {
        return;
    }

    printf("== PCIe ==\n");
    pxb_read_counters(verbose);

    printf("== Doorbell ==\n");
    doorbell_read_counters(verbose);

    printf("== TX Scheduler ==\n");
    txs_read_counters(verbose);

    // Pipelines
    for (pipeline = 0; pipeline < PIPE_CNT; pipeline++) {
        switch (pipeline) {
        case TXDMA:
            printf("== TXDMA ==\n");
            npv_read_counters(verbose);
            break;
        case RXDMA:
            printf("== RXDMA ==\n");
            psp_read_counters(verbose);
            break;
        case P4IG:
            printf("== P4IG ==\n");
            parser_read_counters(verbose, CAP_ADDR_BASE_PPA_PPA_1_OFFSET);
            break;
        case P4EG:
            printf("== P4EG ==\n");
            parser_read_counters(verbose, CAP_ADDR_BASE_PPA_PPA_0_OFFSET);
            break;
        }

        // Visit each Stage in pipeline
        for (stage = 0; stage < (((pipeline == TXDMA) | (pipeline == RXDMA)) ? 8 : 6); stage++) {
            printf(" S%d:", stage);
            if (verbose) {
                printf("\n");
            }
            stg_poll(verbose, pipeline, stage);
            te_read_counters(verbose, pipeline, stage);
            sdp_read_counters(verbose, pipeline, stage);
            // Visit each MPU in state
            for (mpu = 0; mpu < 4; mpu++) {
                mpu_read_counters(verbose, pipeline, stage, mpu);
                mpu_read_table_addr(verbose, pipeline, stage, mpu);
            }
        }

        switch (pipeline) {
        case TXDMA:
            printf(" TxDMA:");
            ptd_read_counters(verbose);
            break;
        case RXDMA:
            printf(" RxDMA:");
            prd_read_counters(verbose);
            break;
        case P4IG:
            break;
        case P4EG:
            break;
        }
    }

    pb_read_counters(verbose);
}

void
reset_counters()
{
    uint8_t mpu, pipeline, stage;

    if (crypto) {
        crypto_reset_counters(verbose);
    }

    if (!no_opt) {
        return;
    }

    // PCIe counter reset
    pxb_reset_counters(verbose);
    // Doorbell counter reset
    doorbell_reset_counters(verbose);
    // TXS counter reset
    txs_reset_counters(verbose);

    // Pipelines
    for (pipeline = 0; pipeline < 4; pipeline++) {
        switch (pipeline) {
        case TXDMA:
            npv_reset_counters(verbose);
            break;
        case RXDMA:
            psp_reset_counters(verbose);
            break;
        case P4IG:
            parser_reset_counters(verbose, CAP_ADDR_BASE_PPA_PPA_1_OFFSET);
            break;
        case P4EG:
            parser_reset_counters(verbose, CAP_ADDR_BASE_PPA_PPA_0_OFFSET);
            break;
        }

        // Visit each Stage in pipeline
        for (stage = 0; stage < (((pipeline == TXDMA) | (pipeline == RXDMA)) ? 8 : 6); stage++) {
            te_reset_counters(verbose, pipeline, stage);
            sdp_reset_counters(verbose, pipeline, stage);
            // Visit each MPU in stage
            for (mpu = 0; mpu < 4; mpu++) {
                mpu_reset_counters(verbose, pipeline, stage, mpu);
            }
        }

        switch (pipeline) {
        case TXDMA:
            ptd_reset_counters(verbose);
            break;
        case RXDMA:
            prd_reset_counters(verbose);
            break;
        case P4IG:
            break;
        case P4EG:
            break;
        }
    }
}

void
qstate_lif_dump(int verbose)
{
    uint32_t cnt[4], size[8], length[8];
    uint32_t valid, hint, hint_cos;
    uint64_t base;
    int lif, type, q, max_type;
    int this_size, this_len;

    for (lif = 0; lif < 128; lif++) {
        pal_reg_rd32w(CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_DHS_LIF_QSTATE_MAP_BYTE_ADDRESS +
                          (16 * lif),
                      cnt, 4);
        // decode lif qstate table:
        base = (uint64_t)CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_QSTATE_BASE_GET(cnt[0]);
        valid = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_VLD_GET(cnt[0]);
        hint = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SCHED_HINT_EN_GET(cnt[2]);
        hint_cos = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SCHED_HINT_COS_GET(cnt[2]);
        // 3 bit size is qstate size: 32B/64B/128B...
        size[0] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_SIZE0_GET(cnt[0]);
        size[1] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE1_GET(cnt[1]);
        size[2] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE2_GET(cnt[1]);
        size[3] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE3_GET(cnt[1]);
        size[4] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_SIZE4_GET(cnt[1]);
        size[5] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE5_GET(cnt[2]);
        size[6] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE6_GET(cnt[2]);
        size[7] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_SIZE7_GET(cnt[2]);
        // 5 bit length is lg2 # entries:
        length[0] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_LENGTH0_GET(cnt[0]);
        length[1] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_4_LENGTH1_0_0_GET(cnt[0]) |
                    (CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH1_4_1_GET(cnt[1]) << 1);
        length[2] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH2_GET(cnt[1]);
        length[3] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH3_GET(cnt[1]);
        length[4] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH4_GET(cnt[1]);
        length[5] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_1_4_LENGTH5_0_0_GET(cnt[1]) |
                    (CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH5_4_1_GET(cnt[2]) << 1);
        length[6] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH6_GET(cnt[2]);
        length[7] = CAP_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_2_4_LENGTH7_GET(cnt[2]);

        for (max_type = 0; max_type < 8; max_type++) {
            if (size[max_type] == 0) {
                break;
            }
        }

        if (valid) {
            base = base << 12; // base is 4KB aligned
            printf("LIF %u valid, qstate_base=0x%lx, hint=%u, hint_cos=%u\n", lif, base, hint,
                   hint_cos);
            for (type = 0; type < max_type; type++) {
                this_len = 1 << length[type];
                this_size = 32 * (1 << size[type]);
                printf(" type %u type_base = 0x%lx, length=%u entries, qstate_size=%u bytes\n",
                       type, base, this_len, this_size);
                for (q = 0; q < this_len; q++) {
                    dump_qstate(verbose, q, base + (uint64_t)(q * this_size));
                }
                base += this_size * this_len;
            }
        }
    }
}

void
dump_qstate(int verbose, int qid, uint64_t qaddr)
{
    uint8_t buf[64];

    typedef struct {
        uint8_t pc_offset;
        uint8_t rsvd0;
        uint8_t cosA : 4;
        uint8_t cosB : 4;
        uint8_t cos_sel;
        uint8_t eval_last;
        uint8_t host : 4;
        uint8_t total : 4;
        uint16_t pid;

        struct {
            uint16_t pi;
            uint16_t ci;
        } rings[8];

    } qstate_t;

    pal_mem_rd(qaddr, buf, 64, 0);
    qstate_t *q = (qstate_t *)buf;

    printf("QID %u: ", qid);
    if (verbose) {
        printf(" total_rings=%02d cosA=%02d cosB=%02d cos_sel=0x%02x pc=0x%02x", q->total, q->cosA,
               q->cosB, q->cos_sel, q->pc_offset);
    }
    for (uint8_t ring = 0; ring < q->total; ring++) {
        printf(" ring %02d: PI=%06d CI=%06d", ring, q->rings[ring].pi, q->rings[ring].ci);
    }
    printf("\n");
}
