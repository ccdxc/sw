//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for capmon tool
/// usage: capmon -v[erbose] -r[eset] -q[ueues] -b[wmon] -s[]<interval>
///  -l[lif] -t[qtype] -i[qid] -R[ring] -p[oll]<count>
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
#include "platform/pal/include/pal.h"

#include "cap_mpu_c_hdr.h"
#include "cap_pbc_c_hdr.h"
#include "cap_ppa_c_hdr.h"
#include "cap_te_c_hdr.h"
#include "cap_top_csr_defines.h"
#include "cap_wa_c_hdr.h"
#include "capmon.hpp"
namespace pt {
#include "cap_pt_c_hdr.h"
}
namespace pr {
#include "cap_pr_c_hdr.h"
}
namespace psp {
#include "cap_psp_c_hdr.h"
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

void usage()
{
    cout << "usage: capmon -v[erbose] -r[eset] -q[ueues] -b[wmon]"
         << " -s[]<interval> -l[lif] -t[qtype] -i[qid] -R[ring] -p[oll]<count>"
         << "\n" << endl;
    cout << "Example: capmon -q -l 1003 -t 0 -i 5 -R 1 -p 100" << endl;
}

int
main(int argc, char *argv[])
{
    int interval = 1;
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
    int opt = 0;
    int start = 0;
    int end = 0;
    int inputs = 0;

    while ((opt = getopt(argc, argv, "rvql:i:t:R:z:p::nbcx:s::h")) != -1) {
        switch (opt) {
        case 'r':
            reset_counters();
            break;
        case 'v':
            verbose=1;
            break;
        case 'q':
            queue_dump = 1;
            break;
        case 'l':
            start = 0;
            end = 0;
            inputs = 0;
            inputs = sscanf(optarg, "%d:%d", &start, &end);
            if (!inputs) {
                cout << "Error: -l requires a valid lif range" << endl;
                usage();
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
                cout << "Error: Invalid lif range " << lif_start << ":"
                     << lif_end << " provided. Valid range is 0:2047"
                     << endl;
                return (-1);
            }
            cout << "lif = " << lif_start << ":" << lif_end << endl;
            break;
        case 't':
            qtype = atoi(optarg);
            if ((qtype < 0) || (qtype >= 8)) {
                cout << "Error: Invalid queue type " << qtype << endl;
                return (-1);
            }
            cout << "qtype = " << qtype << endl;
            break;
        case 'i':
            start = 0;
            end = 0;
            inputs = 0;

            inputs = sscanf(optarg, "%d:%d", &start, &end);
            if (!inputs) {
                cout << "Error: -i requires a queue range" << endl;
                usage();
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
                cout << "Error: Invalid qid range " << qid_start << ":"
                     << qid_end << " provided. Valid range is 0:15999999"
                     << endl;
                return (-1);
            }
            cout << "qid = " << qid_start << ":" << qid_end << endl;
            break;
        case 'R':
            rid = atoi(optarg);
            if ((rid < 0) || (rid >= rid_max)) {
                cout << "Error: Invalid ring id " << rid << " should not "
                     << "exceed max value " << rid_max << endl;
                return (-1);
            }
            cout << "rid = " << rid << endl;
            break;
        case 'z':
            rsize = atoi(optarg);
            if ((rsize < 0) || (rsize > rsize_max)) {
                cout << "Error: Invalid ring size " << rsize << " should "
                     << "not exceed max size " << rsize_max << endl;
                return (-1);
            }
            cout << "rsize = " << rsize << endl;
            break;
        case 'p':
            if (optarg) {
                poll = atoi(optarg);
            } else {
                if (optind < argc) {
                    poll = atoi(argv[optind]);
                    optind++;
                }
            }
            cout << "Poll = " << poll << endl;
            break;
        case 'n':
            no_opt = 0;
            break;
        case 'b':
            bwmon = 1;
            break;
        case 'c':
            crypto = 1;
            break;
        case 'x':
            export_to_file = true;
            if ((strcmp(optarg, "txt") == 0) ||
                (strcmp(optarg, "text") == 0)) {
                export_format = EXPORT_TEXT;
            } else if (strcmp(optarg, "json") == 0) {
                export_format = EXPORT_JSON;
            } else if (strcmp(optarg, "xml") == 0) {
                export_format = EXPORT_XML;
            } else if (strcmp(optarg, "csv") == 0) {
                export_format = EXPORT_CSV;
            } else if (strcmp(optarg, "html") == 0) {
                export_format = EXPORT_HTML;
            } else {
                // set to TEXT by default
                export_format = EXPORT_TEXT;
            }
            break;
        case 's':
            if (optarg) {
                interval = atoi(optarg);
            } else {
                if (optind < argc) {
                    interval = atoi(argv[optind]);
                    optind++;
                }
            }
            measure_pps(interval);
            return (0);
        case 'h':
            usage();
            return (0);
        default:
            usage();
            return (-1);
        }
    }
    capmon_struct_init(NULL);

    if ((poll != 0) &&
        ((lif_start != lif_end) || (qtype == -1) || (qid_start != qid_end))) {
        cout << "Error: -p requires just one lif, qtype and one qid to be "
             << "specified." << endl;
        usage();
        return (-1);
    }

    if (queue_dump == 1) {
        qstate_lif_dump(lif_start, lif_end, qtype, qid_start, qid_end, rid,
                        rsize, poll, verbose);
    } else {
        read_counters();
        captop_display_routine(NULL);
    }

    if (export_to_file && fp) {
        fclose(fp);
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
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET +
                      CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  zero, 4);
    pal_reg_wr32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET +
                      CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  zero, 4);

    // Capture Timestamp
    pal_reg_rd32w(CAP_ADDR_BASE_PB_PBC_OFFSET +
                      CAP_PBC_CSR_HBM_STA_HBM_TIMESTAMP_BYTE_ADDRESS,
                  cnt, 2);
    timestamp_start =
        ((uint64_t)
             CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_0_2_VALUE_31_0_GET(
                 cnt[0])) |
        ((uint64_t)
             CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_1_2_VALUE_47_32_GET(
                 cnt[1])
         << 32);

    // TXDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  cnt, 3);
    txd_start_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

    // RXDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  cnt, 3);
    rxd_start_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

    // P4 EG
    pal_reg_rd32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET +
                      CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  cnt, 4);
    p4eg_start_cnt =
        ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    // P4 IG
    pal_reg_rd32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET +
                      CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  cnt, 4);
    p4ig_start_cnt =
        ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    sleep(interval);

    // Capture Timestamp
    pal_reg_rd32w(CAP_ADDR_BASE_PB_PBC_OFFSET +
                      CAP_PBC_CSR_HBM_STA_HBM_TIMESTAMP_BYTE_ADDRESS,
                  cnt, 2);
    timestamp_end =
        ((uint64_t)
             CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_0_2_VALUE_31_0_GET(
                 cnt[0])) |
        ((uint64_t)
             CAP_PBCHBM_CSR_STA_HBM_TIMESTAMP_STA_HBM_TIMESTAMP_1_2_VALUE_47_32_GET(
                 cnt[1])
         << 32);

    // TXDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  cnt, 3);
    txd_end_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

    // RXDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  cnt, 3);
    rxd_end_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

    // P4 EG
    pal_reg_rd32w(CAP_ADDR_BASE_PPA_PPA_0_OFFSET +
                      CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  cnt, 4);
    p4eg_end_cnt =
        ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
          << 32));

    // P4 IG
    pal_reg_rd32w(CAP_ADDR_BASE_PPA_PPA_1_OFFSET +
                      CAP_PPA_CSR_CNT_PPA_PB_BYTE_ADDRESS,
                  cnt, 4);
    p4ig_end_cnt =
        ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_0_4_SOP_31_0_GET(cnt[0]) +
         ((uint64_t)CAP_PPA_CSR_CNT_PPA_PB_CNT_PPA_PB_1_4_SOP_39_32_GET(cnt[1])
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
read_counters()
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
                                     CAP_ADDR_BASE_PPA_PPA_1_OFFSET);
                break;
            case P4EG:
                parser_read_counters(P4EG, verbose,
                                     CAP_ADDR_BASE_PPA_PPA_0_OFFSET);
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
