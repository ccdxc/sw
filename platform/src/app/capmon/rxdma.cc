/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "cap_top_csr_defines.h"
namespace pr
{
#include "cap_pr_c_hdr.h"
}
namespace psp
{
#include "cap_psp_c_hdr.h"
}

#include "platform/src/lib/pal/include/pal.h"

void
prd_read_counters(int verbose)
{
    uint32_t sta_id, sta_fifo = 0;
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
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_BYTE_ADDRESS +
                      CAP_PRD_CSR_CNT_PHV_BYTE_OFFSET,
                  cnt, 4);
    uint32_t phv_drop = CAP_PRD_CSR_CNT_PHV_CNT_PHV_1_4_DROP_GET(cnt[0]);
    uint32_t phv_err = CAP_PRD_CSR_CNT_PHV_CNT_PHV_2_4_ERR_GET(cnt[2]);
    uint32_t phv_recirc = CAP_PRD_CSR_CNT_PHV_CNT_PHV_3_4_RECIRC_GET(cnt[3]);

    // Number of PHVs sent from RXDMA Stage 7 to PDMA
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_MA_BYTE_ADDRESS, cnt, 3);
    int ma_cnt = CAP_PRD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]);

    // Number of PHVs recirculated
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_PS_RESUB_PKT_BYTE_ADDRESS, cnt,
                  3);
    int resub_cnt = CAP_PRD_CSR_CNT_PS_RESUB_PKT_CNT_PS_RESUB_PKT_0_3_SOP_31_0_GET(cnt[0]);

    // TODO
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, cnt, 3);
    int ps_cnt = CAP_PRD_CSR_CNT_PS_PKT_CNT_PS_PKT_0_3_SOP_31_0_GET(cnt[0]);

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
        pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_STA_FIFO_BYTE_ADDRESS, &sta_fifo,
                      1);

        rd_ff_full += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_FULL_GET(sta_fifo);
        rd_ff_empty += CAP_PRD_CSR_STA_FIFO_RD_LAT_FF_EMPTY_GET(sta_fifo);
        wr_ff_full += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_FULL_GET(sta_fifo);
        wr_ff_empty += CAP_PRD_CSR_STA_FIFO_WR_LAT_FF_EMPTY_GET(sta_fifo);
        pkt_ff_full += CAP_PRD_CSR_STA_FIFO_PKT_FF_FULL_GET(sta_fifo);
        pkt_ff_empty += CAP_PRD_CSR_STA_FIFO_PKT_FF_EMPTY_GET(sta_fifo);

        // Pending reads/writes:
        pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_STA_ID_BYTE_ADDRESS, &sta_id, 1);

        pend_rd += CAP_PRD_CSR_STA_ID_RD_PEND_CNT_GET(sta_id);
        pend_wr += CAP_PRD_CSR_STA_ID_WR_PEND_CNT_GET(sta_id);

        // num PHVs, xoff:
        pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_STA_XOFF_BYTE_ADDRESS, cnt, 3);

        hostq_xoff_cnt += CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_HOSTQ_XOFF_COUNTER_GET(cnt[0]);
        phv_xoff_cnt += CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_NUMPHV_COUNTER_GET(cnt[0]);
        pkt_xoff_cnt +=
            (CAP_PRD_CSR_STA_XOFF_STA_XOFF_1_3_PKT_XOFF_COUNTER_10_9_GET(cnt[1]) << 8) +
            CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_PKT_XOFF_COUNTER_8_0_GET(cnt[0]);

        phv_xoff += CAP_PRD_CSR_STA_XOFF_STA_XOFF_0_3_NUMPHV_XOFF_GET(cnt[0]);

        pb_xoff += CAP_PRD_CSR_STA_XOFF_STA_XOFF_1_3_PKT_PBUS_XOFF_GET(cnt[1]);
        host_xoff = CAP_PRD_CSR_STA_XOFF_STA_XOFF_1_3_HOST_PBUS_XOFF_GET(cnt[1]);

        pkt_ff_depth += CAP_PRD_CSR_STA_XOFF_STA_XOFF_2_3_PKT_FF_DEPTH_GET(cnt[2]);
    }

    printf(" phv=%u pkt=%u drop=%u(%u%%) err=%u recirc=%u resub=%u in_flight=%u\n", ma_cnt, ps_cnt,
           phv_drop, (phv_drop * 100) / ma_cnt, phv_err, phv_recirc, resub_cnt, num_phv / polls);

    printf("       AXI reads=%u writes=%u\n", pend_rd / polls, pend_wr / polls);

    printf("       FIFO (empty%%/full%%) rd=%u/%u wr=%u/%u pkt=%u/%u ff_depth=%u\n",
           (rd_ff_empty * 100) / polls, (rd_ff_full * 100) / polls, (wr_ff_empty * 100) / polls,
           (wr_ff_full * 100) / polls, (pkt_ff_empty * 100) / polls, (pkt_ff_full * 100) / polls,
           pkt_ff_depth / polls);

    printf("       XOFF hostq=%u pkt=%u phv=%u phv_xoff=%u%% pb_xoff=%u%% host_xoff=%u%%\n",
           hostq_xoff_cnt, pkt_xoff_cnt, phv_xoff_cnt, (phv_xoff * 100) / polls,
           (pb_xoff * 100) / polls, (host_xoff * 100) / polls);
}

void
prd_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_BYTE_ADDRESS +
                      CAP_PRD_CSR_CNT_PHV_BYTE_OFFSET,
                  zero, 4);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_MA_BYTE_ADDRESS, zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_PS_RESUB_PKT_BYTE_ADDRESS, zero,
                  3);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PR_CSR_PRD_CNT_PS_PKT_BYTE_ADDRESS, zero, 3);
}

void
psp_read_counters(int verbose)
{
    uint64_t ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt, ma_recirc_cnt;

    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  (uint32_t *)&ma_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  (uint32_t *)&sw_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pb_pbus_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pr_pbus_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  (uint32_t *)&ma_drop_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  (uint32_t *)&ma_recirc_cnt, 2);

    printf(" PSP: phv=%ld pb_pbus=%ld pr_pbus=%ld sw=%ld phv_drop=%ld recirc=%ld\n", ma_cnt,
           sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt, ma_recirc_cnt);
}

void
psp_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PR_PR_OFFSET + CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS, zero, 2);
}
