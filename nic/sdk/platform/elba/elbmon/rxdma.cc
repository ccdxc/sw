//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
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

#include "elb_top_csr_defines.h"
#include "elb_prd_c_hdr.h"
namespace pr {
#include "elb_pr_c_hdr.h"
}
namespace psp {
#include "elb_psp_c_hdr.h"
}

#include "platform/pal/include/pal.h"
#include "elbmon.hpp"

static inline void
elbmon_pipeline_rx_data_store1 (int type, uint64_t hostq_xoff_cnt,
                                uint64_t pkt_xoff_cnt, uint64_t phv_xoff_cnt,
                                uint8_t pb_xoff, uint8_t phv_xoff,
                                uint8_t host_xoff)
{
    pipeline_t *pipeline = &asic->pipelines[type];

    pipeline->hostq_xoff_cnt = hostq_xoff_cnt;
    pipeline->pkt_xoff_cnt = pkt_xoff_cnt;
    pipeline->phv_xoff_cnt = phv_xoff_cnt;
    pipeline->pb_xoff = pb_xoff;
    pipeline->phv_xoff = phv_xoff;
    pipeline->host_xoff = host_xoff;
}

void
prd_read_counters (int verbose)
{
  uint32_t sta_id[3];
  uint32_t sta_fifo[3];
    uint32_t cnt[4];
    int i;
    int pend_rd = 0;
    int pend_wr = 0;
    int num_phv = 0;
    int rd_ff_empty = 0;
    int rd_ff_full = 0;
    int wr_ff_empty = 0;
    int wr_ff_full = 0;
    int pkt_ff_empty = 0;
    int pkt_ff_full = 0;
    int pb_xoff = 0, phv_xoff = 0, host_xoff = 0;
    int hostq_xoff_cnt = 0, pkt_xoff_cnt = 0, phv_xoff_cnt = 0;
    int pkt_ff_depth = 0;
    int polls = 100;

    // Drops at the end of RXDMA pipeline
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET + ELB_PR_CSR_PRD_BYTE_ADDRESS +
                      ELB_PRD_CSR_CNT_PHV_BYTE_OFFSET,
                  cnt, 4);
    uint32_t phv_drop = ELB_PRD_CSR_CNT_PHV_CNT_PHV_1_4_DROP_GET(cnt[1]);
    uint32_t phv_err = ELB_PRD_CSR_CNT_PHV_CNT_PHV_2_4_ERR_GET(cnt[2]);
    uint32_t phv_recirc = ELB_PRD_CSR_CNT_PHV_CNT_PHV_3_4_RECIRC_GET(cnt[3]);

    // Number of PHVs sent from RXDMA Stage 7 to PDMA
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_MA_BYTE_ADDRESS,
                  cnt, 3);
    int ma_cnt = ELB_PRD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]);

    // Number of PHVs recirculated
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_PS_RESUB_PKT_BYTE_ADDRESS,
                  cnt, 3);
    int resub_cnt =
        ELB_PRD_CSR_CNT_PS_RESUB_PKT_CNT_PS_RESUB_PKT_0_3_SOP_31_0_GET(cnt[0]);

    // TODO
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  cnt, 3);
    int ps_cnt = ELB_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

    for (i = 0; i < polls; i++) {
        /*
            [  31   ] axi_wr_ready       drdy
            [  30   ] axi_wr_valid       srdy
            [  29   ] ma_drdy
            [  28   ] ma_srdy
            [  27   ] wr_id_gnt
            [  26   ] pkt_ff_almost_full X
            [  25   ] phv_lpbk_out_srdy  X
            [  24   ] phv_lpbk_in_drdy   X
            [  23   ] ffence_ff_empty    X
            [  22   ] ffence_ff_full     X
            [  21   ] dfence_ff_empty
            [  20   ] dfence_ff_full
            [  19   ] wr_mem_ff_empty
            [  18   ] wr_mem_ff_full
            [  17   ] pkt_stg_ff_empty
            [  16   ] pkt_stg_ff_full
            [  15   ] pkt_order_ff_empty
            [  14   ] pkt_order_ff_full
            [  13   ] pkt_ff_empty
            [  12   ] pkt_ff_full
            [  11   ] cmd_ff_empty
            [  10   ] cmd_ff_full
            [   9   ] cmdflit_ff_empty
            [   8   ] cmdflit_ff_full
            [   7   ] rcv_stg_ff_empty
            [   6   ] rcv_stg_ff_full
            [   5   ] wdata_ff_empty
            [   4   ] wdata_ff_full
            [   3   ] wr_lat_ff_empty
            [   2   ] wr_lat_ff_full
            [   1   ] rd_lat_ff_empty
            [   0   ] rd_lat_ff_full
        */

        // FIFO Status
        pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                          ELB_PR_CSR_PRD_STA_FIFO_BYTE_ADDRESS,
                      sta_fifo, 3);

        rd_ff_full   += ELB_PRD_CSR_STA_FIFO_STA_FIFO_0_3_RD_LAT_FF_FULL_GET(sta_fifo[0]);
        rd_ff_empty  += ELB_PRD_CSR_STA_FIFO_STA_FIFO_0_3_RD_LAT_FF_EMPTY_GET(sta_fifo[0]);
        wr_ff_full   += ELB_PRD_CSR_STA_FIFO_STA_FIFO_0_3_WR_LAT_FF_FULL_GET(sta_fifo[0]);
        wr_ff_empty  += ELB_PRD_CSR_STA_FIFO_STA_FIFO_0_3_WR_LAT_FF_EMPTY_GET(sta_fifo[0]);
        pkt_ff_full  += ELB_PRD_CSR_STA_FIFO_STA_FIFO_0_3_PKT_FF_FULL_GET(sta_fifo[0]);
        pkt_ff_empty += ELB_PRD_CSR_STA_FIFO_STA_FIFO_1_3_PKT_FF_EMPTY_GET(sta_fifo[1]);

        // Pending reads/writes:
        pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
		      ELB_PRD_CSR_CNT_PHV_ID_BYTE_ADDRESS, 
                      sta_id, 3);

        pend_rd += ELB_PRD_CSR_STA_ID_STA_ID_0_3_RD_PEND_CNT_GET(sta_id[0]);
        pend_wr += ELB_PRD_CSR_STA_ID_STA_ID_0_3_WR_PEND_CNT_GET(sta_id[0]);

        // num PHVs, xoff:
        pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                          ELB_PR_CSR_PRD_STA_XOFF_BYTE_ADDRESS,
                      cnt, 3);

        hostq_xoff_cnt +=
            ELB_PRD_CSR_STA_XOFF_STA_XOFF_0_3_HOSTQ_XOFF_COUNTER_GET(cnt[0]);
        phv_xoff_cnt +=
            ELB_PRD_CSR_STA_XOFF_STA_XOFF_0_3_NUMPHV_COUNTER_GET(cnt[0]);
        pkt_xoff_cnt +=
            (ELB_PRD_CSR_STA_XOFF_STA_XOFF_1_3_PKT_XOFF_COUNTER_11_8_GET(cnt[1])
             << 8) +
            ELB_PRD_CSR_STA_XOFF_STA_XOFF_0_3_PKT_XOFF_COUNTER_7_0_GET(cnt[0]);

        phv_xoff += ELB_PRD_CSR_STA_XOFF_STA_XOFF_0_3_NUMPHV_XOFF_GET(cnt[0]);

        pb_xoff += ELB_PRD_CSR_STA_XOFF_STA_XOFF_1_3_PKT_PBUS_XOFF_GET(cnt[1]);
        host_xoff =
            ELB_PRD_CSR_STA_XOFF_STA_XOFF_1_3_HOST_PBUS_XOFF_GET(cnt[1]);

        pkt_ff_depth +=
            ELB_PRD_CSR_STA_XOFF_STA_XOFF_2_3_PKT_FF_DEPTH_GET(cnt[2]);
    }

    elbmon_pipeline_data_store1(RXDMA, ma_cnt, ps_cnt, phv_drop, phv_err,
                                phv_recirc, resub_cnt, num_phv, polls);
    elbmon_pipeline_data_store2(RXDMA, pend_rd / polls, pend_wr / polls,
                                rd_ff_empty, rd_ff_full, wr_ff_empty,
                                wr_ff_full, pkt_ff_empty, pkt_ff_full,
                                pkt_ff_depth, polls);
    elbmon_pipeline_rx_data_store1(RXDMA, hostq_xoff_cnt, pkt_xoff_cnt,
                                   phv_xoff_cnt, (phv_xoff * 100) / polls,
                                   (pb_xoff * 100) / polls,
                                   (host_xoff * 100) / polls);
}

void
prd_reset_counters (int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET + ELB_PR_CSR_PRD_BYTE_ADDRESS +
                      ELB_PRD_CSR_CNT_PHV_BYTE_OFFSET,
                  zero, 4);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_MA_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_PS_RESUB_PKT_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS,
                  zero, 3);
}

void
psp_read_counters (int verbose)
{
    uint64_t ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt,
        ma_recirc_cnt;

    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  (uint32_t *)&ma_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  (uint32_t *)&sw_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pb_pbus_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pr_pbus_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  (uint32_t *)&ma_drop_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  (uint32_t *)&ma_recirc_cnt, 2);

    elbmon_pipeline_data_store3(RXDMA, ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt,
                                ma_drop_cnt, ma_recirc_cnt);
}

void
psp_reset_counters (int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PR_PR_OFFSET +
                      ELB_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  zero, 2);
}

