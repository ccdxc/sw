//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for elbmon tool
/// usage: elbmon -v[erbose] -r[eset] -q[ueues] -p[cie] -b[wmon] -s[pps]
///  -l[lif] -t[qtype] -i[qid] -R[ring] -p[poll]<interval>
///
//===----------------------------------------------------------------------===//

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
#include "nic/sdk/platform/pal/include/pal.h"

#include "elb_mpu_c_hdr.h"
#include "elb_pbc_c_hdr.h"
#include "elb_ppa_c_hdr.h"
#include "elb_te_c_hdr.h"
#include "elb_top_csr_defines.h"
#include "elb_wa_c_hdr.h"
#include "elbmon.hpp"
namespace pt {
#include "elb_pt_c_hdr.h"
}
namespace pr {
#include "elb_pr_c_hdr.h"
}
namespace psp {
#include "elb_psp_c_hdr.h"
}

void read_counters();
void reset_counters();
void qstate_lif_dump(int);
void dump_qstate(int, int, uint64_t);
void measure_pps(int);
void qstate_lif_dump(int lif_start, int lif_end, int queue_type, int qid_start,
                     int qid_end, int rid, int rsize, int poll, int verbose);
using namespace std;

// Default options
int verbose = 0;
int bwmon = 0;
int crypto = 0;
int no_opt = 1;
bool export_to_file = 0;
export_format_t export_format = EXPORT_TEXT;

int
main (int argc, char *argv[])
{
    int i = 1;
    int interval = 0;
    int queue_dump = 0;
    int qtype = -1;
    int qid_max = 16000000;
    int qid_start = 0;
    int qid_end = qid_max - 1;
    int rid_max = 8;
    int rid = -1;
    int poll = 0;
    int rsize_max = 65536;
    int rsize = rsize_max;
    int lif_max = 2048;
    int lif_start = 0;
    int lif_end = lif_max - 1;

    while (i < (argc)) {
        if (strcmp(argv[i], "-r") == 0) {
            reset_counters();
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-q") == 0) {
            queue_dump = 1;
        } else if (strcmp(argv[i], "-l") == 0) {
            int start = 0;
            int end = 0;
            int inputs = 0;

            i++;
            inputs = sscanf(argv[i], "%d:%d", &start, &end);
            if (!inputs) {
                printf("Error: -l requires a lif\n");
                return (-1);
            }
            lif_start = start;

            if (--inputs) {
                lif_end = end;
            } else {
                // If the user hasn't provided a range, assume
                // we are interested in only one lif.
                lif_end = lif_start;
            }

            if ((lif_start < 0) || (lif_end >= lif_max) ||
                (lif_start > lif_end)) {
                printf("Error: Invalid lif range %d:%d\n", lif_start, lif_end);
                return (-1);
            }
            printf("lif = %d:%d\n", lif_start, lif_end);
        } else if (strcmp(argv[i], "-t") == 0) {
            i++;
            if (i >= argc) {
                printf("Error: -t requires a queue type\n");
                return (-1);
            }

            qtype = atoi(argv[i]);
            if ((qtype < 0) || (qtype >= 8)) {
                printf("Error: Invalid queue type %d\n", qtype);
                return (-1);
            }
            printf("qtype = %d\n", qtype);
        } else if (strcmp(argv[i], "-i") == 0) {
            int start = 0;
            int end = 0;
            int inputs = 0;

            i++;
            inputs = sscanf(argv[i], "%d:%d", &start, &end);
            if (!inputs) {
                printf("Error: -i requires a lif\n");
                return (-1);
            }

            qid_start = start;
            if (--inputs) {
                qid_end = end;
            } else {
                // If the user hasn't provided a range, assume
                // we are interested in only one qid.
                qid_end = qid_start;
            }
            if ((qid_start < 0) || (qid_end >= qid_max) ||
                (qid_start > qid_end)) {
                printf("Error: Invalid qid range %d:%d\n", qid_start, qid_end);
                return (-1);
            }
            printf("qid = %d:%d\n", qid_start, qid_end);
        } else if (strcmp(argv[i], "-R") == 0) {
            i++;
            if (i >= argc) {
                printf("Error: -R requires a ring id\n");
                return (-1);
            }

            rid = atoi(argv[i]);
            if ((rid < 0) || (rid >= rid_max)) {
                printf("Error: Invalid ring id %d\n", rid);
                return (-1);
            }
            printf("rid = %d\n", rid);
        } else if (strcmp(argv[i], "-z") == 0) {
            i++;
            if (i >= argc) {
                printf("Error: -z requires ring size\n");
                return (-1);
            }

            rsize = atoi(argv[i]);
            if ((rsize < 0) || (rsize > rsize_max)) {
                printf("Error: Invalid ring size %d\n", rsize);
                return (-1);
            }
            printf("rsize = %d\n", rsize);
        } else if (strcmp(argv[i], "-p") == 0) {
            i++;
            if (i < argc) {
                poll = atoi(argv[i]);
            }
            printf("Poll = %d\n", poll);
        } else if (strcmp(argv[i], "-n") == 0) {
            no_opt = 0;
        } else if (strcmp(argv[i], "-b") == 0) {
            bwmon = 1;
        } else if (strcmp(argv[i], "-c") == 0) {
            crypto = 1;
        } else if (strcmp(argv[i], "-x") == 0) {
            export_to_file = true;
            i++;
            if (i < argc) {
                if ((strcmp(argv[i], "txt") == 0) ||
                    (strcmp(argv[i], "text") == 0)) {
                    export_format = EXPORT_TEXT;
                } else if (strcmp(argv[i], "json") == 0) {
                    export_format = EXPORT_JSON;
                } else if (strcmp(argv[i], "xml") == 0) {
                    export_format = EXPORT_XML;
                } else if (strcmp(argv[i], "csv") == 0) {
                    export_format = EXPORT_CSV;
                } else if (strcmp(argv[i], "html") == 0) {
                    export_format = EXPORT_HTML;
                } else {
                    // set to TEXT by default
                    export_format = EXPORT_TEXT;
                    i--;
                }
            }
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
            printf(
                "usage: elbmon -v[erbose] -r[eset] -q[ueues] -p[cie] -b[wmon]"
                " -s[pps] -l[lif] -t[qtype] -i[qid] -R[ring] -p[poll]"
                "<interval>\n\n");
            printf("Example: elbmon -q -l 1003:1005 -t 0 -i 0:5 -R 1 -p 100\n");
            return (0);
        }
        i++;
    }
    elbmon_struct_init(NULL);

    if ((poll != 0) &&
        ((lif_start != lif_end) || (qtype == -1) || (qid_start != qid_end))) {
        printf("Error: -p requires just one lif, qtype and qid to be "
               "specified.\n");
        return (-1);
    }

    if (queue_dump == 1) {
        qstate_lif_dump(lif_start, lif_end, qtype, qid_start, qid_end, rid,
                        rsize, poll, verbose);
    } else {
        read_counters();
        elbtop_display_routine(NULL);
    }

    if (export_to_file && fp) {
        fclose(fp);
    }

    exit(0);
}

void
measure_pps (int interval)
{
    uint64_t timestamp_start, timestamp_end;
    uint32_t txd_start_cnt, txd_end_cnt;
    uint32_t rxd_start_cnt, rxd_end_cnt;
    uint64_t p4ig_start_cnt, p4ig_end_cnt;
    uint64_t p4eg_start_cnt, p4eg_end_cnt;
    uint32_t zero[4] = {0};
    uint32_t cnt[4] = {0};

    // Clear Packet Counters
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(ELB_ADDR_BASE_PPA_PPA_0_OFFSET +
                      ELB_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  zero, 4);
    pal_reg_wr32w(ELB_ADDR_BASE_PPA_PPA_1_OFFSET +
                      ELB_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  zero, 4);

    // Capture Timestamp
    pal_reg_rd32w(ELB_ADDR_BASE_PB_PBC_OFFSET +
                      ELB_PBC_CSR_HBM_STA_HBM_TIMESTAMP_BYTE_ADDRESS,
                  cnt, 2);
    timestamp_start =
        ((uint64_t)
             ELB_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_0_2_VALUE_31_0_GET(
                 cnt[0])) |
        ((uint64_t)
             ELB_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_1_2_VALUE_47_32_GET(
                 cnt[1])
         << 32);

    // TXDMA
    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  cnt, 3);
    txd_start_cnt = ELB_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

    // RXDMA
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  cnt, 3);
    rxd_start_cnt = ELB_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

    // P4 EG
    pal_reg_rd32w(ELB_ADDR_BASE_PPA_PPA_0_OFFSET +
                      ELB_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  cnt, 4);
    p4eg_start_cnt =
        ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    // P4 IG
    pal_reg_rd32w(ELB_ADDR_BASE_PPA_PPA_1_OFFSET +
                      ELB_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  cnt, 4);
    p4ig_start_cnt =
        ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    sleep(interval);

    // Capture Timestamp
    pal_reg_rd32w(ELB_ADDR_BASE_PB_PBC_OFFSET +
                      ELB_PBC_CSR_HBM_STA_HBM_TIMESTAMP_BYTE_ADDRESS,
                  cnt, 2);
    timestamp_end =
        ((uint64_t)
             ELB_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_0_2_VALUE_31_0_GET(
                 cnt[0])) |
        ((uint64_t)
             ELB_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_1_2_VALUE_47_32_GET(
                 cnt[1])
         << 32);

    // TXDMA
    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  cnt, 3);
    txd_end_cnt = ELB_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

    // RXDMA
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  cnt, 3);
    rxd_end_cnt = ELB_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

    // P4 EG
    pal_reg_rd32w(ELB_ADDR_BASE_PPA_PPA_0_OFFSET +
                      ELB_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  cnt, 4);
    p4eg_end_cnt =
        ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    // P4 IG
    pal_reg_rd32w(ELB_ADDR_BASE_PPA_PPA_1_OFFSET +
                      ELB_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  cnt, 4);
    p4ig_end_cnt =
        ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)ELB_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    uint64_t ts_delta = timestamp_end - timestamp_start;
    uint64_t txd_delta = txd_end_cnt - txd_start_cnt;
    uint64_t rxd_delta = rxd_end_cnt - rxd_start_cnt;
    uint64_t p4ig_delta = p4ig_end_cnt - p4ig_start_cnt;
    uint64_t p4eg_delta = p4eg_end_cnt - p4eg_start_cnt;

    printf("PPS: TXDMA=%.0f RXDMA=%.0f P4IG=%.0f P4EG=%.0f\n",
           txd_delta * 1e9 / (ts_delta * 1.2),
           rxd_delta * 1e9 / (ts_delta * 1.2),
           p4ig_delta * 1e9 / (ts_delta * 1.2),
           p4eg_delta * 1e9 / (ts_delta * 1.2));
}

void
read_counters (void)
{
    uint8_t mpu, pipeline, stage;

    if (bwmon) {
        bwmon_read_counters();
    }

    if (crypto) {
        crypto_read_counters(verbose);
    }

    if (!no_opt) {
        return;
    }

    pxb_read_counters(verbose);

    doorbell_read_counters(verbose);

    txs_read_counters(verbose);

    // Pipelines
    for (pipeline = 0; pipeline < PIPE_CNT; pipeline++) {
        switch (pipeline) {
            case TXDMA:
                npv_read_counters(verbose);
                break;
            case RXDMA:
                psp_read_counters(verbose);
                break;
            case P4IG:
                parser_read_counters(P4IG, verbose,
                                     ELB_ADDR_BASE_PPA_PPA_1_OFFSET);
                break;
            case P4EG:
                parser_read_counters(P4EG, verbose,
                                     ELB_ADDR_BASE_PPA_PPA_0_OFFSET);
                break;
        }

        // Visit each Stage in pipeline
        for (stage = 0;
             stage < (((pipeline == TXDMA) | (pipeline == RXDMA)) ? 8 : 6);
             stage++) {
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
                ptd_read_counters(verbose);
                break;
            case RXDMA:
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
reset_counters (void)
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
                parser_reset_counters(verbose, ELB_ADDR_BASE_PPA_PPA_1_OFFSET);
                break;
            case P4EG:
                parser_reset_counters(verbose, ELB_ADDR_BASE_PPA_PPA_0_OFFSET);
                break;
        }

        // Visit each Stage in pipeline
        for (stage = 0;
             stage < (((pipeline == TXDMA) | (pipeline == RXDMA)) ? 8 : 6);
             stage++) {
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
