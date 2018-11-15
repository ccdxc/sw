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
void qstate_lif_dump(int lif_start, int lif_end, int queue_type,
                     int qid, int rid, int rsize, int poll,
                     int verbose);
using namespace std;

// Default options
int verbose = 0;
int bwmon = 0;
int crypto = 0;
int no_opt = 1;

int
main(int argc, char *argv[])
{
    int i = 1;
    int interval = 0;
    int queue_dump = 0;
    int lif = -1;
    int qtype = -1;
    int qid = -1;
    int rid = -1;
    int poll = 0;
    int rsize = 65536;
    int lif_max = 2048;

    while (i < (argc)) {
        if (strcmp(argv[i], "-r") == 0) {
            reset_counters();
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-q") == 0) {
            queue_dump = 1;
        } else if (strcmp(argv[i], "-l") == 0) {
            i++;
            if(i >= argc ) {
                printf("Error: -l requires a lif\n");
                return(-1);
            }

            lif = atoi(argv[i]);
            if((lif < 0) || (lif >= 2048)) {
                printf("Error: Invalid lif %d\n", lif);
                return(-1);
            }
            printf("lif = %d\n", lif);
        } else if (strcmp(argv[i], "-t") == 0) {
            i++;
            if(i >= argc ) {
                printf("Error: -t requires a queue type\n");
                return(-1);
            }

            qtype = atoi(argv[i]);
            if((qtype < 0) || (qtype >= 8)) {
                printf("Error: Invalid queue type %d\n", qtype);
                return(-1);
            }
            printf("qtype = %d\n", qtype);
        } else if (strcmp(argv[i], "-i") == 0) {
            i++;
            if(i >= argc ) {
                printf("Error: -i requires a queue id\n");
                return(-1);
            }

            qid = atoi(argv[i]);
            if((qid < 0) || (qid >= 16000000)) {
                printf("Error: Invalid queue Id %d\n", qid);
                return(-1);
            }
            printf("qid = %d\n", qid);
        } else if (strcmp(argv[i], "-R") == 0) {
            i++;
            if(i >= argc ) {
                printf("Error: -R requires a ring id\n");
                return(-1);
            }

            rid = atoi(argv[i]);
            if((rid < 0) || (rid >= 8)) {
                printf("Error: Invalid ring id %d\n", rid);
                return(-1);
            }
            printf("rid = %d\n", rid);
        } else if (strcmp(argv[i], "-z") == 0) {
            i++;
            if(i >= argc ) {
                printf("Error: -z requires ring size\n");
                return(-1);
            }

            rsize = atoi(argv[i]);
            if((rsize < 0) || (rsize > 65536)) {
                printf("Error: Invalid ring size %d\n", rsize);
                return(-1);
            }
            printf("rsize = %d\n", rsize);
        } else if (strcmp(argv[i], "-p") == 0) {
            i++;
            if(i < argc ) {
                poll = atoi(argv[i]);
            }
            printf("Poll = %d\n", poll);
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
            printf("usage: capmon -v[erbose] -r[eset] -q[ueues] -p[cie] -b[wmon] -s[pps] -l[lif] -t[qtype] -i[qid] -R[ring] -p[poll]"
                   "<interval>\n");
            return (0);
        }
        i++;
    }

    if((poll != 0) && ((lif == -1) || (qtype == -1) || (qid == -1))) {
      printf("Error: -p requires lif, qtype and qid to be specified.");
      return(-1);
    }

    if(queue_dump == 1) {
      if(lif >= 0 ) {
        qstate_lif_dump(lif, lif, qtype, qid, rid, rsize, poll, verbose);
      } else {
        qstate_lif_dump(0, (lif_max - 1), qtype, qid, rid, rsize, poll, verbose);
      }
    } else {
      read_counters();
    }

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
