//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for fetching and storing info
/// from rxdma pipline
///
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "cap_top_csr_defines.h"
namespace pr {
#include "cap_pr_c_hdr.h"
}
namespace psp {
#include "cap_psp_c_hdr.h"
}

#include "platform/pal/include/pal.h"
#include "capmon.hpp"

void
prd_read_counters(int verbose)
{
    uint32_t sta_id = 0, sta_fifo = 0;
    uint32_t cnt[4] = {0};
    uint32_t pend_rd = 0;
    uint32_t pend_wr = 0;
    uint32_t rd_ff_empty = 0;
    uint32_t rd_ff_full = 0;
    uint32_t wr_ff_empty = 0;
    uint32_t wr_ff_full = 0;
    uint32_t pkt_ff_empty = 0;
    uint32_t pkt_ff_full = 0;
    uint32_t pkt_xoff = 0, phv_xoff = 0, host_xoff = 0;
    uint32_t pkt_xoff_cnt = 0, phv_xoff_cnt = 0, host_xoff_cnt = 0;
    uint32_t pkt_ff_depth = 0;
    int i;
    int polls = 100;

    // Drops at the end of RXDMA pipeline
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_BYTE_ADDRESS +
                      CAP_PRD_CSR_CNT_PHV_BYTE_OFFSET,
                  cnt, 4);
    uint32_t phv_drop = CAP_PRD_CSR_CNT_PHV_CNT_PHV_1_4_DROP_GET(cnt[1]);
    uint32_t phv_err = CAP_PRD_CSR_CNT_PHV_CNT_PHV_2_4_ERR_GET(cnt[2]);
    uint32_t phv_recirc = CAP_PRD_CSR_CNT_PHV_CNT_PHV_3_4_RECIRC_GET(cnt[3]);

    // Number of PHVs sent from RXDMA Stage 7 to PDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_MA_BYTE_ADDRESS,
                  cnt, 3);
    uint64_t phv_cnt =
        CAP_PRD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]) |
        ((uint64_t)(CAP_PRD_CSR_CNT_MA_CNT_MA_1_3_SOP_39_32_GET(cnt[1])) << 32);

    // Number of PHVs recirculated
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_PS_RESUB_PKT_BYTE_ADDRESS,
                  cnt, 3);
    uint64_t resub_cnt =
        CAP_PRD_CSR_CNT_PS_RESUB_PKT_CNT_PS_RESUB_PKT_0_3_SOP_31_0_GET(cnt[0]) |
        ((uint64_t)(CAP_PRD_CSR_CNT_PS_RESUB_PKT_CNT_PS_RESUB_PKT_1_3_SOP_39_32_GET(cnt[1])) << 32);

    // Number of packets received
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  cnt, 3);
    uint64_t pb_cnt =
        CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]) |
        ((uint64_t)(CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_1_3_SOP_39_32_GET(cnt[1])) << 32);

    for (i = 0; i < polls; i++) {
        // FIFO Status
        pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                          CAP_PR_CSR_PRD_STA_FIFO_BYTE_ADDRESS,
                      &sta_fifo, 1);

        rd_ff_full += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_FULL_GET(sta_fifo);
        rd_ff_empty += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_EMPTY_GET(sta_fifo);
        wr_ff_full += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_FULL_GET(sta_fifo);
        wr_ff_empty += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_EMPTY_GET(sta_fifo);
        pkt_ff_full += CAP_PRD_CSR_STA_FIFO_PKT_FF_FULL_GET(sta_fifo);
        pkt_ff_empty += CAP_PRD_CSR_STA_FIFO_PKT_FF_EMPTY_GET(sta_fifo);

        // Pending reads/writes
        pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                          CAP_PR_CSR_PRD_STA_ID_BYTE_ADDRESS,
                      &sta_id, 1);

        pend_rd += CAP_PRD_CSR_STA_ID_RD_PEND_CNT_GET(sta_id);
        pend_wr += CAP_PRD_CSR_STA_ID_WR_PEND_CNT_GET(sta_id);

        // XOFF
        pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                          CAP_PR_CSR_PRD_STA_XOFF_BYTE_ADDRESS,
                      cnt, 3);

        phv_xoff += CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_NUMPHV_XOFF_GET(cnt[0]);
        phv_xoff_cnt +=
            CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_NUMPHV_COUNTER_GET(cnt[0]);

        host_xoff +=
            CAP_PRD_CSR_STA_XOFF_STA_XOFF_1_3_HOST_PBUS_XOFF_GET(cnt[1]);
        host_xoff_cnt +=
            CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_HOSTQ_XOFF_COUNTER_GET(cnt[0]);

        pkt_xoff += CAP_PRD_CSR_STA_XOFF_STA_XOFF_1_3_PKT_PBUS_XOFF_GET(cnt[1]);
        pkt_xoff_cnt +=
            (CAP_PRD_CSR_STA_XOFF_STA_XOFF_1_3_PKT_XOFF_COUNTER_10_9_GET(cnt[1])
             << 8) |
            CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_PKT_XOFF_COUNTER_8_0_GET(cnt[0]);
        pkt_ff_depth +=
            CAP_PRD_CSR_STA_XOFF_STA_XOFF_2_3_PKT_FF_DEPTH_GET(cnt[2]);
    }

    capmon_pipeline_data_store1(RXDMA, phv_cnt, phv_drop, phv_err, phv_recirc,
                                pb_cnt, resub_cnt);
    capmon_pipeline_data_store2(RXDMA, pend_rd, pend_wr,
                                rd_ff_empty, rd_ff_full,
                                wr_ff_empty, wr_ff_full,
                                pkt_ff_empty, pkt_ff_full,
                                pkt_ff_depth, polls);
    capmon_pipeline_data_store4(RXDMA,
                                phv_xoff, phv_xoff_cnt,
                                pkt_xoff , pkt_xoff_cnt,
                                host_xoff, host_xoff_cnt,
                                polls);
}

void
prd_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_BYTE_ADDRESS +
                      CAP_PRD_CSR_CNT_PHV_BYTE_OFFSET,
                  zero, 4);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_MA_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_PS_RESUB_PKT_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  zero, 3);
}

void
psp_read_counters(int verbose)
{
    uint64_t ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt,
        ma_recirc_cnt;

    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  (uint32_t *)&ma_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  (uint32_t *)&ma_drop_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  (uint32_t *)&ma_recirc_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  (uint32_t *)&sw_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pb_pbus_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pr_pbus_cnt, 2);

    capmon_pipeline_data_store3(RXDMA,
                                ma_cnt, ma_drop_cnt, ma_recirc_cnt,
                                sw_cnt, pb_pbus_cnt, pr_pbus_cnt);
}

void
psp_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET +
                      CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  zero, 2);
}
